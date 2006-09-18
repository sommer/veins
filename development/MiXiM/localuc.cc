#include "message.h"
#include "appselector.h"
#include "localuc.h"

Define_Module( LocalUC );

void LocalUC::init() {
	reply_prob = atof(par("replyprob"));
	reply_delay = atof(par("replydelay"));
}

void LocalUC::generate() {
	printf(PRINT_APP, "generate");
	
	vector<int> *neighbours = getNeighbours();
	

	// choose a neighbour
	if(neighbours->size() == 0)  { // no neighbours
		delete neighbours;
		return;
	}

	int nbi = (int)intuniform(0, neighbours->size() - 1, RNG_APP);
	int nb = (*neighbours)[nbi];
	delete neighbours;

	printf(PRINT_APP, "Sending message to %d", nb);
	
	Packet * pkt = new Packet("LocalUC", TX);
	pkt->setLength(msglength);
	pkt->to = nb;
	double now = simTime();
	pkt->setData(PATTERN_DATA, &now, sizeof(now), 0);
	assert(pkt->to != node_id);
	tx(pkt);
	++stat_tx;
}

void LocalUC::rx(Packet * msg) {
	assert(msg);

	double *sendTime = (double *) msg->getData(PATTERN_DATA);
	
	printf(PRINT_APP, "receive");
	++stat_rx;
	stat_delay_count++;
	stat_delay_total += simTime() - *sendTime;

	if(uniform(0.0, 1.0, RNG_APP_AUX) < reply_prob) // chance 
	{
		printf(PRINT_APP, "sending reply to uc");
		msg->to = msg->local_to = msg->local_from;
		tx(msg);
		++stat_tx;
	} else {
		delete msg;
	}
}

void LocalUC::activated() {
	setTimeout(exponential(msginterval, RNG_APP));
}

void LocalUC::deActivated() {}

void LocalUC::timeout() {
	if(turned_on) {
		generate();
		setTimeout(exponential(msginterval, RNG_APP));
	}
}

