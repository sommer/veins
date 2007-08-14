#include <math.h>
#include "PositifLayer.h"
#include <string.h>
#include "winmath.h"

// Node subclass containing Statistic algorithm

#define msec		(1e-3)
#define ACCEPT		0.003
#define POS_DELAY	9

// Define timer types
#define TIMER_SND_POS 0
#define TIMER_DO_TRI  1
#define TIMER_COUNT 2

#define ZERO_CONF       0.00

// Define message types
#define MSG_POSITION  MSG_TYPE_BASE+0

typedef struct {
	Position pos;
	double confidence;
} vague_pos;

typedef struct {
	int idx;
	FLOAT distance;
	Position tl, br;	// top-left and bottom-right corners of bounding box
	vague_pos poss[2];
} nghbor_info;


class Node_Statistic:public PositifLayer {
	double accuracy;
	int domulti;
	FLOAT residu;

	struct {
		Position min, max;
	} rectangle;		// convex region I must be in
	bool valid_rectangle;
	bool df_changed;	// something has changed in the distribution, re-calculate the matrix
	float point_dist;	// distance between two points that get calculated for

	// Only for non-anchor nodes. Another guess...
	// position is now the average guess, confidence ditto. these are the two possibilities
	// (if confidence2 == 0, then there's only 1)
	Position position1;
	double confidence1;
	Position position2;
	double confidence2;

	cLinkedList neighbors;

	nghbor_info summary;
	bool summary_update;

	bool equal_pairs;	//use equal pairs heuristic

	timer_info *bc_position;
	int bc_pos_delay;
	timer_info *comp_position;

	void sendPosition(void *arg = NULL);
	void do_triangulation(void *arg = NULL);

	void Statistic();
	void update_neighbor(cMessage * msg);

	void update_rectangle(nghbor_info * anchor);
	bool inside_rectangle(Position pos);
	bool inside_neighbors_range(Position pos);

	double true_pos_triangulate(void);

	FLOAT minmax(int n_pts, Position ** positions, FLOAT * ranges,
		     FLOAT * confs);
	FLOAT gaussian(FLOAT x1, FLOAT x2, FLOAT mu, FLOAT sigma);
	FLOAT resid(int n_pts, Position ** positions, FLOAT * ranges,
		    FLOAT * confs, Position test, bool use_confs);
	Position *add_pos(Position init, Position add);
	Position *sub_pos(Position init, Position sub);

      public:
	 Module_Class_Members(Node_Statistic, PositifLayer, 0)
	    // Implement Node's abstract functions.
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

Define_Module_Like(Node_Statistic, PositifLayer);

void Node_Statistic::init(void)
{
	algorithm = 1;		// Output on raw data line only.
	version = 8;
	point_dist = 1.0;

	if (MAX_TIMERS < TIMER_COUNT) {
		error
		    ("MAX_TIMERS < TIMER_COUNT: (%d < %d), increase MAX_TIMERS in PositifLayer.h\n",
		     MAX_TIMERS, TIMER_COUNT);
		abort();
	} else {
		/* allocate the needed timers */
		allocateRepeatTimers(TIMER_COUNT);
	}
}

// Dispatch this timer to the proper handler function.
void Node_Statistic::handleTimer(timer_info * timer)
{
	EV << "timer event\n";
	switch (*timer) {
	case TIMER_SND_POS:
		sendPosition(contextPointer(*timer));
		break;

	case TIMER_DO_TRI:
		Statistic();
		break;
	}
}

void Node_Statistic::handleMessage(cMessage * msg, bool newNeighbor)
{
	if (status == STATUS_ANCHOR)
		return;

	if (domulti == 0 && (double) msg->par("confidence1") != 1.0)
		return;

	if (newNeighbor)
		// Activate all timer routines when we meet a new neighbor
//              for (cLinkedListIterator iter = getTimers(); !iter.end();
//                   iter++) {
//                      timer_info *ev = (timer_info *) iter();
//                      resetTimer(ev);
//              }
		resetAllRepeatTimers();
	// Call appropriate handler function depending on whether this node is an
	// anchor or a normal node.
	update_neighbor(msg);
	if (neighbors.length() >= nr_dims) {
		// Don't perform triangulation yet, wait for other updates
		if (comp_position == NULL) {
			comp_position = timer(1, TIMER_DO_TRI);
			addTimer(comp_position);
		}
		resetTimer(comp_position);
	}
}

void Node_Statistic::handleStartMessage(cMessage * msg)
{
	PositifLayer::handleStartMessage(msg);
	// query module parameters
	accuracy = par("accuracy");
	domulti = par("multihop");
	equal_pairs = par("equal_pairs");

	EV << "Equal pairs: " << (equal_pairs ? "true" : "false") << "\n";

	comp_position = NULL;
	bc_position = NULL;

	df_changed = false;

	summary.idx = me;
	summary_update = false;

	EV << "start msg " << status << "\n";
	if (status == STATUS_ANCHOR) {
		confidence = 1;
		nghbor_info *ME = new nghbor_info;
		ME->idx = me;
		ME->poss[0].confidence = confidence;
		// Anchor nodes know where they are, so initialization starts from here
		memcpy(ME->poss[0].pos, position, sizeof(position));
		memcpy(rectangle.min, position, sizeof(position));
		memcpy(rectangle.max, position, sizeof(position));
		EV << "anchor at " << pos2str(position) << "\n";
		valid_rectangle = true;

		bc_position = timer(1, TIMER_SND_POS);
		bc_pos_delay = 0;
		addTimer(bc_position);
		neighbors.insert(ME);
	} else {
		bc_position = NULL;
		confidence = ZERO_CONF;
		valid_rectangle = false;
	}
}

void Node_Statistic::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);


	// Set final confidence to low when there aren't enough neighbors
	if (status != STATUS_ANCHOR && neighbors.length() <= nr_dims)
		node[me].perf_data.confidence = ZERO_CONF + 0.001;

	PositifLayer::handleStopMessage(msg);

	send(returnMsg);

	/* free up things */
