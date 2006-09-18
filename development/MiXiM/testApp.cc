#include "testApp.h"

Define_Module_Like(TestApp, Application);

void TestApp::initialize() {
	Application::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing test application...");

	initTimers(1);
	setTimer(0,TESTAPP_TIMER_INTERVAL);
}

void TestApp::finish() {
	Application::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding test application...");
}

TestApp::~TestApp() {} 

void TestApp::handleTimer(unsigned int idx) {
	Packet* ping;
	// timer
	printf(PRINT_APP, "timer rang");
	// do a broadcast
	ping = new Packet("ping message");
	ping->to = BROADCAST;
	send(ping, findGate("RadioOutMsg"));
	eatCycles(20);
}

void TestApp::handleEvent(Packet* msg) {
	switch(msg->kind()) {
		case TX_DONE:
			printf(PRINT_APP, "msg sent");
			setTimer(0,TESTAPP_TIMER_INTERVAL);
			break;
		case TX_FAILED:
			printf(PRINT_APP, "msg send failed");
			setTimer(0,TESTAPP_TIMER_INTERVAL);
			break;
		case RX:
			printf(PRINT_APP, "Got ping message");
			break;
		default:
			printf(PRINT_APP, "got unknown msg (%s)",msg->name());
	}
	delete msg;
}

