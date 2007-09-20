#include "PositifLayer.h"
#include <string.h>
#include <list>

// Node subclass containing Savvides' algorithm and
// weighted average of beacons forinitialisation
// and Refine for refinement

#define msec		(1e-3)
#define ACCEPT		0.1
#define POS_DELAY	9

#define CONF_THR	(LOW_CONF*1.01)
#define ZERO_CONF       0.01
#define LOW_CONF        0.1

// Define timer types
#define TIMER_SND_POS 0
#define TIMER_SND_ANC 1
#define TIMER_DO_TRI  2
#define TIMER_COUNT 3

// Define message types
#define MSG_ANCHOR    MSG_TYPE_BASE+0
#define MSG_POSITION  MSG_TYPE_BASE+1

#define PHASE_SAVVIDES 0
#define PHASE_AVERAGE 1
#define PHASE1 0

typedef struct {
	int idx;
	double path_dst;
	int last_hop_idx;
	int cnt;
	bool flood;
	Position position;
} anchor_info;

typedef struct {
	int idx;
	double confidence;
	FLOAT distance;
	Position position;
	int nr_nghbrs;
	int *nghbr_idx;
	bool twin;
} nghbor_info;


typedef struct {
	FLOAT res; 
	FLOAT sum_conf;
	int neighbor_count;
	Position pos;
} tria_data;

void delete_tria_data (void * _data) {
	tria_data * data = (tria_data *)_data;
	if (data)
		delete data;
}

using std::list;

class Node_Savvides_Mob:public PositifLayer {
	double accuracy;

	list<timer_info> triangulation_timers;

	FLOAT residu;
	bool i_am_a_twin;

	struct {
		Position min, max;
	} rectangle;		// convex region I must be in
	bool valid_rectangle;

	cLinkedList anchors;
	cLinkedList neighbors;

	nghbor_info summary;
	bool summary_update;

	timer_info *bc_position;
	int bc_pos_delay;
	timer_info *bc_anchor;
	timer_info *comp_position;

	void remove_triangulation_timer(timer_info timer);

	void anchor(cMessage * msg);
	void unknown(cMessage * msg);

	void sendPosition(void *arg = NULL);
	void sendAnchor(void *anc);
	void do_triangulation(void *arg = NULL);
	void delayed_do_triangulation(timer_info timer, void * data);

	void savvides();
	bool new_anchor(cMessage * msg);
	void update_neighbor(cMessage * msg);

	void update_rectangle(anchor_info * anchor);
	bool inside_rectangle(Position pos);
	bool inside_neighbors_range(Position pos);
	bool twins(nghbor_info * a, nghbor_info * b);


	double true_pos_triangulate(void);

      public:
	 Module_Class_Members(Node_Savvides_Mob, PositifLayer, 0)
	    // Implement Node's abstract functions.
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

Define_Module_Like(Node_Savvides_Mob, PositifLayer);

void Node_Savvides_Mob::init(void)
{
	algorithm = 1;		// Output on raw data line only.
	version = 8;

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
void Node_Savvides_Mob::handleTimer(timer_info * timer)
{
	switch (*timer) {
	case TIMER_SND_POS:
		sendPosition(contextPointer(*timer));
		break;

	case TIMER_SND_ANC:
		sendAnchor(contextPointer(*timer));
		break;

	case TIMER_DO_TRI:
		do_triangulation(contextPointer(*timer));
		break;
	default:
		delayed_do_triangulation(*timer, contextPointer(*timer));
		break;
	}
}

void Node_Savvides_Mob::handleMessage(cMessage * msg, bool newNeighbor)
{
	if (newNeighbor)
		// Activate all timer routines when we meet a new neighbor
//              for (cLinkedListIterator iter = getTimers(); !iter.end();
//                   iter++) {
//                      timer_info *ev = (timer_info *) iter();
//                      resetTimer(ev);
//              }
		resetAllTimers();
	// Call appropriate handler function depending on whether this node is an
	// anchor or a normal node.
	if (status == STATUS_ANCHOR) {
		anchor(msg);
	} else {
		unknown(msg);
	}
}

void Node_Savvides_Mob::handleStartMessage(cMessage * msg)
{
	// query module parameters
	accuracy = par("accuracy");

	residu = 10 * range;

	comp_position = NULL;
	bc_anchor = NULL;
	bc_position = NULL;

	valid_rectangle = false;

	summary.idx = me;
	summary.nr_nghbrs = 0;
	summary_update = false;
	i_am_a_twin = false;
	confidence = (status == STATUS_ANCHOR) ? 1 : ZERO_CONF;

	bc_anchor = timer(reps, TIMER_SND_ANC, NULL);
//      bc_anchor->cnt = 0;
	cancelTimer(bc_anchor);
	addTimer(bc_anchor);

	// Is there an initialization (terrain) phase?
	if (status == STATUS_ANCHOR) {
		// Anchor nodes know where they are, so initialization starts from here
		anchor_info *ME = new anchor_info;

		ME->idx = me;
		ME->path_dst = 0.0;
		memcpy(ME->position, position, sizeof(position));
		ME->cnt = reps;
		ME->flood = true;
		resetTimer(bc_anchor);
		anchors.insert(ME);

		bc_position = timer(reps, TIMER_SND_POS);
		bc_pos_delay = 0;
		addTimer(bc_position);
	}
}

void Node_Savvides_Mob::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);