//      delete bc_position;
//      delete comp_position;
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();
		delete neighbor;
	}
}


double Node_Statistic::true_pos_triangulate(void)
{
	int n = neighbors.length();
	FLOAT** pos_list = new FLOAT*[n + 1];
	FLOAT* range_list = new FLOAT[n + 1];
	Position pos;

	int i = 0;
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();

		pos_list[i] = node[neighbor->idx].true_pos;
		range_list[i] = neighbor->distance;
		i++;
	}

	pos_list[i] = pos;
	FLOAT res = triangulate(i, pos_list, range_list, NULL, me);

	delete[] range_list;
	delete[] pos_list;

	return (res < 0 || res > range ? -1 : distance(pos, node[me].true_pos) / range);
}

void Node_Statistic::update_rectangle(nghbor_info * anchor)
{
	// intersect anchor's "square" with existing convex rectangle
	float osize;
	if (valid_rectangle) {
		osize = 1;
		for (int d = 0; d < nr_dims; d++) {
			osize *= (rectangle.max[d] - rectangle.min[d]);
		}
	} else
		osize = -1;
	EV << node[me].ID << ": Updating from " << anchor->idx << "\n";
	EV << node[me].ID << ": old size = " << osize << "\n";
	float size = 1;
	for (int d = 0; d < nr_dims; d++) {
		FLOAT left = floor(Max(0, anchor->tl[d] - range));
		FLOAT right = ceil(anchor->br[d] + range);

		if (!valid_rectangle) {
			rectangle.min[d] = left;
			rectangle.max[d] = right;
		} else {
			rectangle.min[d] = Max(rectangle.min[d], left);
			rectangle.max[d] = Max(0, Min(rectangle.max[d], right));
		}
		size *= (rectangle.max[d] - rectangle.min[d]);
	}
	valid_rectangle = true;
	EV << node[me].ID << ": TL = " << pos2str(rectangle.min) << ",";
	EV << "BR = " << pos2str(rectangle.max) << " size = " << size << "\n";
}


bool Node_Statistic::inside_rectangle(Position pos)
{
	assert(valid_rectangle);

	// Check if this position fits in the convex rectangle
	for (int d = 0; d < nr_dims; d++) {
		if (pos[d] < rectangle.min[d] || pos[d] > rectangle.max[d]) {
			return false;
		}
	}
	return true;
}


bool Node_Statistic::inside_neighbors_range(Position pos)
{
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();

		if (neighbor->poss[0].confidence == 1.0
		    && distance(pos, neighbor->poss[0].pos) > range) {
			return false;
		}
	}
	return true;
}


