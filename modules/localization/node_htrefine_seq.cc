#include "PositifLayer.h"
#include <string.h>

// Node subclass containing Hop-TERRAIN for initialisation and Refine
// for refinement

#define msec		(1e-3)
#define ACCEPT		0.1
#define POS_DELAY	9

#define CONF_THR	(LOW_CONF*1.01)
#define ZERO_CONF       0.01
#define LOW_CONF        0.1

// Define timer types
#define TIMER_SND_POS 0
#define TIMER_SND_ANC 1
#define TIMER_SND_CAL 2
#define TIMER_DO_TRI  3
#define TIMER_COUNT 4

// Define message types
#define MSG_ANCHOR    MSG_TYPE_BASE+0
#define MSG_POSITION  MSG_TYPE_BASE+1
#define MSG_CALIBRATE MSG_TYPE_BASE+2

typedef struct {
	int idx, hop_cnt;
	FLOAT last_hop_dst;
	int last_hop_idx;
	Position position;
	timer_info *bc_timer;
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



class Node_HTRefine_SEQ:public PositifLayer {
	double accuracy;
	enum { NO, SIMPLE_R, SIMPLE_AVG, CALIB_HOPS, CALIB_RSSI } terrain;

	FLOAT residu;
	bool calibrated;
	double avg_hop_dist;
	int calib_src;
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
	timer_info *bc_calib;
	timer_info *comp_position;

	void anchor(cMessage * msg);
	void unknown(cMessage * msg);

	void sendPosition(void *arg = NULL);
	void sendCalib(void *arg = NULL);
	void sendAnchor(void *anc);
	void do_triangulation(void *arg = NULL);

	void hop_based_triangulation(void);
	bool new_anchor(cMessage * msg);
	void update_neighbor(cMessage * msg);
	void calibrate(void);

	void update_rectangle(anchor_info * anchor);
	bool inside_rectangle(Position pos);
	bool inside_neighbors_range(Position pos);
	bool twins(nghbor_info * a, nghbor_info * b);


	double true_pos_triangulate(void);

	bool has_token(void);
	void release_token(void);

      public:
	 Module_Class_Members(Node_HTRefine_SEQ, PositifLayer, 0)
	    // Implement Node's abstract functions.
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

Define_Module_Like(Node_HTRefine_SEQ, PositifLayer);

void Node_HTRefine_SEQ::init(void)
{
	algorithm = 3;		// Output on raw data line only.
	version = 1;

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
void Node_HTRefine_SEQ::handleTimer(timer_info * timer)
{
	switch (*timer) {
	case TIMER_SND_POS:
		sendPosition(contextPointer(*timer));
		break;

	case TIMER_SND_ANC:
		sendAnchor(contextPointer(*timer));
		break;

	case TIMER_SND_CAL:
		sendCalib(contextPointer(*timer));
		break;

	case TIMER_DO_TRI:
//              timer->cnt++;   // Keep timer running in triangulation loop
		resetTimer(timer);
		if (has_token()) {
			do_triangulation(contextPointer(*timer));
			release_token();
		}
		break;
	}
}

bool Node_HTRefine_SEQ::has_token(void)
{
	// If we have it, it's simple.
	if (node[me].token)
		return true;

	// If we don't, and nobody else has, we create it.
	// (Yes, this isn't exactly safe...)
	for (int i = 0; i < num_nodes; i++) {
		if (node[i].token)
			return false;
	}

	node[me].token = true;
	return true;
}

void Node_HTRefine_SEQ::release_token(void)
{
	if (node[me].token) {
		// Find the first next node that is ready for the token (
		for (int i = 1; i < num_nodes; i++)	// start at i=1, so skip the current node
			if (node[(i + me) % num_nodes].wants_token) {
				node[(i + me) % num_nodes].token = true;
				node[me].token = false;
				return;
			}
	}
	// We'll just keep it if no one wants it.
}

void Node_HTRefine_SEQ::handleMessage(cMessage * msg, bool newNeighbor)
{
	if (msg->kind() == MSG_POSITION) {
		int src;
		src = msg->par("src");
		fprintf(stderr, "in: %d <- %d\n", me, src);
	}
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
	if (status == STATUS_ANCHOR) {
		anchor(msg);
	} else {
		unknown(msg);
	}
}

void Node_HTRefine_SEQ::handleStartMessage(cMessage * msg)
{
	// query module parameters
	accuracy = par("accuracy");

	const char *mode = par("terrain");
	if (strcmp(mode, "NO") == 0) {
		terrain = NO;
	} else if (strcmp(mode, "simple R") == 0) {
		terrain = SIMPLE_R;
	} else if (strcmp(mode, "simple avg") == 0) {
		terrain = SIMPLE_AVG;
	} else if (strcmp(mode, "calib hops") == 0) {
		terrain = CALIB_HOPS;
	} else if (strcmp(mode, "calib RSSI") == 0) {
		terrain = CALIB_RSSI;
	} else {
		error("incorect terrain mode '%s'", mode);
	}

	residu = 10 * range;
	calibrated = false;

	get_struct(msg, "position", position);	// Waarom is deze nodig als de node geen anchor is?
	comp_position = NULL;
	bc_position = NULL;
	bc_calib = NULL;

	valid_rectangle = false;

	summary.idx = me;
	summary.nr_nghbrs = 0;
	summary_update = false;
	i_am_a_twin = false;
	confidence = (status == STATUS_ANCHOR) ? 1 : ZERO_CONF;

	// Is there an initialization (terrain) phase?
	if (terrain == NO) {
		// All nodes should refine their positions right from the start
		bc_position = timer(1, TIMER_SND_POS);
		bc_pos_delay = POS_DELAY;
		addTimer(bc_position);
	} else if (status == STATUS_ANCHOR) {
		// Anchor nodes know where they are, so initialization starts from here
		anchor_info *ME = new anchor_info;

		ME->idx = me;
		ME->hop_cnt = 0;
		memcpy(ME->position, position, sizeof(Position));
		ME->bc_timer = timer(reps, TIMER_SND_ANC, ME);

		addTimer(ME->bc_timer);

		bc_position = timer(reps, TIMER_SND_POS);
		bc_pos_delay = 0;
		addTimer(bc_position);
	}
}

void Node_HTRefine_SEQ::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);

	// Use the 2nd phase slot to measure the error using Koens method.
	if (status != STATUS_ANCHOR)
		node[me].perf_data.phase2_err = true_pos_triangulate();

	// Set final confidence to low when there aren't enough neighbors
	if (status != STATUS_ANCHOR && neighbors.length() <= nr_dims)
		node[me].perf_data.confidence = ZERO_CONF + 0.001;

	send(returnMsg);
}

void Node_HTRefine_SEQ::anchor(cMessage * msg)
{
	switch (msg->kind()) {
	case MSG_ANCHOR:
		if (new_anchor(msg)) {
			calibrate();
		}
		break;

	case MSG_CALIBRATE:
	case MSG_POSITION:
		// We don't care about what the others do
		break;

	default:
		error("anchor(): unexpected message kind: %d", msg->kind());
		break;
	}
}


void Node_HTRefine_SEQ::unknown(cMessage * msg)
{
	switch (msg->kind()) {
	case MSG_ANCHOR:
		if (new_anchor(msg)) {
			if (calibrated) {
				hop_based_triangulation();
				resetTimer(bc_position);
				seqno[MSG_POSITION]++;
			}
		}
		break;

	case MSG_CALIBRATE:{
			int src = msg->par("src");
			if (!calibrated) {
				bc_position = timer(1, TIMER_SND_POS);
				bc_pos_delay = POS_DELAY;
				addTimer(bc_position);
				bc_calib = timer(reps, TIMER_SND_CAL);
				addTimer(bc_calib);

				calib_src = src;
				calibrated = true;
			}
			// We must flood the network and respond to updates
			// We will stick to the first that calibrates us (and consequently
			// use the same flooding scheme on all iterations)
			if (src == calib_src) {
				avg_hop_dist = msg->par("avg-hop-dist");

				// Get initial position estimate
				hop_based_triangulation();

				// Go tell others
				resetTimer(bc_position);
				seqno[MSG_POSITION]++;
				resetTimer(bc_calib);
				seqno[MSG_CALIBRATE]++;
			}
		}
		break;

	case MSG_POSITION:
		update_neighbor(msg);

		if (calibrated && neighbors.length() > nr_dims) {
			// Don't perform triangulation yet, wait for other updates
			if (comp_position == NULL) {
				// This timer will never be stopped by handleTimer. Bit of a hack to keep looping the sequence.
				comp_position = timer(1, TIMER_DO_TRI);
				node[me].wants_token = true;
				addTimer(comp_position);
			}
		}
		break;

	default:
		error("unknown(): unexpected message kind: %d", msg->kind());
		break;
	}
}


double Node_HTRefine_SEQ::true_pos_triangulate(void)
{
	int n = neighbors.length();
	FLOAT *pos_list[n + 1];
	FLOAT range_list[n + 1];
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

	return (res < 0 || res > range ? -1 :
		distance(pos, node[me].true_pos) / range);
}

void Node_HTRefine_SEQ::do_triangulation(void *arg)
{
	fprintf(stderr, "tri: %d\n", me);
	flops++;
	int n = neighbors.length();
	assert(n > nr_dims);
	FLOAT *pos_list[n + 1];
	FLOAT range_list[n + 1];
	FLOAT weights[n];
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

// 	wait(i * nr_dims * nr_dims * msec);

	// Filter out moves that violate the convex constraints imposed
	// by (distant) anchors and neighbors
	if (res >= 0 && !inside_rectangle(pos)) {
		hop_based_triangulation();
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


bool Node_HTRefine_SEQ::new_anchor(cMessage * msg)
{
	int idx = msg->par("anchor idx");
	int hop_cnt = msg->par("hop-cnt");

	if (idx == me)
		return false;

	for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
		anchor_info *anchor = (anchor_info *) iter();
		if (anchor->idx == idx) {
			if (hop_cnt < anchor->hop_cnt) {
				anchor->hop_cnt = hop_cnt;
				anchor->last_hop_dst =
				    (double) msg->par("distance");
				anchor->last_hop_idx = msg->par("src");
//EV << node[me].ID << ": update anchor, ID = " << node[idx].ID << " hop-cnt = " << hop_cnt << "\n";
				// Flood update to others WHEN needed (bc_timer != NULL)
				if (anchor->bc_timer != NULL)
					resetTimer(anchor->bc_timer);
				update_rectangle(anchor);
				return true;
			}
			return false;
		}
	}
//EV << node[me].ID << ": new anchor, ID = " << node[idx].ID << " hop-cnt = " << hop_cnt << "\n";
	anchor_info *anchor = new anchor_info;
	anchor->idx = idx;
	anchor->hop_cnt = hop_cnt;
	anchor->last_hop_dst = (double) msg->par("distance");
	anchor->last_hop_idx = msg->par("src");
	get_struct(msg, "pos", anchor->position);

	// Limit flooding, one additional anchor is enough
	if (anchors.length() <= nr_dims + 1) {
		anchor->bc_timer = timer(reps, TIMER_SND_ANC, anchor);
		addTimer(anchor->bc_timer);
	} else {
		anchor->bc_timer = NULL;
	}

	anchors.insert(anchor);

	update_rectangle(anchor);
	return true;
}


void Node_HTRefine_SEQ::update_rectangle(anchor_info * anchor)
{
	// intersect anchor's "square" with existing convex rectangle
	for (int d = 0; d < nr_dims; d++) {
		FLOAT left = anchor->position[d] - anchor->hop_cnt * range;
		FLOAT right = anchor->position[d] + anchor->hop_cnt * range;

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


bool Node_HTRefine_SEQ::inside_rectangle(Position pos)
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


bool Node_HTRefine_SEQ::inside_neighbors_range(Position pos)
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


void Node_HTRefine_SEQ::update_neighbor(cMessage * msg)
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


void Node_HTRefine_SEQ::hop_based_triangulation(void)
{
	int n = anchors.length();

	if (n > nr_dims) {
#ifndef NDEBUG
		EV << node[me].ID << ": triangulate with anchors:";
#endif
		FLOAT *pos_list[n + 1];
		FLOAT range_list[n + 1];

		int i = 0;
		for (cLinkedListIterator iter(anchors); !iter.end();
		     iter++, i++) {
			anchor_info *anchor = (anchor_info *) iter();

#ifndef NDEBUG
			EV << " " << node[anchor->idx].ID << "@" << anchor->
			    hop_cnt;
#endif
			pos_list[i] = anchor->position;
			switch (terrain) {
			case SIMPLE_R:
				range_list[i] = anchor->hop_cnt * range;
				break;

			case SIMPLE_AVG:
				range_list[i] = anchor->hop_cnt * range;
				break;

			case CALIB_HOPS:
				range_list[i] = anchor->hop_cnt * avg_hop_dist;
				break;

			case CALIB_RSSI:
				range_list[i] =
				    (anchor->hop_cnt - 1) * avg_hop_dist +
				    anchor->last_hop_dst;
				break;
			case NO:	// ignore
				break;
			}
		}

		Position next;
		pos_list[n] = next;
		FLOAT res = triangulate(n, pos_list, range_list, NULL, me);

// 		wait(n * nr_dims * nr_dims * msec);

		if (0 <= res && res <= range) {
			memmove(position, next, sizeof(Position));
			confidence = LOW_CONF;
			status = STATUS_POSITIONED;	// After Hop-TERRAIN, a node is positioned regardless of its confidence.
			residu = range;
			node[me].perf_data.phase1_err =
			    distance(position, node[me].true_pos) / range;
#ifndef NDEBUG
			EV << node[me].
			    ID << ": UPDate pos to " << pos2str(position)
			    << " (" << 100 *
			    node[me].perf_data.phase1_err << "% error)\n";
#endif
		}
	}
}


void Node_HTRefine_SEQ::calibrate(void)
{
	// Don't calibrate on the first anchor to counter msg loss (which
	// may lead to detours arriving first!)

	if (anchors.length() + 1 > nr_dims) {	// count me too
		FLOAT dist_sum = 0;
		int hop_sum = 0;

		for (cLinkedListIterator iter(anchors); !iter.end(); iter++) {
			anchor_info *anchor = (anchor_info *) iter();

			assert(anchor->idx != me);
			dist_sum += distance(position, anchor->position);
			hop_sum += anchor->hop_cnt;
		}
		avg_hop_dist = dist_sum / hop_sum;
#ifndef NDEBUG
		EV << node[me].
		    ID << ": CALIBRATE avg hop distance " << dist_sum /
		    hop_sum << "\n";
#endif
		if (!calibrated) {
			calibrated = true;
			bc_calib = timer(reps, TIMER_SND_CAL);
			addTimer(bc_calib);
		} else {
			resetTimer(bc_calib);
			seqno[MSG_CALIBRATE]++;
		}
	}
}


void Node_HTRefine_SEQ::sendPosition(void *arg)
{
	fprintf(stderr, "out: %d\n", me);
	// Maybe we're only interested in Hop-TERRAIN this time.
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
		bool sound[num_nodes];

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

		if (cnt <= nr_dims) {
			return;
		}
	}

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


bool Node_HTRefine_SEQ::twins(nghbor_info * a, nghbor_info * b)
{
	bool nghbr_a[num_nodes];

	if (a->nr_nghbrs != b->nr_nghbrs)
		return false;

	for (int i = 0; i < num_nodes; i++) {
		nghbr_a[i] = false;
	}
	for (int i = 0; i < a->nr_nghbrs; i++) {
		nghbr_a[a->nghbr_idx[i]] = true;
	}

	if (!nghbr_a[b->idx])
		return false;

	for (int i = 0; i < b->nr_nghbrs; i++) {
		int n = b->nghbr_idx[i];

		if (n != a->idx && !nghbr_a[n])
			return false;
	}
	return true;
}


void Node_HTRefine_SEQ::sendCalib(void *arg)
{
	cMessage *msg = new cMessage("CALIBRATE", MSG_CALIBRATE);
	msg->addPar("avg-hop-dist") = avg_hop_dist;

	send(msg);
}


void Node_HTRefine_SEQ::sendAnchor(void *anc)
{
	anchor_info *anchor = (anchor_info *) anc;

	cMessage *msg = new cMessage("ANCHOR", MSG_ANCHOR);
	msg->addPar("anchor idx") = anchor->idx;
	add_struct(msg, "pos", anchor->position);
	msg->addPar("hop-cnt") = anchor->hop_cnt + 1;

	seqno[MSG_ANCHOR]++;	// kludge: no filtering iso seqno per anchor event
	send(msg);
}