	// OLD: using 2nd phase now to compare weighted avg and Savvides_Mob
	// Use the 2nd phase slot to measure the error using Koens method.
	//  if(status!=STATUS_ANCHOR)
	//  node[me].perf_data.phase2_err = true_pos_triangulate();

	// Set final confidence to low when there aren't enough neighbors
	if (status != STATUS_ANCHOR && neighbors.length() <= nr_dims)
		node[me].perf_data.confidence = ZERO_CONF + 0.001;

	PositifLayer::handleStopMessage(msg);

	send(returnMsg);
}

void Node_Savvides_Mob::anchor(cMessage * msg)
{
	switch (msg->kind()) {
	case MSG_ANCHOR:
		if (new_anchor(msg)) {
			// Don't need to do anything. Bit of a hack.
			// new_anchor has a sideeffect causing it to propagate
			// the anchor information.
		}
		break;

	case MSG_POSITION:
		// We don't care about what the others do
		break;

	default:
		error("anchor(): unexpected message kind: %d", msg->kind());
		break;
	}
}


void Node_Savvides_Mob::unknown(cMessage * msg)
{
	switch (msg->kind()) {
	case MSG_ANCHOR:
		if (new_anchor(msg)) {
			savvides();	// Will do the bounding box thing, or just return if too few anchors are known.
			resetTimer(bc_position);
			seqno[MSG_POSITION]++;
		}
		break;

	case MSG_POSITION:
		// Maybe we're only interested in the initial estimate this time.
		if (!do_2nd_phase)
			return;

		update_neighbor(msg);

		if (neighbors.length() > nr_dims) {
			// Don't perform triangulation yet, wait for other updates
			if (comp_position == NULL) {
				comp_position = timer(1, TIMER_DO_TRI);
				addTimer(comp_position);
			}
			resetTimer(comp_position);
		}
		break;

	default:
		error("unknown(): unexpected message kind: %d", msg->kind());
		break;
	}
}


double Node_Savvides_Mob::true_pos_triangulate(void)
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

	return (res < 0 || res > range ? -1 :
		distance(pos, node[me].true_pos) / range);
}

void Node_Savvides_Mob::do_triangulation(void *arg)
{
	int n = neighbors.length();
	assert(n > nr_dims);
	FLOAT** pos_list = new FLOAT*[n + 1];
	FLOAT* range_list = new FLOAT[n + 1];
	FLOAT* weights = new FLOAT[n];
	Position pos;

#ifndef NDEBUG
	EV << node[me].ID << ": triangulate with nodes (out of " << n << "):";
#endif
	int i = 0;
	FLOAT sum_conf = 0;
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();
		double w = neighbor->twin ? LOW_CONF / 8 : neighbor->confidence;

		pos_list[i] = neighbor->position;
		range_list[i] = neighbor->distance;
		weights[i] = w;
		sum_conf += w;
		i++;
#ifndef NDEBUG
		EV << ' ' << node[neighbor->idx].ID << (neighbor->
							twin ? "t" : "") << '@'
		    << neighbor->distance;
#endif
	}
