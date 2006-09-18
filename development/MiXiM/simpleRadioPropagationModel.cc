#include "simpleRadioPropagationModel.h"

Define_Module_Like(SimpleRadioPropagationModel, PropagationModelClass);

void SimpleRadioPropagationModel::initialize() {
	PropagationModel::initialize();
	printfNoInfo(PRINT_INIT, "Simple radio propagation model...");
	cModule* worldContainer = parentModule();
	network = worldContainer->parentModule();
	//~ cModule* nodesTest = network->submodule("nodes", 0);
}

void SimpleRadioPropagationModel::finish() {
	PropagationModel::finish();
	printfNoInfo(PRINT_INIT, "Ending simple radio propagation model...");
}

void SimpleRadioPropagationModel::handleMessage(cMessage *msg) {
	printf(PRINT_PROP, "simple radio prop.layer got msg %d", msg->kind());
	assert(msg);
	printf(PRINT_PROP, "RPM: %d: %s", msg->arrivalGate()->index(), msg->name());

	switch(msg->kind()) {
		case TX_START:
			printf(PRINT_PROP, "RPM: tx start");
			start_tx(msg->senderModule());
			break;
		case TX_STOP:
			stop_tx(msg->senderModule());
			printf(PRINT_PROP, "RPM: tx stop");
			break;
		case TX:
			printf(PRINT_PROP, "RPM: tx");
			assert_type(msg, Packet*);
			msg->setKind(NEIGHBOUR_TX);
			distributePacket((Packet*) msg, msg->senderModule());
			break;
		//~ case PREAMBLE:
			//~ preamble(msg->arrivalGate()->index());
			//~ break;
		default:
			printf(PRINT_PROP, "RPM: unknown!!");
			assert(false); // unknown
	}
	delete msg;

}

void SimpleRadioPropagationModel::start_tx(cModule *senderModule) {
	Packet start_pack("neighbour_start", NEIGHBOUR_START);
	distributePacket(&start_pack, senderModule);
}

void SimpleRadioPropagationModel::stop_tx(cModule *senderModule) {
	Packet stop_pack("neighbour_stop", NEIGHBOUR_STOP);
	distributePacket(&stop_pack, senderModule);
}

void SimpleRadioPropagationModel::distributePacket(Packet *msg, cModule *senderModule) {
	// for now:
	// send to all; radius? Wasda?
	unsigned i = 0;
	unsigned size;
	int index;
	Position senderPos = {0, 0, 0};
	double range;
	Packet *dup;
	Node *node;
	Node *sender;

	printf(PRINT_PROP, "PropModel: Got msg to distribute");
	
	cModule* nodeContainer = network->submodule("nodes", 0);

	if (senderModule != NULL) {
		cModule* radio = senderModule;
		cModule* container = radio->parentModule();
		index = container->index();
//		ev.printf("Index of sender: %d\n", index);
		sender = (Node*)(container->submodule("node"));
		msg->setLocalFrom(sender->getNodeId());
		// Don't care about power, as it is all the same
		msg->power = 1.0;
//		ev.printf("nodeid of sender: %d\n", sender->getNodeId());
		senderPos = sender->getPosition();
		range = sender->getMaxRadioRange();

		size = nodeContainer->size();
//		ev.printf("Size of container: %d\n", size);

		while (i < size) {
			double distance;
			node = (Node*)(nodeContainer->submodule("node"));
			Position pos = node->getPosition();
			distance = absDistance(senderPos, pos);
			printf(PRINT_PROP, "distance of node %d to %d: %f", node->getNodeId(), sender->getNodeId(), distance);
			if (distance < range && i != (unsigned)index) {
				printf(PRINT_PROP, "sending to %d (index %d)", node->getNodeId(), i);
				dup = (Packet*)msg->dup();
				send(dup, "outPort", i);
			}
			nodeContainer = network->submodule("nodes", ++i);
		}
	} else {
//		ev.printf("Sendermodule is null\n");
		for (i = 0; i < (unsigned) gateSize("outPort"); i++) {
	//		if (i != index) {
				dup = (Packet*)msg->dup();
				send(dup, "outPort", i);
	//		}
		}
	}
}
