#include <stdlib.h>

#include "message.h"
#include "areamanager.h"

#include "tosink.h"

Define_Module( ToSink );

#define AGG_MODE_NONE	0
#define AGG_MODE_CONCAT	1
#define AGG_MODE_AVG	2

void ToSink::init() {
	sinknode = atoi(par("sinknode"));
	assert(sinknode>=0);
	ttl = atoi(par("ttl"));
	assert(ttl > 0);

	agg_mode = atoi(par("aggmode"));
	if(agg_mode != AGG_MODE_NONE) {
		agg_timeout = atof(par("aggdelay"));
		assert(agg_timeout > 0.0);
		agg_max = atoi(par("aggmax"));
		assert(agg_max > 0);
	}
}

// generate is a command to generate the next msg
void ToSink::generate() {
	// skip if i'm sink
	if(node_id == sinknode)
		return;

	Packet * pkt = new Packet(0, TX);
	pkt->setLength(msglength);
	pkt->to = sinknode; 	// sink
	Header header = { simTime(), ttl, 1 };
	pkt->setData(PATTERN_DATA, &header, sizeof(header), 0);
	printf(PRINT_APP, "generating new message");
	++stat_tx;
	aggregate(pkt);
}

void ToSink::rx(Packet * msg) {
	assert(msg);
	assert(msg->to == sinknode);
	Header *header = (Header *) msg->getData(PATTERN_DATA);
	assert(header->ttl > 0); // hops left before this

	if(node_id == sinknode) { // congrats
		cModule *network = parentModule()->parentModule()->parentModule()->parentModule();
		ToSink *origin = NULL;
		for (int i = 0; i < network->submodule("nodes", 0)->size(); i++) {
			Node *node = (Node *) network->submodule("nodes", i)->submodule("node");
			if (node->getNodeId() == msg->from) {
				origin = (ToSink *) network->submodule("nodes", i)->submodule("software")->submodule("application")->submodule(name());
				break;
			}
		}
		origin->stat_rx += header->msgCount; // count all aggregated msgs
		origin->stat_delay_count++;
		origin->stat_delay_total += simTime() - header->sendTime;
		printf(PRINT_APP, "msg received at sink");
		assert(header->msgCount > 0);
		stat_rx += header->msgCount; // count all aggregated msgs
		stat_delay_count++;
		stat_delay_total += simTime() - header->sendTime;
		delete msg;
	} else {
		if(--header->ttl == 0 ) {
			printf(PRINT_APP, "message expired");
			delete msg;
			return;
		}
		printf(PRINT_APP, "received message for %d", msg->to);
		aggregate(msg);
	}

}

void ToSink::activated() {
	if (!is_active || node_id == centerNode)
		setTimeout(exponential(msginterval, RNG_APP));
}

void ToSink::deActivated() {}

void ToSink::timeout() {
	if(turned_on) {
		generate();
		setTimeout(exponential(msginterval, RNG_APP));
	}
}
/*
void ToSink::route(Packet * msg) {
	assert(msg);
	int to = msg->to;
	int hops = model -> hopsTo(node_id, to);
	assert(hops>0);
	if(hops == 1) { // neighbour
		msg->local_to = msg->to;
	} else {
		// go through all my neighbours and 
		// pick one that is closer and has fewer or equal hops to dest
		int nbc = model->neighbourCount(node_id);
		const int * nb = model->neighbourArray(node_id);
		int candidate[nbc];
		int cand_count = 0;
		for(; nbc>0; nb++, nbc--) {
			if(model->hopsTo(*nb, to) < hops)
				candidate[cand_count++] = *nb;
		}
		// all candidates selected now
		if(cand_count == 0) { // oops dead end
			printf(PRINT_APP, "no route to node %d", to);
			delete msg;
			return;
		}
		// use generator 2 here because it's not certain a message arrives at all
		msg->local_to = candidate[(int)intuniform(0, cand_count-1, RNG_APP_AUX)];
	}
	printf(PRINT_APP, "routing msg to %d via %d", to, msg->local_to);
	tx(msg);
}
*/	
void ToSink::aggregate(Packet * msg) {
	if(agg_mode == AGG_MODE_NONE) {
		//~ route(msg); // send now
		tx(msg); // send now
		return;
	}

	int nItems = localQ.items();

	assert(localQ.items() < agg_max);

	// if this msg has a different dest than the others, flush
	if(nItems > 0) {
		Packet * firstmsg = (Packet *)localQ[0];
		assert_type(firstmsg, Packet *);
		if(firstmsg->to != msg->to) {
			printf(PRINT_APP, "different destination, flushing");
			sendAggregated();
			nItems = 0;
		}
	}

	// add to queue
	localQ.add(msg);
	printf(PRINT_APP, "%d items in local q now", localQ.items());

	// if maximum aggregated, send now
	if(++nItems >= agg_max) {
		printf(PRINT_APP, "flushing");
		sendAggregated();
	} else if(nItems == 1) { 
		// first message, set timer
		setTimeout2(agg_timeout);
	}
}

void ToSink::sendAggregated() {
	assert(localQ.items() > 0);
	Packet * msg = (Packet *)localQ.remove(0);
	assert(msg);
	assert_type(msg, Packet *);
	Header *header = (Header *) msg->getData(PATTERN_DATA);
	int total_length = msg->length();
	int total_msgs = header->msgCount;
	int i;
	for(i=localQ.items()-1; i>0; i--) {
		Packet * m = (Packet *)localQ[i];
		assert(m);
		Header *h = (Header *) m->getData(PATTERN_DATA);
		total_msgs += h->msgCount;
		switch(agg_mode) {
			case AGG_MODE_AVG:
				break; // total length = length[0]
			case AGG_MODE_CONCAT:
				total_length += m ->length();
				break;
			default:
				assert(false); // wrong / unknown agg mode
		}
	}
	// aggregate into first message
	msg->setLength(total_length);
	header->msgCount = total_msgs;	// for stats
	//~ route(msg); // send it
	tx(msg); // send it

	// and kill the rest
	for(i = localQ.items()-1; i>1; i--) {
		cObject * o = localQ.remove(i);
		delete o;
	}
	localQ.clear();
	cancelTimeout2(); 
}

void ToSink::timeout2() {
	printf(PRINT_APP, "aggregation timer expired, flushing");
	sendAggregated();
}
