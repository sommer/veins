#include "sinkRouting.h"
#include "gmac.h"

Define_Module_Like(SinkRouting, Routing);

bool SinkRouting::parametersInitialised = false;
double SinkRouting::floodInterval, SinkRouting::initWait;

void SinkRouting::initialize() {
	SinkRoutingBase::initialize();
	printfNoInfo(PRINT_INIT, "\t\t\tInitializing sink routing layer...");
	macBusy = false;
	
	if (!parametersInitialised) {
		parametersInitialised = true;
		floodInterval = getTimeParameter("floodInterval", 10);
		initWait = getTimeParameter("initWait", 0.01);
	}
	
	if (node->getNodeId() == destination) {
		cMessage *timerMessage = new cMessage("SinkRouting_timer");
		scheduleAt(simTime() + initWait, timerMessage);
	}
}

void SinkRouting::finish() {
	SinkRoutingBase::finish();
	printfNoInfo(PRINT_INIT, "\t\t\tEnding sink routing layer...");
}

SinkRouting::~SinkRouting() {
	parametersInitialised = false;
}	

void SinkRouting::handleMessage(cMessage *msg) {
	Packet *packet;
	
	/* Handle timer */
	if (msg->isSelfMessage()) {
		Header header;
		packet = new Packet("SinkRouting_Flood");
		
		packet->setTo(BROADCAST);
		header.type = ROUTE_SETUP;
		header.hopsToSink = 0;
		header.serial = ++serial;
		packet->setData(ROUTING_DATA, &header, sizeof(header), 2);
		printf(PRINT_ROUTING, "Starting flood");
		runQueue(packet);
		scheduleAt(simTime() + floodInterval, msg);
		return;
	}
	
	packet = (Packet *) msg;

	switch(msg->kind()) {
		case RX: {
			Header *header = (Header *) packet->getData(ROUTING_DATA);
			if (packet->local_from == node->getNodeId())
				printf(PRINT_STATS, "Error packet: f%d t%d lf%d lt%d [t%d] (%d)", packet->from, packet->to, packet->local_from, packet->local_to, header->type, node->getNodeId());
			assert(packet->local_from != node->getNodeId());
			
			if (header->type == ROUTE_SETUP) {
				if (serial > header->serial) {
					// Drop old message.
					delete msg;
					return;
				}				
				printf(PRINT_ROUTING, "Flood received");
				if (parentHops < header->hopsToSink) {
					delete msg;
					return;
				} else if (parentHops == header->hopsToSink) {
					parents.push_back(packet->local_from);
					if (fixedParent) {
						parent = intuniform(0, parents.size() - 1, RNG_ROUTING);
						printf(PRINT_ROUTING, "chose parent %d of %d", parent, parents.size());
					}
					delete msg;
					return;
				}

				parentHops = header->hopsToSink;
				parents.clear();
				printf(PRINT_ROUTING, "Parent %d caused flush", packet->local_from);
				parents.push_back(packet->local_from);
				parent = 0;

				serial = header->serial;
				parent = packet->local_from;
				parentHops = header->hopsToSink;
				header->hopsToSink++;
				packet->setKind(TX);
				runQueue(packet);
				return;
			}
			
			assert(header->type == ROUTE_DATA);
			/* FALLTHROUGH */
		}
		default:
			SinkRoutingBase::handleMessage(msg);
			break;
	}
}