#ifndef NDEBUG
	EV << "\n";
#endif

	pos_list[i] = pos;
	FLOAT res = triangulate(i, pos_list, range_list, weights, me);

	tria_data * data = new tria_data;
	data->res = res;
	data->sum_conf = sum_conf;
	data->neighbor_count = i;
	for (int d = 0; d < nr_dims; d++)
		data->pos[d] = pos[d];
	timer_info timer = setRepeatTimer(i * nr_dims * nr_dims * msec, 1);
	triangulation_timers.push_back(timer);
	setContextPointer(timer, data);
	setContextDestructor(timer, delete_tria_data);

//      wait(i * nr_dims * nr_dims * msec);

// 	// Filter out moves that violate the convex constraints imposed
// 	// by (distant) anchors and neighbors
// 	if (res >= 0 && !inside_rectangle(pos)) {
// 		//        savvides();
// 		res = -2;
// 	} else if (res >= 0 && !inside_neighbors_range(pos)) {
// 		res = -3;
// 	}

// 	if (res < 0 || res > range) {
// 		if (confidence > ZERO_CONF) {
// 			confidence = ZERO_CONF;
// 			status = STATUS_BAD;

// 			resetTimer(bc_position);
// 			seqno[MSG_POSITION]++;
// 		}
// 	} else {
// 		// Anchors must have a solid confidence compared to unknowns, so bound
// 		// derived confidence
// 		double conf = Min(0.5, sum_conf / i);

// 		// Only take action on significant moves
// 		if (distance(position, pos) > accuracy) {
// 			bool significant = false;

// 			if (res > 1.1 * residu && confidence > 1.1 * LOW_CONF) {
// 				conf = LOW_CONF;
// 				// significant = true;
// 			}
// 			// We accept bad moves occasionally to get out of local minima
// 			if (res < residu || uniform(0, 1) < ACCEPT) {
// 				// record significant improvement and tell others
// 				memmove(position, pos, sizeof(Position));
// 				residu = res;
// #ifndef NDEBUG
// 				if (res < residu) {
// 					EV << node[me].
// 					    ID << ": UPDATE pos to " <<
// 					    pos2str(position) << " (" << 100 *
// 					    distance(position,
// 						     node[me].true_pos) /
// 					    range << "% error)" << "\n";
// 				} else {
// 					EV << node[me].
// 					    ID << ": ESCAPE pos to " <<
// 					    pos2str(position) << " (" << 100 *
// 					    distance(position,
// 						     node[me].true_pos) /
// 					    range << "% error)" << "\n";
// 				}
// #endif
// 				significant = true;
// 			} else {
// #ifndef NDEBUG
// 				EV << node[me].
// 				    ID << ": REJECT move (" << res << " > " <<
// 				    residu << ")\n";
// #endif
// 			}
// 			// Go tell others. Don't send message out directly, but use
// 			// retransmit queue instead. This will reduce the number of
// 			// xmitted msgs since multiple (buffered) incoming msgs
// 			// then yield one outgoing msg.

// 			if (significant) {
// 				resetTimer(bc_position);
// 				seqno[MSG_POSITION]++;
// 			}
// 		}
// 		confidence = (3 * confidence + conf) / 4;
// 		status = confidence > CONF_THR ? STATUS_POSITIONED : STATUS_BAD;
// 	}
	delete[] pos_list;
	delete[] range_list;
	delete[] weights;
}

void Node_Savvides_Mob::remove_triangulation_timer(timer_info timer) {
	list<timer_info>::iterator current;
	for (current = triangulation_timers.begin();
	     current != triangulation_timers.end();
	     current ++) {
		if (*current == timer) {
			triangulation_timers.erase(current);
			deleteRepeatTimer(timer);
			return;
		}
	}
	error("Non-existing triangulation_timer %d", timer);
	abort();
}