void Node_Statistic::update_neighbor(cMessage * msg)
{
	nghbor_info *neighbor = NULL;
	int src = msg->par("src");

	bool found = false;
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		neighbor = (nghbor_info *) iter();

		if (neighbor->idx == src) {
			found = true;
			break;
		}
	}

	if (!found) {
		neighbor = new nghbor_info;
		neighbor->idx = src;
		neighbors.insert(neighbor);
	}

	get_struct(msg, "pos1", *(Position *) neighbor->poss[0].pos);
	get_struct(msg, "pos2", *(Position *) neighbor->poss[1].pos);
	get_struct(msg, "tl", *(Position *) neighbor->tl);
	get_struct(msg, "br", *(Position *) neighbor->br);
	neighbor->distance = (double) msg->par("distance");
	neighbor->poss[0].confidence = msg->par("confidence1");
	neighbor->poss[1].confidence = msg->par("confidence2");
	update_rectangle(neighbor);

	df_changed = true;
}

void Node_Statistic::sendPosition(void *arg)
{
	assert(valid_rectangle);
	if (confidence <= ACCEPT && confidence2 <= ACCEPT
	    && confidence1 <= ACCEPT) {
		EV << node[me].
		    ID << ": pos msg junked. New conf = " << confidence << "\n";
		return;
	}
	//assert(position[0] != 0 || position[1] != 0);
	cMessage *msg = new cMessage("POSITION", MSG_POSITION);
	if (confidence2 > 0) {
		add_struct(msg, "pos1", position1);
		msg->addPar("confidence1") = confidence1;
	} else {
		add_struct(msg, "pos1", position);
		msg->addPar("confidence1") = confidence;
	}
	msg->addPar("confidence2") = confidence2;
	add_struct(msg, "pos2", position2);
	add_struct(msg, "tl", rectangle.min);
	add_struct(msg, "br", rectangle.max);

	char tmp[100];
	sprintf(tmp, "p=%d,%d;i=ball_vs",
		(int) (100 * position[0] / sqrt(area)),
		(int) (100 * position[1] / sqrt(area)));
	//setDisplayString(dispSUBMOD, (const char *) tmp, true);

	EV << node[me].ID << ": sending position\n";

	send(msg);
}

void Node_Statistic::Statistic(void)
{
	if (status == STATUS_ANCHOR || !df_changed)	// nothing has changed, so nothing to do!
		return;
	int n = neighbors.length();

	Position** pos_list = new Position*[(n + 1) * 2];
	if (range_list)
		delete[]range_list;
	range_list = new FLOAT[n + 1];
	if (real_range_list)
		delete[]real_range_list;
	FLOAT *conf_list = new FLOAT[(n + 1) * 2];

	real_range_list = new FLOAT[n + 1];	// Used for debug output only

	EV << node[me].ID << ": working with ";
	int i = 0;
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *anchor = (nghbor_info *) iter();
		pos_list[i * 2] = &(anchor->poss[0].pos);
		conf_list[i * 2] = anchor->poss[0].confidence;
		pos_list[(i * 2) + 1] = &(anchor->poss[1].pos);
		conf_list[(i * 2) + 1] = anchor->poss[1].confidence;
		range_list[i] = anchor->distance;
		real_range_list[i] =
		    distance(node[me].true_pos, node[anchor->idx].true_pos);
#ifndef NDEBUG
		EV << " " << node[anchor->idx].ID << "@" << pos2str(anchor->
								    poss[0].
								    pos);
		EV << "(tp " << pos2str(node[anchor->idx].
					true_pos) << ", rr=" <<
		    real_range_list[i] << ") (conf=" << anchor->poss[0].
		    confidence << ")";
		if (conf_list[(i * 2) + 1] > 0)
			EV << " & " << pos2str(anchor->poss[1].
					       pos) << " (conf=" << anchor->
			    poss[1].confidence << ")";
#endif
		i++;
	}
	EV << "\n";

	Position next, next2;
	pos_list[n * 2] = &next;
	pos_list[(n * 2) + 1] = &next2;

	// Calculate performance data for range error
	FLOAT sum_err = 0, sum_rel_err = 0, sum_abs_err = 0, sum_abs_rel_err =
	    0, sum_range = 0;
	for (i = 0; i < n; i++) {
		sum_err += range_list[i] - real_range_list[i];
		sum_rel_err +=
		    (range_list[i] - real_range_list[i]) / real_range_list[i];
		sum_abs_err += fabs(range_list[i] - real_range_list[i]);
		sum_abs_rel_err +=
		    fabs(range_list[i] -
			 real_range_list[i]) / real_range_list[i];
		sum_range += range_list[i];
	}
	node[me].perf_data.anchor_range_error = sum_err / used_anchors;
	node[me].perf_data.rel_anchor_range_error = sum_rel_err / used_anchors;
	node[me].perf_data.abs_anchor_range_error = sum_abs_err / used_anchors;
	node[me].perf_data.abs_rel_anchor_range_error =
	    sum_abs_rel_err / used_anchors;
	node[me].perf_data.anchor_range = sum_range / used_anchors;
	node[me].perf_data.anchor_range_error_count = used_anchors;

	/*FLOAT res = */ minmax(n, pos_list, range_list, conf_list);

	logprintf(" %4.0f,%4.0f\n", next[0], next[1]);

