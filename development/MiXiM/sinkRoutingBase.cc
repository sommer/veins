#include "sinkRoutingBase.h"
#include "gmac.h"

bool SinkRoutingBase::parametersInitialised = false;
bool SinkRoutingBase::fixedParent, SinkRoutingBase::gmacLowLatency;
int SinkRoutingBase::destination, SinkRoutingBase::max_retries;

void SinkRoutingBase::initialize() {
	Routing::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing sink routing base layer...");
	
	if (!parametersInitialised) {
		parametersInitialised = true;
		destination = getLongParameter("destination", 0);
		fixedParent = getBoolParameter("fixedParent", true);
		if (!fixedParent) {
			string result = ev.getParameter(simulation.runNumber(), "net.macType");
			gmacLowLatency = result.substr(1, 4) == "GMac" ? getBoolParameter("gmacLowLatency", true) : false;
		} else {
			gmacLowLatency = false;
		}
		max_retries = getLongParameter("maxRetries", 3);
	}

	parent = 0;
	parentHops = INT_MAX;
	retries = 0;
}

void SinkRoutingBase::finish() {
	Routing::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding sink routing base layer...");
}

SinkRoutingBase::~SinkRoutingBase() {
	parametersInitialised = false;
	parents.clear();
}

void SinkRoutingBase::setLocalTo(Packet *packet) {
	if (packet->to != destination) {
		packet->local_to = packet->to;
	} else if (fixedParent) {
		printf(PRINT_CRIT, "parent = %d", parent);
		packet->local_to = parents[parent];
	} else if (gmacLowLatency) {
		GMac *gmac = dynamic_cast<GMac *> (parentModule()->submodule("mac"));
		assert(gmac);
		packet->local_to = gmac->firstToWake(&parents);
		printf(PRINT_ROUTING, "First to wake: %d", packet->local_to);
	} else {
		packet->local_to = parents[intuniform(0, parents.size() - 1, RNG_ROUTING)];
	}
}

void SinkRoutingBase::runQueue(Packet *msg) {
	if (macBusy) {
		addToQueue(msg);
	} else {
		setLocalTo(msg);
		sendToMac(msg);
		macBusy = true;
	}
}


void SinkRoutingBase::rx(Packet *packet) {
	Header *header = (Header *) packet->getData(ROUTING_DATA);
	assert(packet->local_from != node->getNodeId());
	
	assert(header->type == ROUTE_DATA);
	
	if (packet->to == node->getNodeId() || packet->to == BROADCAST) {
		printf(PRINT_ROUTING, "Sending message to App");
		sendToApp(packet);
		return;
	}

	if (parents.empty()) {
		stat_tx_drop++;
		delete packet;
		return;
	}

	packet->setKind(TX);
	printf(PRINT_ROUTING, "Forwarding message from %d toward %d", packet->local_from, packet->to);
	runQueue(packet);
}

void SinkRoutingBase::tx(Packet *packet) {
	Header header;
	
	printf(PRINT_ROUTING, "Got message from App");

	header.type = ROUTE_DATA;

	/* Header contains: 1 byte type, 2*2 bytes to and from address.
	   However, for local bcast and ucast no addresses are necessary. */
	packet->setData(ROUTING_DATA, &header, sizeof(header), packet->to == destination ? 1 + 4 : 1);

	/* Broadcast and local unicast never fail because of lack of parents. */
	if (packet->to == destination && parents.empty()) {
		// sendToApp will update stats
		packet->setKind(TX_FAILED);
		sendToApp(packet);
		return;
	}

	runQueue(packet);
}

void SinkRoutingBase::txDone(Packet *packet) {
	Header *header;

	/* Reset the number of retries, because we will be sending a new message. */
	retries = 0;
	if (!msgQueue->empty()) {
		Packet *nextPacket = (Packet *) msgQueue->pop();
		setLocalTo(nextPacket);
		sendToMac(nextPacket);
	}

	header = (Header *) packet->getData(ROUTING_DATA);
	if (header->type == ROUTE_SETUP)
		delete packet;
	else if (packet->from == node->getNodeId())
		sendToApp(packet);
	else
		//FIXME: stats?
		delete packet;
}

void SinkRoutingBase::txNoAck(Packet *packet) {
	retries++;
	if (retries < max_retries) {
		setLocalTo(packet);
		packet->setKind(TX);
		sendToMac(packet);
		return;
	}
	packet->setKind(TX_FAILED);
	txDone(packet);
}
void SinkRoutingBase::txSkipped(Packet *packet) {
	setLocalTo(packet);
	packet->setKind(TX);
	sendToMac(packet);
}