// typedef struct {FLOAT res; Position pos;} tria_data;
void Node_Savvides_Mob::delayed_do_triangulation(timer_info timer, void * _data) {
	tria_data * data = (tria_data *)_data;
	FLOAT res = data->res;
	FLOAT sum_conf = data->sum_conf;
	int i = data->neighbor_count;
	Position pos;
	for (int d = 0; d < nr_dims; d++)
		pos[d] = data->pos[d];

	remove_triangulation_timer(timer);

	// Filter out moves that violate the convex constraints imposed
	// by (distant) anchors and neighbors
	if (res >= 0 && !inside_rectangle(pos)) {
		//        savvides();
		res = -2;
	} else if (res >= 0 && !inside_neighbors_range(pos)) {
		res = -3;
	}

	if (res < 0 || res > range) {
		if (confidence > ZERO_CONF) {
			confidence = ZERO_CONF;
			status = STATUS_BAD;

			resetTimer(bc_position);
			seqno[MSG_POSITION]++;
		}
	} else {
		// Anchors must have a solid confidence compared to unknowns, so bound
		// derived confidence
		double conf = Min(0.5, sum_conf / i);

		// Only take action on significant moves
		if (distance(position, pos) > accuracy) {
			bool significant = false;

			if (res > 1.1 * residu && confidence > 1.1 * LOW_CONF) {
				conf = LOW_CONF;
				// significant = true;
			}
			// We accept bad moves occasionally to get out of local minima
			if (res < residu || uniform(0, 1) < ACCEPT) {
				// record significant improvement and tell others
				memmove(position, pos, sizeof(Position));
				residu = res;
#ifndef NDEBUG
				if (res < residu) {
					EV << node[me].
					    ID << ": UPDATE pos to " <<
					    pos2str(position) << " (" << 100 *
					    distance(position,
						     node[me].true_pos) /
					    range << "% error)" << "\n";
				} else {
					EV << node[me].
					    ID << ": ESCAPE pos to " <<
					    pos2str(position) << " (" << 100 *
					    distance(position,
						     node[me].true_pos) /
					    range << "% error)" << "\n";
				}
#endif
				significant = true;
			} else {
#ifndef NDEBUG
				EV << node[me].
				    ID << ": REJECT move (" << res << " > " <<
				    residu << ")\n";
#endif
			}
			// Go tell others. Don't send message out directly, but use
			// retransmit queue instead. This will reduce the number of
			// xmitted msgs since multiple (buffered) incoming msgs
			// then yield one outgoing msg.

			if (significant) {
				resetTimer(bc_position);
				seqno[MSG_POSITION]++;
			}
		}
		confidence = (3 * confidence + conf) / 4;
		status = confidence > CONF_THR ? STATUS_POSITIONED : STATUS_BAD;
	}
}
bool Node_Savvides_Mob::new_anchor(cMessage * msg)
{
	int num_anchors = msg->par("num_anchors");
	double distance = (double) msg->par("distance");
	char parname[100] = "";
	bool changed = false;

	for (int i = 0; i < num_anchors; i++) {
		sprintf(parname, "anchor_%d", i);
		anchor_info *anchor = new anchor_info;
		get_struct2(msg, parname, anchor);
		anchor->path_dst += distance;

		if (anchor->idx == me)
			continue;

		// Check if we received this one before
		bool found = false;
		anchor_info *old_anchor = NULL;
		for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
			old_anchor = (anchor_info *) iter();
			if (anchor->idx == old_anchor->idx) {
				/* Let anchors be differentiated by their position */
				if (PositifLayer::distance(anchor->position, old_anchor->position) == 0.0) {
					found = true;
					break;
				}
			}
		}

		if (!found) {
			if (anchors.length() < flood_limit || flood_limit == -1)
				anchor->flood = true;
			else
				anchor->flood = false;
			anchor->last_hop_idx = msg->par("src");
			anchor->cnt = reps;
			resetTimer(bc_anchor);
			changed = true;
			anchors.insert(anchor);
			update_rectangle(anchor);
			logprintf("new anchor(nr %d) %d from %d\n",
				  anchors.length() + 1, anchor->idx,
				  anchor->last_hop_idx);
		} else {
			if (anchor->path_dst < old_anchor->path_dst) {	// Found it, but the new one has a shorter path.
				old_anchor->path_dst = anchor->path_dst;
				old_anchor->last_hop_idx = msg->par("src");
				old_anchor->cnt = reps;
				resetTimer(bc_anchor);
				changed = true;
				update_rectangle(anchor);
				logprintf("updated anchor(nr %d) %d from %d\n",
					  anchors.length() + 1, anchor->idx,
					  anchor->last_hop_idx);
				/* @todo Shouldn't anchor be deleted here also? */
			} else
				delete(anchor);
		}
	}

	return changed;
}


