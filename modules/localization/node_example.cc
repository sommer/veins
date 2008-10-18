#include "PositifLayer.h"
#include <string.h>

// Example Node subclass

// Define timer types
#define TIMER_SND_POS 	0
#define TIMER_COUNT 1

#define MSG_POSITION MSG_TYPE_BASE+1

typedef struct {
	int idx;
	FLOAT distance;
	Position position;
} nghbor_info;

class Node_Example:public PositifLayer {
	double accuracy;

	cLinkedList neighbors;

	timer_info *bc_position;
	void sendPosition(void *arg = NULL);

	void update_neighbor(cMessage * msg);

      public:
	 //Module_Class_Members(Node_Example, PositifLayer, 0)
	    // Implement Node's abstract functions.
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

//Define_Module_Like(Node_Example, PositifLayer);

void Node_Example::init(void)
{
	algorithm = 8;		// Output on raw data line only.
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
void Node_Example::handleTimer(timer_info * timer)
{
	switch (*timer) {
	case TIMER_SND_POS:
		sendPosition(contextPointer(*timer));
		break;
	}
}

void Node_Example::handleMessage(cMessage * msg, bool newNeighbor)
{
	// Call appropriate handler function
	switch (msg->getKind()) {

	case MSG_POSITION:
		update_neighbor(msg);
		break;

	default:
		error("unknown(): unexpected message kind: %d", msg->getKind());
		break;
	}
}

void Node_Example::handleStartMessage(cMessage * msg)
{
	if (status == STATUS_ANCHOR) {
		// Anchor nodes know where they are, so broadcast location to neighbours.
		bc_position = timer(1, TIMER_SND_POS);
		addTimer(bc_position);
	}
}

void Node_Example::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);
	PositifLayer::handleStopMessage(msg);
	send(returnMsg);

	fprintf (stdout, "Neighbours of %d: ", findHost()->getIndex());
	
	for (cLinkedList::Iterator iter = neighbors; !iter.end(); iter++) {
		nghbor_info *nb = (nghbor_info *) iter();
		fprintf (stdout, "%d; ", nb->idx);
	}
	fprintf (stdout, "\n");
}

void Node_Example::update_neighbor(cMessage * msg)
{
	nghbor_info *neighbor = NULL;
	int src = msg->par("src");

	bool found = false;
	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
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

	assert(msg->hasPar("pos"));
	get_struct(msg, "pos", neighbor->position);
	neighbor->distance = (double) msg->par("distance");
}

void Node_Example::sendPosition(void *arg)
{
	cMessage *msg = new cMessage("POSITION", MSG_POSITION);
	add_struct(msg, "pos", position);
	send(msg);
}
