#include "pathLossRadioPropagationModel.h"
#include "radio.h"

Define_Module_Like(PathLossRadioPropagationModel, PropagationModelClass);

bool PathLossRadioPropagationModel::parametersInitialised = false;
double PathLossRadioPropagationModel::beta;

double PathLossRadioPropagationModel::d0power;

void PathLossRadioPropagationModel::initialize(int stage) {
	PropagationModel::initialize();

	if (stage == 0) {
		printfNoInfo(PRINT_INIT, "Path loss radio propagation model...");
		cModule* worldContainer = parentModule();
		network = worldContainer->parentModule();
		//~ cModule* nodesTest = network->submodule("nodes", 0);
	} else if (stage == 1) {
		if (!parametersInitialised) {
			Node *node = (Node *) network->submodule("nodes", 0)->submodule("node");
			double range = node->getMaxRadioRange();
			Radio *radio = (Radio *) network->submodule("nodes", 0)->submodule("radio");
			double rx_threshold = radio->getRXThreshold();
			//FIXME: use per message TX power!
			d0power = (radio->getMaxTXPower() * pow(299792458.0 / 868000000.0 /* wave length */, 2.0)) / pow(4.0 * M_PI, 2.0);

			beta = log(d0power / rx_threshold) / log(range);
			
			//~ ::fprintf(stderr, "Beta: %e\n", beta);
			parametersInitialised = true;
		}
	}
}

void PathLossRadioPropagationModel::finish() {
	printfNoInfo(PRINT_INIT, "Ending path loss radio propagation model...");
	PropagationModel::finish();
}

PathLossRadioPropagationModel::~PathLossRadioPropagationModel() {
	parametersInitialised = false;
}

void PathLossRadioPropagationModel::handleMessage(cMessage *msg) {
	printf(PRINT_PROP, "path loss radio prop.layer got msg %d", msg->kind());
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

void PathLossRadioPropagationModel::start_tx(cModule *senderModule) {
	Packet start_pack("neighbour_start", NEIGHBOUR_START);
	distributePacket(&start_pack, senderModule);
}

void PathLossRadioPropagationModel::stop_tx(cModule *senderModule) {
	Packet stop_pack("neighbour_stop", NEIGHBOUR_STOP);
	distributePacket(&stop_pack, senderModule);
}

void PathLossRadioPropagationModel::distributePacket(Packet *msg, cModule *senderModule) {
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
		sender = (Node*)(container->submodule("node"));
		msg->setLocalFrom(sender->getNodeId());
		senderPos = sender->getPosition();
		range = sender->getMaxRadioRange();
		
		size = nodeContainer->size();

		while (i < size) {
			double distance;
			node = (Node*)(nodeContainer->submodule("node"));
			Position pos = node->getPosition();
			distance = absDistance(senderPos, pos);
			//~ printf(PRINT_PROP, "distance of node %d to %d: %f", node->getNodeId(), sender->getNodeId(), distance);
			//~ if (distance < range && i != (unsigned)index) {
			if (i != (unsigned)index) {
				dup = (Packet*)msg->dup();
				dup->power = d0power / pow(distance, beta);
				if (dup->power > 2 * d0power)
					dup->power = d0power;
				printf(PRINT_PROP, "sending to %d (index %d) distance %f, power %e", node->getNodeId(), i, distance, dup->power);
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
