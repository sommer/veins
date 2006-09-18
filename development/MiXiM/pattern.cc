#include "message.h"
#include "pattern.h"
#include "node.h"

void Pattern::initialize() {
	MiximBaseModule::initialize();
	turned_on = !is_active;
	timeout1_msg = new cMessage("pattern timeout 1");
	timeout2_msg = new cMessage("pattern timeout 2");
	node_id = ((Node *) parentModule()->parentModule()->parentModule()->submodule("node"))->getNodeId();
	stat_tx = stat_rx = stat_delay_count = 0;
	stat_delay_total = 0.0;
	init();
	if(turned_on)
		activated();
}

void Pattern::finish() {
	printf(PRINT_STATS, "stats: tx=%d rx=%d delay=%lf",
			stat_tx,
			stat_rx,
			stat_delay_total / (double)stat_delay_count);
	MiximBaseModule::finish();
}

Pattern::~Pattern() {
	cancelAndDelete(timeout1_msg);
	cancelAndDelete(timeout2_msg);
}

void Pattern::handleMessage(cMessage * msg) {
	assert(msg);
	if(msg == timeout1_msg) {
		timeout();
		return;
	}
	if(msg == timeout2_msg) {
		timeout2();
		return;
	}
	switch(msg->kind()) {
		case RX:
			assert_type(msg, Packet *);
			rx((Packet *)msg);
			break;
		case BECOME_ACTIVE:
			if(is_active) {
				assert(!turned_on);
				turned_on = 1;
				activated();
			}
			delete msg;
			break;
		case BECOME_IDLE:
			if(is_active) {
				assert(turned_on);
				turned_on = 0;
				deActivated();
			}
			delete msg;
			break;
		case SEND_LATENCY:
			//~ printf(PRINT_STATS, "%g", (double) msg->par("delay"));
			rxDelay(msg);
			delete msg;
			break;
		default:
			printf(PRINT_APP, "kind is %d", msg->kind());
			assert(false); // unknown kind
	}
}

void Pattern::rx(Packet * msg) {
	delete msg;
}

void Pattern::tx(Packet * msg) {
	msg->setKind(TX);
	assert(msg->to != node_id);
	send(msg, "out");
}

void Pattern::setTimeout(double time) {
	assert(timeout1_msg);
	cancelTimeout();
	scheduleAt(simTime() + time, timeout1_msg);
}

void Pattern::setTimeout2(double time) {
	assert(timeout2_msg);
	cancelTimeout2();
	scheduleAt(simTime() + time, timeout2_msg);
}

void Pattern::cancelTimeout() {
	assert(timeout1_msg);
	if(timeout1_msg->isScheduled()) 
		cancelEvent(timeout1_msg);
}

void Pattern::cancelTimeout2() {
	assert(timeout2_msg);
	if(timeout2_msg->isScheduled()) 
		cancelEvent(timeout2_msg);
}

void Pattern::activated() {}

void Pattern::deActivated() {}

void Pattern::init() {}

void Pattern::timeout() {}

void Pattern::timeout2() {}

vector<int> *Pattern::getNeighbours() {
	return getNeighbours((Node *) parentModule() // application
		->parentModule() // software
		->parentModule() // node container
		->submodule("node"));
}

vector<int> *Pattern::getNeighbours(int id) {
	return getNeighbours((Node *) parentModule() // application
		->parentModule() // software
		->parentModule() // node container
		->parentModule() // nodes
		->submodule("nodes", id)
		->submodule("node"));
}

vector<int> *Pattern::getNeighbours(Node *node) {
	double range = node->getMaxRadioRange();
	Position myPos = node->getPosition();
	
	cModule *network = node->parentModule() // node container
		->parentModule();
	cModule *firstNode = network->submodule("nodes", 0);
	
	vector<int> *neighbours = new vector<int>;
	
	for (int i = 0; i < firstNode->size(); i++) {
		double distance;

		Node *toNode = (Node*) network->submodule("nodes", i)->submodule("node");
		Position pos = toNode->getPosition();
		distance = absDistance(myPos, pos);
		if (distance < range && toNode->getNodeId() != node->getNodeId())
			neighbours->push_back(toNode->getNodeId());
	}
	return neighbours;	
}