#ifndef NDEBUG
	EV << node[me].ID << ": True pos = " << pos2str(node[me].
							true_pos) << "\n";
	EV << node[me].
	    ID << ": possible pos " << pos2str(next) << " (conf = " <<
	    conf_list[n * 2] << ") err = " << distance(next,
						       node[me].
						       true_pos) << "\n";
#endif
	if (conf_list[(n * 2) + 1] != 0) {
#ifndef NDEBUG
		EV << node[me].
		    ID << ": possible pos " << pos2str(next2) << " (conf = " <<
		    conf_list[(n * 2) + 1] << ") err = " << distance(next2,
								     node[me].
								     true_pos)
		    << "\n";
#endif
		for (int i = 0; i < nr_dims; i++)
			position[i] =
			    ((next[i] * conf_list[n * 2]) +
			     (next2[i] * conf_list[(n * 2) + 1])) /
			    (conf_list[n * 2] + conf_list[(n * 2) + 1]);
		float mult =
		    2.0 * (1 -
			   (fabs(conf_list[n * 2] - conf_list[(n * 2) + 1]) /
			    conf_list[n * 2]));
		EV << node[me].ID << ": mult factor = " << mult << "\n";
		confidence =
		    (conf_list[n * 2] + conf_list[(n * 2) + 1]) / (2.0 + mult);
	} else {
		confidence = conf_list[n * 2];
		memmove(position, next, sizeof(Position));
	}

	confidence1 = conf_list[n * 2];
	confidence2 = conf_list[(n * 2) + 1];
	delete[]conf_list;
	memmove(position1, next, sizeof(Position));
	memmove(position2, next2, sizeof(Position));
#ifndef NDEBUG
	EV << node[me].
	    ID << ": UPDate pos to " << pos2str(position) << " (conf = " <<
	    confidence << ") err = " << distance(position,
						 node[me].true_pos) << "\n";
#endif

	if (confidence > ACCEPT || confidence1 > ACCEPT || confidence2 > ACCEPT) {
		status = STATUS_POSITIONED;
		node[me].perf_data.phase1_err =
		    distance(position, node[me].true_pos) / range;
#ifndef NDEBUG
		EV << node[me].ID << ": ACCEPT transmit\n";
#endif
		if (bc_position == NULL) {
			bc_position = timer(1, TIMER_SND_POS);
			bc_pos_delay = POS_DELAY;
			addTimer(bc_position);
		} else
			resetTimer(bc_position);
	} else {
		status = STATUS_BAD;
#ifndef NDEBUG
		EV << node[me].ID << ": REJECT transmit\n";
#endif
	}
//      wait(n * nr_dims * nr_dims * msec);
	delete[] pos_list;
}

#define POSCONF(p) map[(int)p[0]][(int)p[1]]


