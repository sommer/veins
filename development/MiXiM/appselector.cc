#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mixim.h"
#include "message.h"

#include "appselector.h"

Define_Module_Like( AppSelector, Application )

bool AppSelector::parametersInitialised = false;

void AppSelector::initialize() {
	Application::initialize();

	active_refcount = 0;
	stat_tx = stat_rx = 0;
	
	addGate("from_area", 'I');
	
	if (!parametersInitialised) {
		cModuleType *moduleType = findModuleType("ScenarioManager");
		cModule *module = moduleType->create("scenario", parentModule()->parentModule()->parentModule());
		module->buildInside();
		module->scheduleStart(simTime());
		
		moduleType = findModuleType("AreaManager");
		module = moduleType->create("areaManager", parentModule()->parentModule()->parentModule());
		module->buildInside();
		module->scheduleStart(simTime());
		
		parametersInitialised = true;
	}
}

void AppSelector::finish() {
	printf(PRINT_STATS, "stats: tx=%d rx=%d", stat_tx, stat_rx);
	Application::finish();
}

AppSelector::~AppSelector() {
	parametersInitialised = false;
}

void AppSelector::handleMessage(cMessage *msg) {
	assert(msg);
	switch(msg->kind()) {
		case TX:
			// get pattern index
			assert(msg->arrivalGate());
			assert(msg->arrivalGate()->index() >= 0);
			assert_type(msg, Packet *);
			
			tx((Packet *) msg);
			break;
		case RX:
			assert_type(msg, Packet *);
			rx((Packet *) msg);
			break;
		case TX_DONE:
			assert_type(msg, Packet *);
			txDone((Packet *) msg);
		case TX_FAILED:
			delete msg;
			break;
		case BECOME_ACTIVE:
			if(++active_refcount == 1)
				becomeActive();
			delete msg;
			break;
		case BECOME_IDLE:
			assert(active_refcount > 0);
			if(--active_refcount == 0)
				becomeIdle();
			delete msg;
			break;
		default:
			printf(PRINT_CRIT, "Got a %s\n",msg->name());
			assert(false); // unknown msg
	}	
}

void AppSelector::txDone(Packet *packet) {
	Header *header = (Header *) packet->getData(APPLICATION_DATA);
	
	printf(PRINT_ROUTING, "tx packet done");
	cMessage * msg = new cMessage("send latency", SEND_LATENCY);
	msg->addPar("delay") = simTime() - header->sendTime;
	send(msg, findGate("toPattern", header->pattern));
}


void AppSelector::tx(Packet * msg) {
	Header header;
	header.pattern = msg->arrivalGate()->index();
	header.sendTime = simTime();

	stat_tx++;

	// Size is port
	msg->setData(APPLICATION_DATA, &header, sizeof(header), 1);
	send(msg, findGate("RadioOutMsg"));
}

void AppSelector::rx(Packet * msg) {
	Header *header;
	
	header = (Header *) msg->getData(APPLICATION_DATA);

	assert(header->pattern >= 0);
	
	if(msg->to == BROADCAST) {
		printf(PRINT_APP, "received broadcast, pattern = %d", header->pattern);
		++stat_rx;
	} else {
		assert(msg->from >= 0);
		//Note: duplicate filtering is now done by routing layer
		printf(PRINT_APP, "received packet, pattern = %d", header->pattern);
		++stat_rx;
	}
	// report msg
	assert(header->pattern >= 0 && header->pattern < gate("toPattern")->size());
	send(msg, findGate("toPattern", header->pattern));
}

void AppSelector::becomeActive() {
	printf(PRINT_APP, "becoming active");
	int gates = gate("toPattern")->size();
	int i;
	for(i = 0; i < gates; i++)
		send(new cMessage("become active",BECOME_ACTIVE), findGate("toPattern", i));
}

void AppSelector::becomeIdle(){
	printf(PRINT_APP, "becoming idle");
	int gates = gate("toPattern")->size();
	int i;
	for(i = 0; i < gates; i++)
		send(new cMessage("become idle",BECOME_IDLE), findGate("toPattern", i));
}
