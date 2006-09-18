#include "multihopApp.h"

Define_Module_Like(MultihopApp, Application);

void MultihopApp::initialize() {
	Application::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing application...");

	source = getLongParameter("source", 1);
	destination = getLongParameter("destination", 0);
	
	if (node->getNodeId() == source) {
		initTimers(1);
		setTimer(0,10.0);
	}
	stat_tx_done = stat_tx_fail = stat_rx = 0;
}

void MultihopApp::finish() {
	Application::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding application...");
	printf(PRINT_STATS, "stats: tx_done=%u tx_fail=%u rx=%u ", stat_tx_done, stat_tx_fail, stat_rx);

}

void MultihopApp::handleTimer(unsigned int idx)
{
	setTimer(0,10.0);
	Packet *packet = new Packet("send");
	packet->setLength(25);
	packet->to = destination;
	send(packet, findGate("RadioOutMsg"));
}

void MultihopApp::handleEvent(Packet* msg) {
	switch(msg->kind()) {
		case TX_DONE:
			printf(PRINT_APP, "msg sent");
			delete msg;
			stat_tx_done++;
			break;
		case RX: 
			printf(PRINT_APP, "received message from %d, via %d", ((Packet *) msg)->from, ((Packet *) msg)->local_from);
			delete msg;
			stat_rx++;
			break;
		case TX_FAILED:
			printf(PRINT_APP, "msg send failed");
			delete msg;
			stat_tx_fail++;
			break;
		default:
			printf(PRINT_APP, "got unknown msg");
			delete msg;
	}
}