void Node_Savvides_Mob::update_rectangle(anchor_info * anchor)
{
	// intersect anchor's "square" with existing convex rectangle
	for (int d = 0; d < nr_dims; d++) {
		FLOAT left = anchor->position[d] - anchor->path_dst;
		FLOAT right = anchor->position[d] + anchor->path_dst;

		if (!valid_rectangle) {
			rectangle.min[d] = left;
			rectangle.max[d] = right;
		} else {
			rectangle.min[d] = Min(rectangle.min[d], left);
			rectangle.max[d] = Max(rectangle.max[d], right);
		}
	}
	valid_rectangle = true;
}


bool Node_Savvides_Mob::inside_rectangle(Position pos)
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


bool Node_Savvides_Mob::inside_neighbors_range(Position pos)
{
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();

		if (neighbor->confidence > 2 * LOW_CONF &&
		    distance(pos, neighbor->position) > range) {
			return false;
		}
	}
	return true;
}


void Node_Savvides_Mob::update_neighbor(cMessage * msg)
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
		neighbor->nr_nghbrs = 0;
		neighbor->twin = false;
		neighbors.insert(neighbor);

		// Update summary
		assert(summary.nr_nghbrs == neighbors.length() - 1);

		if (summary.nr_nghbrs > 0) {
			delete summary.nghbr_idx;
		}
		int n = ++summary.nr_nghbrs;
		summary.nghbr_idx = new int[n];
		int i = 0;
		for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
			summary.nghbr_idx[i++] = ((nghbor_info *) iter())->idx;
		}
		summary_update = true;

		// The summary is piggybacked on ordinary MSG_POSITION msgs, so
		// reset the associated timer if that has already been activated
		if (bc_position != NULL) {
			resetTimer(bc_position);
			/* @todo Zorgt dit voor teveel broadcast berichten? */
			seqno[MSG_POSITION]++;
		}
	}

	get_struct(msg, "pos", *(Position *) neighbor->position);
	neighbor->distance = (double) msg->par("distance");
	if (!neighbor->twin)
		neighbor->confidence = msg->par("confidence");

	int n = msg->par("#neighbors");
	if (n > 0) {
		if (neighbor->nr_nghbrs > 0) {
			delete neighbor->nghbr_idx;
		}
		neighbor->nr_nghbrs = n;
		neighbor->nghbr_idx = new int[n];
		get_array(msg, "id", neighbor->nghbr_idx, n);

		// Update may introduce new twins and/or remove old twins
		// Simply check all pairs for twins (updating is too difficult)
		for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
			nghbor_info *m = (nghbor_info *) iter();

			m->twin = false;
		}
		for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
			nghbor_info *m = (nghbor_info *) iter();

			// Skip anchors
			if (m->confidence == 1) {
				continue;
			}

			if (twins(m, &summary)) {
				i_am_a_twin = m->twin = true;
			}

			for (cLinkedListIterator iter(neighbors); !iter.end();
			     iter++) {
				nghbor_info *k = (nghbor_info *) iter();

				// Skip anchors
				if (k->confidence == 1) {
					continue;
				}

				if (m->idx != k->idx && twins(m, k)) {
					m->twin = k->twin = true;
				}
			}
		}
	}
}



