#include <stdlib.h>

#include "message.h"
#include "localbc.h"

#define BC_PERCENTAGE

Define_Module( LocalBC );

void LocalBC::init() {
	reply_prob = atof(par("replyprob"));
	reply_delay = atof(par("replydelay"));
}

// generate is a command to generate the next msg
void LocalBC::generate() {
	Packet * pkt = new Packet(0, TX);
	pkt->setLength(msglength);
	pkt->to = BROADCAST;
	tx(pkt);
//FIXME: make this an option
#ifdef BC_PERCENTAGE
	/* FIXME: this may not be entirely correct in the case of mobility, because there will
	   be a delay between generation and transmission. But for now we don't care.
	   Eventually we should probably abuse rxDelay for this, as it is called right after
	   the send operation is done. Then we can also make it such that we keep two different
	   counts, one simply the number of messages, one the number of expected receives. */
	vector<int> *neighbours = getNeighbours();
	stat_tx += neighbours->size();
	delete neighbours;
#else
	++stat_tx;
#endif
}

void LocalBC::rx(Packet * msg) {
	assert(msg);
#ifdef BC_PERCENTAGE
	++stat_rx;
#else
	//Note: use a seqno for which packet the received counter is for
	//since all packets are received "simultaniously" we will only get a
	//different seqno after some time. So if the seqno's are different, reset
	//the received counter
	//increase the received counter for this packet on the originating node
	//check if it sums to the neighbour count of the originating node
	//if so increase it's stat_rx
#endif

	if(msg->to == BROADCAST	// was broadcast
			&& uniform(0.0, 1.0, RNG_APP_AUX) < reply_prob) // chance 
	{
		printf(PRINT_APP, "sending reply to bc");
		msg->to = msg->local_to = msg->local_from;
		tx(msg);
		++stat_tx;
		
	} else {
		delete msg;
	}
}

void LocalBC::activated() {
	setTimeout(exponential(msginterval, RNG_APP));
}

void LocalBC::deActivated() {}

void LocalBC::timeout() {
	if(turned_on) {
		generate();
		setTimeout(exponential(msginterval, RNG_APP));
	}
}

void LocalBC::rxDelay(cMessage * msg) {
	stat_delay_count++;
	stat_delay_total += (double) msg->par("delay");
}
