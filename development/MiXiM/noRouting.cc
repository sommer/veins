#include "noRouting.h"
#include "gmac.h"

Define_Module_Like(NoRouting, Routing);

//FIXME: make this configurable
int NoRouting::max_retries = 3;
bool NoRouting::parametersInitialised = false, NoRouting::gmacLowLatency;

void NoRouting::initialize() {
	Routing::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing dummy routing layer...");
	if (!parametersInitialised) {
		parametersInitialised = true;
		string result = ev.getParameter(simulation.runNumber(), "net.macType");
		gmacLowLatency = result.substr(1, 4) == "GMac" ? getBoolParameter("gmacLowLatency", false) : false;
		max_retries = getLongParameter("maxRetries", 3);
	}
}

void NoRouting::finish() {
	Routing::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding dummy routing layer...");
}

Packet *NoRouting::selectPacket(Packet *packet) {
	GMac *gmac;
	int first;
	
	if (msgQueue->length() == 0)
		return packet;
	
	cQueue::Iterator iterator(*msgQueue, false);
	vector<int> targets;
	targets.push_back(packet->to);

	gmac = dynamic_cast<GMac *> (parentModule()->submodule("mac"));
	assert(gmac);
	
	printf(PRINT_ROUTING, "packet to: %d", packet->to);
	
	for (; !iterator.end(); iterator--) {
		Packet *current = (Packet *) iterator();
		targets.push_back(current->to);
		printf(PRINT_ROUTING, "queued packet to: %d", current->to);
	}
	
	first = gmac->firstToWake(&targets);
	if (packet->to == first)
		return packet;

	msgQueue->insertAfter(msgQueue->tail(), packet);

	iterator.init(*msgQueue, false);
	for (; !iterator.end(); iterator--) {
		Packet *current = (Packet *) iterator();
		if (current->to == first) {
			msgQueue->remove(current);
			return current;
		}
	}
	assert(0);
}

void NoRouting::rx(Packet *packet) {
	sendToApp(packet);
}

void NoRouting::tx(Packet *packet) {
	int retries = 0;
	packet->local_to = packet->to;
	packet->setData(ROUTING_DATA, &retries, sizeof(retries), 0);
	if (macBusy)
		addToQueue(packet);
	else
		sendToMac(packet);
}

void NoRouting::txDone(Packet *packet) {
	if (!msgQueue->empty()) {
		Packet *nextPacket = gmacLowLatency ? selectPacket((Packet *) msgQueue->pop()) : (Packet *) msgQueue->pop();
		sendToMac(nextPacket);
	}

	sendToApp(packet);
}

void NoRouting::txNoAck(Packet *packet) {
	int *retries = (int *) packet->getData(ROUTING_DATA);

	(*retries)++;
	if (*retries < max_retries) {
		packet->setKind(TX);
		packet = selectPacket(packet);
		sendToMac(packet);
		return;
	}
	packet->setKind(TX_FAILED);
	txDone(packet);
}

void NoRouting::txSkipped(Packet *packet) {
	packet->setKind(TX);
	packet = selectPacket(packet);
	sendToMac(packet);
}