void Node_Savvides_Mob::sendPosition(void *arg)
{
	// Maybe we're only interested in the initial estimate this time.
	if (!do_2nd_phase)
		return;

	// Wait some time for hop-TERRAIN to stabilize
#if 1
	if (bc_pos_delay > 0) {
		bc_pos_delay--;
#else
	if (simTime() < 99) {
#endif
		resetTimer(bc_position);
		return;
	}

	if ((status != STATUS_ANCHOR) && neighbors.length() <= nr_dims) {
		// Find out if I am a sound node
		bool* sound = new bool[num_nodes];

		for (int n = 0; n < num_nodes; n++)
			sound[n] = false;

		for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
			sound[((nghbor_info *) iter())->idx] = true;
		}
		for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
			sound[((anchor_info *) iter())->last_hop_idx] = true;
		}

		int cnt = 0;
		for (int n = 0; n < num_nodes; n++)
			if (sound[n])
				cnt++;

		delete[] sound;

		if (cnt <= nr_dims) {
			return;
		}
	}

	if (refine_limit > -1 && refine_count >= refine_limit)
		return;

	refine_count++;

	cMessage *msg = new cMessage("POSITION", MSG_POSITION);
	msg->addPar("confidence") = confidence;
	add_struct(msg, "pos", position);

	if (summary_update) {
		msg->addPar("#neighbors") = summary.nr_nghbrs;
		add_array(msg, "id", summary.nghbr_idx, summary.nr_nghbrs);
		summary_update = false;
	} else {
		msg->addPar("#neighbors") = 0;
	}

	char tmp[100];
	sprintf(tmp, "p=%d,%d;i=ball_vs",
		(int) (100 * position[0] / sqrt(area)),
		(int) (100 * position[1] / sqrt(area)));
	setDisplayString(dispSUBMOD, (const char *) tmp, true);

	send(msg);
}


bool Node_Savvides_Mob::twins(nghbor_info * a, nghbor_info * b)
{
	if (a->nr_nghbrs != b->nr_nghbrs)
		return false;

	bool* nghbr_a = new bool[num_nodes];

	for (int i = 0; i < num_nodes; i++) {
		nghbr_a[i] = false;
	}
	for (int i = 0; i < a->nr_nghbrs; i++) {
		nghbr_a[a->nghbr_idx[i]] = true;
	}

	if (!nghbr_a[b->idx]) {
		delete[] nghbr_a;
		return false;
	}

	for (int i = 0; i < b->nr_nghbrs; i++) {
		int n = b->nghbr_idx[i];

		if (n != a->idx && !nghbr_a[n]) {
			delete[] nghbr_a;		
			return false;
		}
	}
	delete[] nghbr_a;	
	return true;
}


void Node_Savvides_Mob::sendAnchor(void *anc)
{
	cMessage *msg = new cMessage("ANCHOR", MSG_ANCHOR);

	int i = 0;

	char parname[100] = "";
	for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
		anchor_info *anchor = (anchor_info *) iter();
		if (anchor->cnt != 0 && anchor->flood) {
			sprintf(parname, "anchor_%d", i);
			add_struct2(msg, parname, anchor);
			i++;
			anchor->cnt--;
			logprintf("send anchor(%d) %d\n", anchor->idx);
		}
	}
	msg->addPar("num_anchors") = i;

	if (i > 0) {
		seqno[MSG_ANCHOR]++;	// kludge: no filtering iso seqno per anchor event
		send(msg);
	} else
		delete(msg);
}


