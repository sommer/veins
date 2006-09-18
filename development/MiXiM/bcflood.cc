#include <stdlib.h>

#include "message.h"
#include "bcflood.h"

Define_Module( BCFlood );

void BCFlood::init() {
	source = atoi(par("source"));
	assert(source >= 0);
	mySerial = 1;
	lastSerial = 0;
	bitmap = 0;
}

// generate is a command to generate the next msg
void BCFlood::generate() {
	if (node_id != source)
		return;
	
	Header header = { simTime(), mySerial++ };
	// Prevent receiving/rebroadcasting my own
	lastSerial = header.serial;
	Packet * pkt = new Packet(0, TX);
	pkt->setLength(msglength);
	pkt->to = BROADCAST;
	pkt->setData(PATTERN_DATA, &header, sizeof(header), 0);
	tx(pkt);
	++stat_tx;
}

void BCFlood::rx(Packet * msg) {
	assert(msg);
	assert(msg->to == BROADCAST);

	Header *header = (Header *) msg->getData(PATTERN_DATA);
	
	if (!registerSerial(header->serial)) {
		printf(PRINT_APP, "Received duplicate (%d)", lastSerial);
		delete msg;
		return;
	}
	
	printf(PRINT_APP, "Received flood (%d)", lastSerial);
	
	// Not previously seen flood
	++stat_rx;
	stat_delay_count++;
	stat_delay_total += simTime() - header->sendTime;
	
	tx(msg);
}

void BCFlood::activated() {
	setTimeout(exponential(msginterval, RNG_APP));
}

void BCFlood::deActivated() {}

void BCFlood::timeout() {
	if(turned_on && node_id == source) {
		generate();
		setTimeout(exponential(msginterval, RNG_APP));
	}
}

#define HISTORY_BITS 8
#define HISTORY_MASK ((1<<HISTORY_BITS) - 1)

bool BCFlood::registerSerial(int serial) {
	if (lastSerial < serial) {
		int shift = serial - lastSerial;
		if (shift > HISTORY_BITS + 1)
			bitmap = 0;
		else
			bitmap = (((1 << HISTORY_BITS) | bitmap) >> shift) & HISTORY_MASK;
		lastSerial = serial;
		return true;
	} else if (lastSerial == serial) {
		return false;
	} else if (lastSerial - serial <= HISTORY_BITS) {
		int shift = HISTORY_BITS - (lastSerial - serial);
		assert(shift >= 0 && shift <= HISTORY_BITS - 1);
		if (bitmap & (1 << shift))
			return false;
		else
			bitmap |= (1 << shift);
		return true;
	} else {
		return false;
	}
}
