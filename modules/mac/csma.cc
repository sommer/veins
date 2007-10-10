/*
 * CSMA_LPL
 *
 * A very simple mac protocol. If we want to send a packet,
 * we sense if the medium is free (using rssi). If it is free,
 * send the packet. If the medium is not free, we delay for some
 * time and try again. We also delay if we just sent a msg.
 *
 * Receiving is done by using polling to detect the preamble.
 * When a preamble is detected the radio stays on until a packet
 * is received.
 */


#include "message.h"

#include "csma.h"

Define_Module_Like( Csma, EyesMacLayer );

#define IDLE		1
#define TRANSMIT	2
#define DELAYING	6
#define SENSING		7

// basic delay time: 3 ms
#define BASIC_DELAY	(3*33)

// random delay time: 0-1 ms
#define RANDOM_DELAY	(33)
	
void Csma::initialize() {
	printfNoInfo(PRINT_INIT, "\t\tCSMA initializing...");
	tx_msg = NULL;
	state = IDLE;
	
	setRadioListen();
}

void Csma::finish() {
	EyesMacLayer::finish();
	printfNoInfo(PRINT_INIT, "\t\tCSMA ending...");
}

Csma::~Csma()
{
	if (tx_msg)
		delete tx_msg;
}

void Csma::txPacket(MacPacket * msg){
	assert(msg);
	if(tx_msg) {
		printf(PRINT_ROUTING, "MAC busy! dropping at tx_packet");
		++stat_tx_drop;
		delete msg;
		return;
	}
	tx_msg = msg;
	if(state == IDLE && !isReceiving())
		gotoSensing();
}

void Csma::gotoIdle() {
	state = IDLE;
	if(tx_msg && !isReceiving())
		gotoSensing();
	else {
		printf(PRINT_MAC, "idle");
		setRadioListen();
	}
}

void Csma::gotoSensing() {
	printf(PRINT_MAC, "start sensing channel");
	state = SENSING;
	// turn on the receiver, just long enough to 
	// get a reliable rssi value
	setRadioListen();
	setTimeout(1); // 30 usecs
}

void Csma::gotoDelaying() {
	printf(PRINT_MAC, "delaying");
	state = DELAYING;
	// wait for some time and try again
	int delay_time = BASIC_DELAY + (int)intuniform(0, RANDOM_DELAY, RNG_MAC);
	assert(delay_time>0);
	setTimeout(delay_time);
	setRadioListen();
}

void Csma::gotoTransmit() {
	assert(tx_msg);
	MacPacket * msg = (MacPacket *)tx_msg->dup(); // delete tx msg itself later
	
	setRadioTransmit();

	msg->setLocalFrom(macid());
	// local_to should be set by upper layer
	printf(PRINT_MAC, "starting frame transmission->%d",msg->local_to);
	// schedule transmission
	reg_tx_data(msg); // statistics
	msg->local_from = macid();
	startTransmit(msg);

	state = TRANSMIT;
}

void Csma::rxFrame(MacPacket * msg) {
	assert(msg);
	if(msg->local_to == macid()) {
		printf(PRINT_MAC, "unicast frame received");	
		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
	} else if(msg->local_to == BROADCAST) {
		printf(PRINT_MAC, "local broadcast received");
		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
	} else {
		printf(PRINT_MAC, "overheard frame, not for me");
		reg_rx_overhear(msg);
		delete msg;
	}
	rxEnd();
}

void Csma::rxFailed() {
	rxEnd();
}

void Csma::rxEnd() {
	if(state == IDLE && tx_msg)
		gotoSensing();
}

void Csma::transmitDone() {
	assert(state == TRANSMIT);
	printf(PRINT_MAC, "transmit complete");
	// cleanup
	assert(tx_msg);
	txPacketDone(tx_msg); // report success
	tx_msg = NULL;

	++stat_tx;

	// we must delay sending for fairness
	gotoDelaying();
}

void Csma::timeout() {
	switch(state) {
		case SENSING:
			if(getRssi() < 0.5) {
				printf(PRINT_MAC, "channel clear");
				gotoTransmit();
			} else {
				printf(PRINT_MAC, "channel busy");
				gotoDelaying();
			}
			return;
		case DELAYING:
			printf(PRINT_MAC, "done delaying");
			if(tx_msg)
				gotoSensing();
			else
				gotoIdle();
			return;
		default:
			assert(false); // illegal state
	}
}

int Csma::headerLength() {
	// csma needs no special fields, just address
	return 4;
}