//
// Two algorithms are executed: the first is Savvides_Mob' initial estimation
// phase, which creates a bounding box using the ranges from the anchors
// and takes the center of this box as position estimate. (phase1_err)
// The second takes a weighted average of all anchors. (phase2_err)
// Which value is used in the end is determined by the constant PHASE1
//
void Node_Savvides_Mob::savvides(void)
{
	int n = anchors.length();

	if (n >= phase1_min_anchors) {

#ifndef NDEBUG
		EV << node[me].
		    ID << ": savvides' phase2 initialization with anchors:";
#endif

		used_anchors = (n < phase1_max_anchors
				|| phase1_max_anchors ==
				-1) ? n : phase1_max_anchors;
		FLOAT** pos_list = new FLOAT*[used_anchors + 1];
		if (range_list)
			delete range_list;
		range_list = new FLOAT[used_anchors + 1];
		if (real_range_list)
			delete real_range_list;
		real_range_list = new FLOAT[used_anchors + 1];	// Used for debug output only
		int* idx_list = new int[used_anchors];

		int i = 0;
		for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
			anchor_info *anchor = (anchor_info *) iter();
			int store_at = -1;
			FLOAT range = 0;

#ifndef NDEBUG
			EV << " " << node[anchor->idx].ID << "@" << anchor->cnt;
#endif
			range = anchor->path_dst;

			if (i < used_anchors) {
				store_at = i;
				i++;
			} else {
				for (int j = 0; j < used_anchors; j++)
					if ((store_at < 0
					     && range_list[j] > range)
					    || (store_at >= 0
						&& range_list[j] >
						range_list[store_at])) {
						store_at = j;
					}
			}

			/* @todo The true_pos here is requested from the node structure, but
			 * this cannot be done with the new anchor list. The new anchor list
			 * must have this information stored internally.
			 */
			if (store_at >= 0) {
				pos_list[store_at] = anchor->position;
				range_list[store_at] = range;
				real_range_list[store_at] =
				    distance(node[me].true_pos,
					     anchor->position);
// 					     node[anchor->idx].true_pos);
				idx_list[store_at] = anchor->idx;
			}
		}

		Position next;
		pos_list[used_anchors] = next;

		// Calculate performance data for range error
		FLOAT sum_err = 0, sum_rel_err = 0, sum_abs_err =
		    0, sum_abs_rel_err = 0, sum_range = 0;
		for (i = 0; i < used_anchors; i++) {
			sum_err += range_list[i] - real_range_list[i];
			sum_rel_err +=
			    (range_list[i] -
			     real_range_list[i]) / real_range_list[i];
			sum_abs_err += fabs(range_list[i] - real_range_list[i]);
			sum_abs_rel_err +=
			    fabs(range_list[i] -
				 real_range_list[i]) / real_range_list[i];
			sum_range += range_list[i];
		}
		node[me].perf_data.anchor_range_error = sum_err / used_anchors;
		node[me].perf_data.rel_anchor_range_error =
		    sum_rel_err / used_anchors;
		node[me].perf_data.abs_anchor_range_error =
		    sum_abs_err / used_anchors;
		node[me].perf_data.abs_rel_anchor_range_error =
		    sum_abs_rel_err / used_anchors;
		node[me].perf_data.anchor_range = sum_range / used_anchors;
		node[me].perf_data.anchor_range_error_count = used_anchors;

		FLOAT res;
		if (tri_alg == 0)
			res =
			    triangulate(used_anchors, pos_list, range_list,
					NULL, me);
		else
			res =
			    savvides_minmax(used_anchors, pos_list, range_list,
					    NULL, me);

		logprintf(" %4.0f,%4.0f\n", next[0], next[1]);

//              wait(n * nr_dims * nr_dims * msec);

		if (0 <= res && res <= range) {
			memmove(position, next, sizeof(Position));
			node[me].perf_data.phase1_err =
			    distance(position, node[me].true_pos) / range;

			confidence = LOW_CONF;
			status = STATUS_POSITIONED;	// After Savvides initialisation, a node is positioned regardless of its confidence.
#ifndef NDEBUG
			EV << node[me].
			    ID << ": UPDate pos to " << pos2str(position)
			    << " (" << 100 *
			    node[me].perf_data.phase1_err << "% error)\n";
#endif

			if (bc_position == NULL) {
				bc_position = timer(1, TIMER_SND_POS);
				bc_pos_delay = POS_DELAY;
				addTimer(bc_position);
			}
		}

		logprintf(" p1->%4.0f,%4.0f (%1.1f) using ", position[0],
			  position[1], node[me].perf_data.phase1_err);
		for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
			anchor_info *anchor = (anchor_info *) iter();
			bool used = false;
			int j;
			for (j = 0; j < used_anchors; j++)
				if (idx_list[j] == anchor->idx) {
					used = true;
					break;
				}
			if (used)
				logprintf
				    (" -->(%d:(%4.0f,%4.0f)@%4.0fd,%4.0fr)<-- ",
				     anchor->idx, anchor->position[0],
				     anchor->position[1], range_list[j],
				     distance(node[me].true_pos,
					      node[anchor->idx].true_pos));
			else
				logprintf(" (%d:(%4.0f,%4.0f@%4.0fr) ",
					  anchor->idx, anchor->position[0],
					  anchor->position[1],
					  distance(node[me].true_pos,
						   node[anchor->idx].true_pos));
		}
		logprintf("\n");
		delete[] idx_list;
		delete[] pos_list;
	}
}
