#include "magicSinkRouting.h"
#include "gmac.h"

#define MAGIC_MESSAGE 7001

Define_Module_Like(MagicSinkRouting, Routing);

bool MagicSinkRouting::parametersInitialised = false;

void MagicSinkRouting::initialize() {
	SinkRoutingBase::initialize();
	printfNoInfo(PRINT_INIT, "\t\t\tInitializing magic sink routing layer...");

	addGate("magic", 'I');

	if (node->getNodeId() == destination) {
		Packet *newPacket = new Packet("Magic_route_setup", MAGIC_MESSAGE);
		Header header = { ROUTE_SETUP, 0 };
		newPacket->setLocalFrom(node->getNodeId());
		newPacket->setData(ROUTING_DATA, &header, sizeof(header), 0);
		sendDirect(newPacket, simTime(), this, "magic");
	}
}

void MagicSinkRouting::finish() {
	SinkRoutingBase::finish();
	printfNoInfo(PRINT_INIT, "\t\t\tEnding magic sink routing layer...");
}

MagicSinkRouting::~MagicSinkRouting() {
	parametersInitialised = false;
}

void MagicSinkRouting::handleMessage(cMessage *msg) {
	Packet *packet = (Packet *) msg;
	
	switch(msg->kind()) {
		case MAGIC_MESSAGE: {
			Header *header = (Header *) packet->getData(ROUTING_DATA);
			
			if (parentHops < header->hopsToSink) {
				delete msg;
				return;
			} else if (parentHops == header->hopsToSink) {
				parents.push_back(packet->local_from);
				if (fixedParent)
					parent = intuniform(0, parents.size() - 1, RNG_ROUTING);
				delete msg;
				return;
			}

			parentHops = header->hopsToSink;
			parents.clear();
			parents.push_back(packet->local_from);
			parent = 0;

			int i;
			double range = node->getMaxRadioRange();
			Position myPos = node->getPosition();
			cModule* network = parentModule()->parentModule()->parentModule();
			cModule* nodeContainer = network->submodule("nodes", 0);

			for (i = 0; i < nodeContainer->size(); i++) {
				double distance;

				cModule* nodeContainer = network->submodule("nodes", i);
				Node *toNode = (Node*)(nodeContainer->submodule("node"));
				Position pos = toNode->getPosition();
				distance = absDistance(myPos, pos);
				//~ printf(PRINT_ROUTING, "distance of toNode %d to %d: %f", toNode->getNodeId(), node->getNodeId(), distance);
				if (distance < range) {
					Packet *newPacket = new Packet("Magic_route_setup", MAGIC_MESSAGE);
					Header header = { ROUTE_SETUP, parentHops + 1};
					newPacket->setLocalFrom(node->getNodeId());
					newPacket->setData(ROUTING_DATA, &header, sizeof(header), 0);

					//~ printf(PRINT_ROUTING, "sending to %d (index %d)", toNode->getNodeId(), i);
					sendDirect(newPacket, simTime(), nodeContainer->submodule("software")->submodule("routing"), "magic");
				}
			}
			delete msg;
			break;
		}
		default:
			SinkRoutingBase::handleMessage(msg);
			break;
	}
}

