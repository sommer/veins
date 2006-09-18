#include "tosinkApp.h"

Define_Module_Like(TosinkApp, Application);

bool TosinkApp::parametersInitialised = false;
int TosinkApp::destination, TosinkApp::dataSize;
double TosinkApp::interval, TosinkApp::initialWait;

void TosinkApp::initialize() {
	Application::initialize();
	initTimers(1);
	printfNoInfo(PRINT_INIT, "\t\tInitializing application...");

	if (!parametersInitialised) {
		parametersInitialised = true;
		destination = getLongParameter("destination", 0);
		interval = getDoubleParameter("interval", 10.0);
		initialWait = getDoubleParameter("initialWait", 10.0);
		dataSize = getLongParameter("dataSize", 20);
	}

	if (node->getNodeId() != destination) {
		setTimer(0, initialWait + exponential(interval, RNG_APP));
	}
	stat_tx_done = stat_tx_fail = stat_rx = stat_initiated = 0;
	stat_sumlatency = 0.0;
}

void TosinkApp::finish() {
	Application::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding application...");
	printf(PRINT_STATS, "stats: initiated=%u tx_done=%u tx_fail=%u rx=%u avg_latency=%f", stat_initiated, stat_tx_done, stat_tx_fail, stat_rx, stat_sumlatency / stat_rx);
}

TosinkApp::~TosinkApp() {
	parametersInitialised = false;
}

void TosinkApp::handleTimer(unsigned int idx){
	Packet *packet = new Packet("send");
	double time = simTime();
	setTimer(0, exponential(interval, RNG_APP));
	packet->to = destination;
	packet->setData(APPLICATION_DATA, &time, sizeof(time), dataSize);
	stat_initiated++;
	send(packet, findGate("RadioOutMsg"));
}

void TosinkApp::handleEvent(cMessage* msg) {
	assert_type(msg, Packet *);
	Packet *packet = (Packet*) msg;

	switch(packet->kind()) {
		case TX_DONE:
			printf(PRINT_APP, "msg sent");
			delete packet;
			stat_tx_done++;
			break;
		case RX:  {
			double *time = (double *) packet->getData(APPLICATION_DATA);
			stat_sumlatency += simTime() - *time;
			stat_rx++;

			printf(PRINT_APP, "received message from %d, via %d", packet->from, packet->local_from);
			delete packet;
			break;
		}
		case TX_FAILED:
			printf(PRINT_APP, "msg send failed");
			delete packet;
			stat_tx_fail++;
			break;
		default:
			printf(PRINT_APP, "got unknown msg");
			assert(0);
			delete packet;
	}
}
