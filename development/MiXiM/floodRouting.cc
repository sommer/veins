#include "floodRouting.h"

Define_Module_Like(FloodRouting, Routing);

void FloodRouting::initialize() {
	Routing::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing flood routing layer...");
}

void FloodRouting::finish() {
	Routing::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding flood routing layer...");
}

void FloodRouting::rx(Packet *packet) {
	assert(packet->from != node->getNodeId());
	
	if (packet->to == node->getNodeId() || packet->to == BROADCAST) {
		printf(PRINT_ROUTING, "Sending message to App");
		sendToApp(packet);
		return;
	}

	printf(PRINT_ROUTING, "Rebroadcasting packet with serial %d from node %d", packet->serial, packet->from);
	packet->setKind(TX);
	if (macBusy)
		addToQueue(packet);
	else
		sendToMac(packet);
}

void FloodRouting::tx(Packet *packet) {
	printf(PRINT_ROUTING, "Got message from App");
	packet->local_to = BROADCAST;
	if (macBusy)
		addToQueue(packet);
	else
		sendToMac(packet);
}

void FloodRouting::txDone(Packet *packet) {
	if (!msgQueue->empty())
		sendToMac((Packet *) msgQueue->pop());
	
	if (packet->from == node->getNodeId())
		sendToApp(packet);
	else
		//FIXME: stats?
		delete packet;
}
