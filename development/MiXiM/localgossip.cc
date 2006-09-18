#include "message.h"
#include "localgossip.h"

Define_Module( LocalGossip );

void LocalGossip::init() {
	reply_prob = atof(par("replyprob"));
	reply_delay = atof(par("replydelay"));

	int centernode = atoi(par("centernode"));
	int hops = atoi(par("hops"));
	gossiping = node_id == centernode || checkGossiping(centernode, hops);
}

bool LocalGossip::checkGossiping(int centernode, int hops) {
	bool retval = false;
	
	if (hops <= 0)
		return false;

	vector<int> *neighbours = getNeighbours(centernode);
	
	for (unsigned int i = 0; i < neighbours->size(); i++) {
		if ((*neighbours)[i] == node_id || checkGossiping((*neighbours)[i], hops-1)) {
			retval = true;
			break;
		}
	}

	delete neighbours;
	return retval;
}	

void LocalGossip::generate() {
	printf(PRINT_APP, "generate");

	// choose a neighbour
	vector<int> *neighbours = getNeighbours();

	if(neighbours->size() == 0) { // no neighbours
		delete neighbours;
		return;
	}

	int nbi = (int)intuniform(0, neighbours->size() - 1, RNG_APP);

	Packet * pkt = new Packet(0, TX);
	pkt->setLength(msglength);
	pkt->to = (*neighbours)[nbi];;
	
	double now = simTime();
	pkt->setData(PATTERN_DATA, &now, sizeof(now), 0);
	assert(pkt->to != node_id);
	tx(pkt);
	++stat_tx;
	
	delete neighbours;
}

void LocalGossip::rx(Packet * msg) {
	assert(msg);
	double *sendTime = (double *) msg->getData(PATTERN_DATA);

	printf(PRINT_APP, "receive");
	++stat_rx;
	stat_delay_count++;
	stat_delay_total += simTime() - *sendTime;

	if(uniform(0.0, 1.0, RNG_APP_AUX) < reply_prob) // chance 
	{
		printf(PRINT_APP, "sending reply to uc");
		msg->to = msg->local_from;
		tx(msg);
		++stat_tx;
	} else {
		delete msg;
	}
}

void LocalGossip::activated() {
	if (gossiping)
		setTimeout(exponential(msginterval, RNG_APP));
}

void LocalGossip::deActivated() {}

void LocalGossip::timeout() {
	if(turned_on) {
		generate();
		setTimeout(exponential(msginterval, RNG_APP));
	}
}