FLOAT Node_Statistic::resid(int n_pts, Position ** positions, FLOAT * ranges,
			    FLOAT * confs, Position test, bool use_confs)
{
	FLOAT residu = 0;
	FLOAT sum_c = 0;
	//EV << node[me].ID << ": test = "<<pos2str(test)<<"\n";
	for (int j = 0; j < n_pts * 2; j++) {
		if (confs[j] == 0)
			continue;
		FLOAT c = use_confs ? confs[j] : 1;
		//EV << node[me].ID << ": pos = "<<pos2str(*positions[j])<<", distance = "<<distance(test, *positions[j])<<"\n";
		residu +=
		    c * fabs(ranges[(j / 2)] - distance(test, *positions[j]));
		//EV << node[me].ID << ": residu = "<<residu<<"\n";
		assert(!isnan(residu));
		sum_c += c;
	}
	if (sum_c > 0)
		residu /= sum_c;
	assert(!isnan(residu));
	return residu;
}

Position *Node_Statistic::add_pos(Position init, Position add)
{
	static Position ret;
	for (int i = 0; i < nr_dims; i++)
		ret[i] = init[i] + add[i];
	return &ret;
}

Position *Node_Statistic::sub_pos(Position init, Position add)
{
	static Position ret;
	for (int i = 0; i < nr_dims; i++)
		ret[i] = init[i] - add[i];
	return &ret;
}

