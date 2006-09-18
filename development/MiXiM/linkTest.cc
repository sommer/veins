#include "linkTest.h"

Define_Module_Like(LinkTest, Application);

void LinkTest::initialize()
{
	Application::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing link test application...");

	initTimers(1);
	
	node_id = node->getNodeId();
	node_count = strtol(ev.getParameter(simulation.runNumber(), "net.nodeCount").c_str(), NULL, 0);

	if (node_id == 0 || node_id == 1)
		timer_interval = getDoubleParameter("core-interval", 10.0);
	else	
		timer_interval = getDoubleParameter("cloud-interval", 0.5);
	setTimer(0,timer_interval);
	
	stats_tx = stats_rx = 0;
}

void LinkTest::finish() {
	Application::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding link test application...");
	printf(PRINT_STATS, "stats: tx=%u rx=%u", stats_tx, stats_rx);
}

LinkTest::~LinkTest() {} 

void LinkTest::handleTimer(unsigned int idx) {
	Packet* ping;
	// timer
	printf(PRINT_APP, "timer rang");
	// do a broadcast
	ping = new Packet("ping message");
	if(node_id == 0) {
		ping->to = 1;
		stats_tx++;
	} else if(node_id == 1) {
		ping->to = 0;
		stats_tx++;
	} else {
		/*while (1)
		{
			int node = intuniform(2, node_count, RNG_APP); // not 0 or 1
			ping->to = node;
			if (ping->to != node_id)
				break;
		}
		printf(PRINT_APP, "Sending (random choice) to %d\n", ping->to);
		*/
		ping->to = BROADCAST;
	}
	printf(PRINT_APP, "Sending to %d", ping->to);
	ping->local_to = ping->to;
	send(ping, findGate("RadioOutMsg"));
	eatCycles(20);
	setTimer(0,timer_interval);
}

void LinkTest::handleEvent(cMessage* msg) {
	assert_type(msg, Packet *);
	Packet *packet = (Packet*) msg;
	switch(packet->kind()) {
		case TX_DONE:
			printf(PRINT_APP, "msg sent");
			break;
		case TX_FAILED:
			printf(PRINT_APP, "msg send failed");
			break;
		case RX:
			if (packet->to != BROADCAST)
			    stats_rx++;
			printf(PRINT_APP, "Got ping message");
			break;
		default:
			printf(PRINT_APP, "got unknown msg (%s)",msg->name());
			assert(0);
	}
	delete packet;
}

