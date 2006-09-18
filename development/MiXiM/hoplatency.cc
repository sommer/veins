#include "message.h"
#include "appselector.h"
#include "hoplatency.h"

Define_Module( HopLatency );

void HopLatency::init() {
	source = atoi(par("source"));
}

void HopLatency::generate() {
	printf(PRINT_APP, "generate");

	Packet * pkt = new Packet("hoplatency", TX);
	pkt->setLength(msglength);
	pkt->to = 0;
	double now = simTime();
	pkt->setData(PATTERN_DATA, &now, sizeof(now), 0);
	assert(pkt->to != BROADCAST);
	tx(pkt);
	++stat_tx;
}

void HopLatency::rx(Packet * msg) {
	assert(msg);
	printf(PRINT_APP, "receive");
	++stat_rx;
	
	if (node_id == 0) {
		double *sendTime = (double *) msg->getData(PATTERN_DATA);
		stat_delay_count++;
		stat_delay_total += simTime() - *sendTime;
		delete msg;
	} else {
		assert(0);
		delete msg;
	}
}

void HopLatency::activated() {
/*	if (source == node_id)*/
	if (source == parentModule()->parentModule()->parentModule()->index()) {
		printf(PRINT_STATS, "Sending messages");
		setTimeout(exponential(msginterval, RNG_APP));
	}
}

void HopLatency::deActivated() {}

void HopLatency::timeout() {
	if(turned_on) {
		generate();
		setTimeout(exponential(msginterval, RNG_APP));
	}
}

