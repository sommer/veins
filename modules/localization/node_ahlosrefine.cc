#include "PositifLayer.h"
#include <string.h>

// Node subclass containing AHLoSRefine iterative multilateration (not test,
// may not work properly)

#define msec		(1e-3)

#define CONF_THR	(LOW_CONF*1.01)
#define ZERO_CONF       0.01
#define LOW_CONF        0.1

// Define timer types
#define TIMER_SND_POS 0
#define TIMER_COUNT 1

// Define message types
#define MSG_POSITION MSG_TYPE_BASE+1

#define ACCEPT		0.1

typedef struct {
	int idx;
	double confidence;
	FLOAT distance;
	Position position;
} nghbor_info;

class Node_AHLoSRefine:public PositifLayer {
	FLOAT residu;
	double accuracy;
	cLinkedList neighbors;
	nghbor_info summary;
	bool summary_update;
	timer_info *bc_position;

	void unknown(cMessage * msg);
	void do_triangulation();
	void sendPosition();
	bool inside_neighbors_range(Position pos);


      public:
	 //Module_Class_Members(Node_AHLoSRefine, PositifLayer, 0)
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

//Define_Module_Like(Node_AHLoSRefine, PositifLayer);


void Node_AHLoSRefine::handleStartMessage(cMessage * msg)
{
	accuracy = par("accuracy");

	// Create the send position timer, but don't send yet.
	bc_position = timer(reps, TIMER_SND_POS);
//      bc_position->cnt = 0;
	cancelTimer(bc_position);
	addTimer(bc_position);

	if (status == STATUS_ANCHOR) {
		// Anchor nodes send start the algorithm by sending their positions
		resetTimer(bc_position);
	}
}

void Node_AHLoSRefine::handleMessage(cMessage * msg, bool newNeighbor)
{
	if (status == STATUS_ANCHOR || status == STATUS_POSITIONED) {
		// Anchors discard all incoming messages
	} else {
		// Unknowns collect positions from all anchors, and calculate their own position
		// when they know the position of 3 anchors. In that case they become anchors as well.
		unknown(msg);
	}
}

void Node_AHLoSRefine::handleTimer(timer_info * timer)
{
	switch (*timer) {
	case TIMER_SND_POS:
		sendPosition();
	}
}

void Node_AHLoSRefine::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);

	send(returnMsg);
}

// If we haven't received this node's position yet,
// (we shouldn't have, because all nodes only send it once)
// add it to the list. If we know the range and position of
// three nodes, this function will call the triangulation function,
// which in turn will schedule the sendPosition timer.
void Node_AHLoSRefine::unknown(cMessage * msg)
{
	int src = msg->par("src");
	nghbor_info *neighbor;
	bool found = false;

	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
		neighbor = (nghbor_info *) iter();

		if (neighbor->idx == src)
			found = true;
	}

	if (!found)		// Create new entry for this node
	{
		neighbor = new nghbor_info();
		neighbors.insert(neighbor);
	}
	// Update/write data
	neighbor->idx = src;
	neighbor->confidence = msg->par("confidence");
	neighbor->distance = (double) msg->par("distance");
	get_struct(msg, "pos", neighbor->position);

	// Check if we have nr_dims+1 or more neighbors
	if (neighbors.length() > nr_dims)
		// If so, call triangulation function.
		do_triangulation();
}


bool Node_AHLoSRefine::inside_neighbors_range(Position pos)
{
	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();

		if (neighbor->confidence > 2 * LOW_CONF &&
		    distance(pos, neighbor->position) > range) {
			return false;
		}
	}
	return true;
}


void Node_AHLoSRefine::do_triangulation(void)
{
	flops++;
	int n = neighbors.length();
	assert(n > nr_dims);
	FLOAT** pos_list = new FLOAT*[n + 1];
	FLOAT* range_list = new FLOAT[n + 1];
	FLOAT* weights = new FLOAT[n];
	Position pos;

	int i = 0;
	FLOAT sum_conf = 0;
	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();
		double w =
		    /* TODO: fix neighbor->twin ? LOW_CONF/8 : */
		    neighbor->confidence;

		pos_list[i] = neighbor->position;
		range_list[i] = neighbor->distance;
		weights[i] = w;
		sum_conf += w;
		i++;
	}

	pos_list[i] = pos;
	FLOAT res = triangulate(i, pos_list, range_list, weights, me);

//      wait(i * nr_dims * nr_dims * msec);

	// Filter out moves that violate the convex constraints imposed
	// by (distant) anchors and neighbors
	if (res >= 0 && !inside_neighbors_range(pos)) {
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
				significant = true;
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
	delete[] pos_list;
	delete[] range_list;
	delete[] weights;
}



void Node_AHLoSRefine::sendPosition()
{
	cMessage *msg = new cMessage("POSITION", MSG_POSITION);
	msg->addPar("confidence") = confidence;
	add_struct(msg, "pos", position);

	send(msg);
}

void Node_AHLoSRefine::init(void)
{
	algorithm = 2;		// Output on raw data line only.
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