FLOAT Node_Statistic::minmax(int n_pts, Position ** positions, FLOAT * ranges,
			     FLOAT * confs)
{
	assert(nr_dims == 2);	// can't cope with anything else *yet*
	int width = (int) ceil(rectangle.max[0] - rectangle.min[0]), height =
	    (int) ceil(rectangle.max[1] - rectangle.min[1]);
	FLOAT** map = new FLOAT*[width];
	for (int i = 0; i < width; i++)
		map[i] = new FLOAT[height];

	Position rel_true_pos;
	memcpy(rel_true_pos, sub_pos(node[me].true_pos, rectangle.min),
	       sizeof(Position));
	Position pos, best;
	Position centre;
	//FLOAT weightings[n_pts*2];

	for (int i = 0; i < n_pts * 2; i++) {
		if (confs[i] != 0) {
			EV << node[me].
			    ID << ": anchor is at " <<
			    pos2str(*sub_pos(*positions[i], rectangle.min)) <<
			    " range = " << ranges[i /
						  2] << " real range = " <<
			    real_range_list[i / 2] << "\n";
		}
	}

	for (int i = 0; i < nr_dims; i++) {
		double k = 0;
		centre[i] = 0;
		for (int j = 0; j < n_pts * 2; j++) {
			if (confs[j] > 0) {
				centre[i] +=
				    confs[j] * ((*positions[j])[i] -
						rectangle.min[i]);
				k += confs[j];
			}
		}
		centre[i] /= k;
	}
	Position delta;
	int not_zero = 0;
	for (int i = 0; i < nr_dims; i++) {
		int k = 0;
		delta[i] = 0;
		for (int j = 0; j < n_pts * 2; j++) {
			if (confs[j] > 0) {
				delta[i] +=
				    confs[j] *
				    fabs(((*positions[j])[i] -
					  rectangle.min[i]) - centre[i]);
				k++;
			}
		}
		if (equal_pairs && delta[i] > 0)
			not_zero += 1;
	}
	if (equal_pairs) {
		for (int i = 0; i < nr_dims; i++) {
			if (delta[i] == 0)
				continue;
			for (int j = 0; j < nr_dims; j++) {
				if (i == j || delta[j] == 0)
					continue;
				if ((delta[j] / delta[i]) > 10.0)
					not_zero -= 1;
			}
		}
	}

	EV << node[me].ID << ": delta = " << pos2str(delta) << "\n";

	EV << node[me].
	    ID << ": width= " << width << ", height=" << height << "\n";

	for (int j = 0; j < width; j++) {
		for (int k = 0; k < height; k++) {
			map[j][k] = 1;
		}
	}

	double total = 1, newtotal;
	for (int i = 0; i < n_pts; i++) {
		newtotal = 0;
		for (int j = 0; j < width; j++) {
			pos[0] = j + rectangle.min[0] + point_dist / 2.0;
			for (int k = 0; k < height; k++) {
				pos[1] = k + rectangle.min[1];
				FLOAT dist = distance(*(positions[i * 2]), pos);
				FLOAT val =
				    confs[i * 2] * gaussian(dist -
							    (point_dist / 2.0),
							    dist +
							    (point_dist / 2.0),
							    ranges[i],
							    range * 0.2);
				//EV << node[me].ID << ": Adding in "<<pos2str(*positions[i*2]) <<" with conf "<<val<<", range="<<dist<<"\n";
				if (equal_pairs && confs[i * 2] != 1.0
				    && confs[(i * 2) + 1] > 0) {
					dist =
					    distance(*positions[(i * 2) + 1],
						     pos);
					val +=
					    confs[(i * 2) + 1] * gaussian(dist -
									  (point_dist
									   /
									   2.0),
									  dist +
									  (point_dist
									   /
									   2.0),
									  ranges
									  [i],
									  range
									  *
									  0.2);
					val /= 4.0;
				}
				map[j][k] *= val / total;
				assert(!isnan(map[j][k]));
				assert(map[j][k] >= 0);
				newtotal += map[j][k];
				assert(!isnan(newtotal));
			}
		}
		total = newtotal;
	}

	Position avg;
	memset(avg, 0, sizeof(Position));
	for (int j = 0; j < width; j++) {
		for (int k = 0; k < height; k++) {
			avg[0] += j * map[j][k];
			assert(!isnan(avg[0]));
			avg[1] += k * map[j][k];
			assert(!isnan(avg[1]));
			if ((j == 0 && k == 0) || map[j][k] > POSCONF(best)) {
				best[0] = j;
				best[1] = k;
			}
		}
	}

	//memcpy(best, avg, sizeof(Position));
	EV << node[me].
	    ID << ": total = " << total << " n_pts = " << n_pts << "\n";
	EV << node[me].
	    ID << ": average = " << pos2str(avg) << " conf = " << POSCONF(avg)
	    << "\n";

	Position second;
	second[0] = -1;

	EV << node[me].
	    ID << ": centre = " << pos2str(centre) << " conf = " <<
	    POSCONF(centre) << "\n";
	EV << node[me].
	    ID << ": abs centre = " << pos2str(*add_pos(centre, rectangle.min))
	    << "\n";
	EV << node[me].
	    ID << ": current guess = " << pos2str(best) << " conf = " <<
	    POSCONF(best) << "\n";

	Position work;
	memcpy(work, centre, sizeof(Position));	/* allows for easy toggle between avg and centre */

	if (equal_pairs)
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < nr_dims; j++) {
				int next = j + 1;
				if (next == nr_dims)
					next = 0;
				FLOAT a = best[next] - work[next];
				FLOAT b = best[j] - work[j];
				int signmod =
				    (a > 0 ? 1 : -1) == (b >
							 0 ? 1 : -1) ? 1 : -1;
				best[j] = work[j] - signmod * a;
				best[next] = work[next] + signmod * b;
				//best[j] += (work[j]-best[j])*2;
				//EV << "next="<<next<<" j="<<j<<"\n";

				int l = 0;
				for (; l < nr_dims; l++)
					if (best[l] < 0
					    || best[l] >
					    rectangle.max[l] - rectangle.min[l])
						break;
				if (l != nr_dims)
					continue;
				EV << node[me].
				    ID << ": Alternate = " << pos2str(best) <<
				    " conf = " << POSCONF(best) <<
				    " true pos dist= " << distance(rel_true_pos,
								   best) <<
				    "\n";
				if (j != nr_dims - 1 || i != 1) {
					if (second[0] == -1
					    || POSCONF(best) > POSCONF(second))
						memcpy(second, best,
						       sizeof(Position));
				}
			}
	//second[0] = -1;
	EV << node[me].
	    ID << ": rel true pos = " << pos2str(rel_true_pos) << " conf = " <<
	    POSCONF(rel_true_pos) << "\n";

	if (!equal_pairs || not_zero == nr_dims) {
		second[0] = -1;
		/*if (distance(avg,best)>distance(avg,second))
		   {
		   memcpy(best,second,sizeof(Position));
		   } */
	}
	confs[n_pts * 2] = POSCONF(best);
	if (second[0] != -1)
		confs[(n_pts * 2) + 1] = POSCONF(second);
	else
		confs[(n_pts * 2) + 1] = 0;
	//confs[n_pts] = total_conf / (nr_dims * 2);

	{
		float m = (range * range * 2) / (width * height);
		m = 1 / m;
		float s = fabs(static_cast<float>(width - height)) / (range * 2);
		EV << node[me].ID << ": mult factor = " << m << "\n";
		EV << node[me].ID << ": squareness = " << s << "\n";
		EV << node[me].ID << ": metric (m*s) = " << m *
		    s << " or (m/s) = " << m / s << "\n";
		EV << node[me].ID << ": metric total = " << (m * s) + (m / s) +
		    m + s << "\n";
		float f_weight =
		    resid(n_pts, positions, ranges, confs, best, true);
		if (f_weight > range * 5) {
			confs[(n_pts * 2)] = 0.99 * ACCEPT;
		}
		EV << node[me].
		    ID << ": residue (first) weighted = " << f_weight <<
		    " not = " << resid(n_pts, positions, ranges, confs, best,
				       false) << "\n";
		if (confs[(n_pts * 2) + 1] != 0) {
			float s_weight =
			    resid(n_pts, positions, ranges, confs, second,
				  true);
			EV << node[me].
			    ID << ": residue (second) weighted = " << s_weight
			    << " not = " << resid(n_pts, positions, ranges,
						  confs, second, false) << "\n";
			if (equal_pairs && s_weight > range * 5
			    && confs[(n_pts * 2) + 1] > 0.99 * ACCEPT)
				confs[(n_pts * 2) + 1] = 0.99 * ACCEPT;
		}

	}

	EV << node[me].
	    ID << ": Best guess = " << pos2str(best) << " conf = " <<
	    confs[n_pts * 2] << "\n";

	for (int i = 0; i < nr_dims; i++) {
		(*positions[n_pts * 2])[i] = best[i] + rectangle.min[i];
		(*positions[(n_pts * 2) + 1])[i] = second[i] + rectangle.min[i];
	}

	char fname[256];
	sprintf(fname, "output/%d.data", node[me].ID);
	FILE *dump = fopen(fname, "w");
	for (int j = 0; j < width; j++)
		for (int k = 0; k < height; k++)
			fprintf(dump, "d %d %d %f\n", j, k, map[j][k]);
	fprintf(dump, "r %f %f %f %f\n", rectangle.min[0], rectangle.min[1],
		rectangle.max[0], rectangle.max[1]);
	for (int i = 0; i < n_pts; i++) {
		if (confs[i * 2] == 1 || confs[(i * 2) + 1] < 0.00001)
			fprintf(dump, "a %f %f %f %f\n", (*positions[i * 2])[0],
				(*positions[i * 2])[1], confs[i * 2],
				ranges[i]);
		else
			fprintf(dump, "i %f %f %f %f %f %f %f\n",
				(*positions[i * 2])[0], (*positions[i * 2])[1],
				confs[i * 2], (*positions[(i * 2) + 1])[0],
				(*positions[(i * 2) + 1])[1],
				confs[(i * 2) + 1], ranges[i]);
	}
	fprintf(dump, "t %f %f %f\n", rel_true_pos[0], rel_true_pos[1],
		POSCONF(rel_true_pos));
	fprintf(dump, "g %f %f %f\n", best[0], best[1], confs[n_pts * 2]);
	if (confs[(n_pts * 2) + 1] > 0)
		fprintf(dump, "g %f %f %f\n", second[0], second[1],
			confs[(n_pts * 2) + 1]);

	fclose(dump);

	for (int i = 0; i < width; i++)
		delete[] map[i];
	delete[] map;

	return residu;
}

FLOAT Node_Statistic::gaussian(FLOAT x1, FLOAT x2, FLOAT mu, FLOAT sigma)
{
	static FLOAT sq2 = M_SQRT2;
	double z = (x1 - mu) / sigma;
	double ret1 = 0.5 * (1.0 + erf(z / sq2));
	assert(ret1 <= 1 && ret1 >= -1);
	z = (x2 - mu) / sigma;
	double ret2 = 0.5 * (1.0 + erf(z / sq2));
	assert(ret2 <= 1 && ret2 >= -1);
	double ret = ret2 - ret1;
	assert(ret <= 1 && ret >= 0);
	return ret;
	/*double ret1 = 1/(sigma*sqrt(M_PI)) * exp(-pow((x1-mu)/sigma,2) / 2);
	   double ret2 = 1/(sigma*sqrt(M_PI)) * exp(-pow((x2-mu)/sigma,2) / 2);
	   double ret1 = ndtr((x1-mu)/sigma);
	   double ret2 = ndtr((x2-mu)/sigma);
	   return ret2-ret1; */
}
