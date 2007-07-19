#include "PositifLayer.h"
#include <string.h>
#include <list>
// Example PositifLayer subclass

// Define timer types
#define TIMER_SND_POS 	0

#define MSG_POSITION MSG_TYPE_BASE+1

typedef struct {
	int idx;
	FLOAT distance;
	Position position;
} nghbor_info;

class Node_Example:public PositifLayer {
	double accuracy;
	std::list <nghbor_info *> neighbors;
	void sendPosition(void *arg = NULL);
	void update_neighbor(cMessage * msg);

      public:
	 Module_Class_Members(Node_Example, PositifLayer, 0);
	// Implement PositifLayer's abstract functions.
	virtual void init(void);
	virtual void handleRepeatTimer(unsigned int index);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

Define_Module_Like(Node_Example, PositifLayer);

void Node_Example::init(void)
{
	algorithm = 8;          // Output on raw data line only.
	version = 1;

	/* This is a hack. */
	status = STATUS_ANCHOR;
}

// Dispatch this timer to the proper handler function.
void Node_Example::handleRepeatTimer(unsigned int index)
{
	Enter_Method_Silent();

	switch (index) {
	case TIMER_SND_POS:
		sendPosition(NULL);
		break;
	}
}

void Node_Example::handleMessage(cMessage * msg, bool newNeighbor)
{
	// Call appropriate handler function
	switch (msg->kind()) {
	case MSG_POSITION:
		update_neighbor(msg);
		break;
	default:
		error("unknown(): unexpected message kind: %d", msg->kind());
		break;
	}
}

void Node_Example::handleStartMessage(cMessage * msg)
{
	if (status == STATUS_ANCHOR) {
		// Anchor nodes know where they are, so broadcast location to neighbours.
		setRepeatTimer(TIMER_SND_POS, me + 1, 1);
// 		timer(1, TIMER_SND_POS);
//              addTimer(bc_position);
	}
}

void Node_Example::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics

// PP: I don't think that it's valid to send a stop message.
//      cMessage *returnMsg = new cMessage("DONE", MSG_DONE);
//      PositifLayer::handleStopMessage(msg);
//      send(returnMsg);
	nghbor_info *neighbor = NULL;

	PositifLayer::handleStopMessage(msg);

	fprintf (stdout, "Neighbours of %d: ", findHost()->index());

	std::list<nghbor_info *>::const_iterator it;
	for (it = neighbors.begin(); it != neighbors.end(); it++) {
		fprintf (stdout, "%d; ", (*it)->idx);
	}
	fprintf (stdout, "\n");
}

void Node_Example::update_neighbor(cMessage * msg)
{
	nghbor_info *neighbor = NULL;
	int src = msg->par("src");
	bool found = false;

	std::list<nghbor_info *>::const_iterator it;
	for (it = neighbors.begin(); it != neighbors.end(); it++) {
		if ((*it)->idx == src) {
			found = true;
			break;
		}
	}

	if (!found) {
		neighbor = new nghbor_info;
		neighbor->idx = src;
		neighbors.push_back(neighbor);
	} else {
		neighbor = (*it);
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
