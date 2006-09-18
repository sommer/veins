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


#include "mixim.h"
#include "message.h"

#include "csmaca.h"


Define_Module_Like( CsmaCA, MacClass );

void CsmaCA::initialize(){
	Mac::initialize();

	proto_state = PROTO_STATE_IDLE;
	proto_next_state = -1;
	tx_msg = NULL;
	nav_state = NAV_STATE_CLEAR;
	nav_end_time = 0;
	rts_contend_time = RTS_CONTEND_TIME;

	initTimers(2);

	// start listening
	setRadioListen();
}

CsmaCA::~CsmaCA()
{
	if (tx_msg)
		delete tx_msg;
}

void CsmaCA::txPacket(Packet * msg) {
	assert(msg);
	assert(msg->local_to != node->getNodeId());
	if(tx_msg) {
		printf(PRINT_MAC, "got message while busy");
		++stat_tx_drop;
		delete msg;
		return;
	}
	tx_msg = msg;
	evalState();
}

void CsmaCA::setIdle() {
	proto_state = PROTO_STATE_IDLE;
	evalState();
}

void CsmaCA::evalState() {
	if(proto_state == PROTO_STATE_IDLE && !isReceiving()) {
		// idling
		if(nav_state == NAV_STATE_CLEAR) {
			// listening / active state
			if(tx_msg) {
				if(mustUseCA(tx_msg)) {
					printf(PRINT_MAC, 
				"preparing to send RTS");
					// start contending
					proto_next_state = PROTO_STATE_SEND_RTS;
					startContending(rts_contend_time);
				} else {
					printf(PRINT_MAC, 
				"preparing to send data");
					proto_next_state = 
						PROTO_STATE_SEND_DATA;
					startContending(RTS_CONTEND_TIME);
				}
				return;
			}
			// nothing to do, listen
			printf(PRINT_MAC, "idle listening");
			setRadioListen();
		} else {
			printf(PRINT_MAC, "idle sleeping");
			setRadioSleep();
		}
	}
}

int CsmaCA::mustUseCA(Packet * pkt) {
	// use CA for all non-broadcast packets
	return pkt->local_to != BROADCAST;
}

void CsmaCA::startContending(int time) {
	assert(proto_next_state >= 0); // must have something todo
	assert(time >= 5);
	if(nav_state == NAV_STATE_BUSY) {
		printf(PRINT_MAC, "contend: skipping because nav is busy");
		proto_next_state = -1;
		setIdle();
	} else {
		proto_state = PROTO_STATE_CONTEND;
		int ctime = (int)intuniform(5, time);
		printf(PRINT_MAC, "starting contention, will fire in %d", ctime);
		setRadioListen(); 
		setProtocolTimeout(ctime);
	}
}

void CsmaCA::rxFrame(Packet * msg){
	assert(msg);
	Header *header = (Header *) msg->getData(MAC_DATA);
	if(proto_state == PROTO_STATE_WFCTS && 
			(header->kind != KIND_CTS 
			 || msg->local_to != node->getNodeId()))
	{
		printf(PRINT_MAC, "received packet, but not cts we want");
		incBackoff();
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
	}
			
	if(proto_state == PROTO_STATE_WFACK &&
			(header->kind != KIND_ACK
			 || msg->local_to != node->getNodeId()))
	{
		printf(PRINT_MAC, "received packet, but not ack we want");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
	}
			
	switch(header->kind) {
		case KIND_RTS:
			receiveRts(msg);
			break;
		case KIND_CTS:
			receiveCts(msg);
			break;
		case KIND_DATA:
			receiveData(msg);
			break;
		case KIND_ACK:
			receiveAck(msg);
			break;
		default:
			assert(false); // unknown msg
	}
	evalState();
}

void CsmaCA::transmitDone(){
	printf(PRINT_MAC, "transmitDone");
	switch(proto_state) {
		case PROTO_STATE_SEND_RTS:
			proto_state = PROTO_STATE_WFCTS;
			setProtocolTimeout(TIMEOUT_WFCTS);
			setRadioListen();
			break;
		case PROTO_STATE_SEND_CTS:
			proto_state = PROTO_STATE_WFDATA;
			setProtocolTimeout(TIMEOUT_WFDATA);
			setRadioListen();
			break;
		case PROTO_STATE_SEND_DATA:
			assert(tx_msg);
			if(tx_msg->local_to == BROADCAST) {
				txDone();
				setIdle();
			} else {
				proto_state = PROTO_STATE_WFACK;
				setProtocolTimeout(TIMEOUT_WFACK);
				setRadioListen();
			}
			break;
		case PROTO_STATE_SEND_ACK:
			setIdle();
			break;
		default:
			assert(false); // unknown
	}
}

void CsmaCA::txDone() {
	++stat_tx;
	tx_msg->setKind(TX_DONE);
	txPacketDone(tx_msg);
	tx_msg = NULL;
}

void CsmaCA::rxFailed() {
	evalState();
}

void CsmaCA::rxStarted() {
	// if we were contending, cancel it
	if(proto_state == PROTO_STATE_CONTEND) {
		printf(PRINT_MAC, "reception started, cancelling contention");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
		proto_next_state = -1; // none
	} else if(proto_state == PROTO_STATE_WFDATA) {
		printf(PRINT_MAC, "received start of packet, cancelling wait-for-data");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
	}
}

void CsmaCA::protocolTimeout() {
	int next_state;
	switch(proto_state) {
		case PROTO_STATE_CONTEND:
			assert(proto_next_state >= 0);
			assert(!isReceiving()); // should be cancelled
			assert(nav_state == NAV_STATE_CLEAR);
			// take a rssi sample, to be sure
			setRadioListen(); // make sure we sample the ether GPH
			if(getRssi()>0.5) { // someone in the air, restart 
				printf(PRINT_MAC, 
			"sensed communication, cancelling");
				setIdle();
				return;
			}
			// start the next state
			next_state = proto_next_state;
			proto_next_state = -1;
			switch(next_state) {
				case PROTO_STATE_SEND_RTS:
					sendRts();
					break;
				case PROTO_STATE_SEND_CTS:
					sendCts();
					break;
				case PROTO_STATE_SEND_DATA:
					sendData();
					break;
				case PROTO_STATE_SEND_ACK:
					sendAck();
					break;
				default: assert(false); // invalid
			}
			break;
		case PROTO_STATE_WFCTS:
			printf(PRINT_MAC, "wait-for-cts timeout");
			incBackoff();
			setIdle();	// retry
			break;
		case PROTO_STATE_WFDATA:
			printf(PRINT_MAC, "wait-for-data timeout");
			setIdle();
			break;
		case PROTO_STATE_WFACK:
			printf(PRINT_MAC, "wait-for-ack timeout");
			setIdle();
			break;
			
		default:
			assert(false); // invalid
	}
}

void CsmaCA::sendRts() {
	assert(tx_msg);
	assert(tx_msg->local_to != node->getNodeId());
	Header header;
	printf(PRINT_MAC, "sending rts -> %d", tx_msg->local_to);
	proto_state = PROTO_STATE_SEND_RTS;
	Packet * msg = new Packet("RTS");
	msg->local_from = node->getNodeId();
	msg->local_to = tx_msg->local_to;
	header.kind = KIND_RTS;
	header.nav = NAV_RTS(tx_msg->length());
	msg->setData(MAC_DATA, &header, sizeof(header), 0);
	msg->setLength(0);
	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void CsmaCA::sendCts() {
	printf(PRINT_MAC, "sending cts");
	proto_state = PROTO_STATE_SEND_CTS;
	Packet * msg = new Packet("CTS");
	Header header;
	msg->local_from = node->getNodeId();
	assert(cts_to >=0);
	msg->local_to = cts_to;
	header.kind = KIND_CTS;
	header.nav = (ushort)(cts_nav_end - node->getCurrentTime());
	msg->setData(MAC_DATA, &header, sizeof(header), 0);
	msg->setLength(0);
	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void CsmaCA::sendData() {
	printf(PRINT_MAC, "sending data");
	proto_state = PROTO_STATE_SEND_DATA;
	assert(tx_msg);
	assert(tx_msg->local_to != node->getNodeId());
	Packet * msg = (Packet *)tx_msg->dup();
	Header header;
	msg -> local_from = node->getNodeId();
	header.kind = KIND_DATA;
	msg->setData(MAC_DATA, &header, sizeof(header), 0);
	setRadioTransmit();
	reg_tx_data(msg);
	startTransmit(msg);
}

void CsmaCA::sendAck() {
	printf(PRINT_MAC, "sending ack");
	proto_state = PROTO_STATE_SEND_ACK;
	Packet * msg = new Packet("ACK");
	Header header;
	msg->local_from = node->getNodeId();
	assert(ack_to >= 0);
	msg->local_to = ack_to;
	header.kind = KIND_ACK;
	msg->setData(MAC_DATA, &header, sizeof(header), 0);
	msg->setLength(0);
	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void CsmaCA::receiveRts(Packet * msg) {
	assert(msg->local_to != BROADCAST);
	Header *header = (Header*)msg->getData(MAC_DATA);
	if(msg->local_to == node->getNodeId()) { 
		printf(PRINT_MAC, "received RTS, preparing for cts");
		reg_rx_overhead(msg);
		cts_to = msg->local_from;
		cts_nav_end = node->getCurrentTime() + (ushort)header->nav;
		cts_nav_rcv = header->nav;
		cts_nav_t = node->getCurrentTime();
		proto_next_state = PROTO_STATE_SEND_CTS;
		startContending(CTS_CONTEND_TIME);
	} else {
		printf(PRINT_MAC, "received RTS for %d (not for me)", msg->local_to);
		reg_rx_overhear(msg);
		updateNav(header->nav);
	}
	delete msg;
}

void CsmaCA::receiveCts(Packet * msg) {
	assert(msg->local_to != BROADCAST);
	Header *header = (Header*)msg->getData(MAC_DATA);
	if(msg->local_to == node->getNodeId()) {
		reg_rx_overhead(msg);
		if(proto_state != PROTO_STATE_WFCTS
				|| msg->local_from != tx_msg->local_to) 
		{
			printf(PRINT_MAC, "ignoring unsoll. cts");
		} else {
			cancelTimeout(TIMER_PROTOCOL);
			decBackoff();
			printf(PRINT_MAC, "received CTS, preparing to send data");
			proto_next_state = PROTO_STATE_SEND_DATA;
			startContending(DATA_CONTEND_TIME);
		}
	} else {
		printf(PRINT_MAC, "received CTS for %d (not for me)", msg->local_to);
		reg_rx_overhear(msg);
		updateNav(header->nav);
	}
	delete msg;
}

void CsmaCA::receiveAck(Packet * msg) {
	assert(msg->local_to != BROADCAST);
	if(msg->local_to == node->getNodeId()) {
		reg_rx_overhead(msg);
		if(proto_state != PROTO_STATE_WFACK
				|| msg->local_from != tx_msg->local_to) 
		{
			printf(PRINT_MAC, "ignoring unsoll. ack");
		} else {
			cancelTimeout(TIMER_PROTOCOL);
			printf(PRINT_MAC, "received ack");
			txDone();
			setIdle();
		}
	} else {
		printf(PRINT_MAC, "received ack for %d (not me)", msg->local_to);
		reg_rx_overhear(msg);
	}
	delete msg;
}

void CsmaCA::receiveData(Packet * msg) {
	if(msg->local_to == node->getNodeId()) {
		printf(PRINT_MAC, "received unicast packet");
		ack_to = msg->local_from;

		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;

		proto_next_state = PROTO_STATE_SEND_ACK;
		startContending(ACK_CONTEND_TIME);
	} else if(msg->local_to == BROADCAST) {
		printf(PRINT_MAC, "received broadcast packet");
		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
	} else {
		printf(PRINT_MAC, "overheard data packet");
		updateNav(NAV_ACK);
		reg_rx_overhear(msg);
		delete msg;
	}
}

void CsmaCA::updateNav(ushort t) {
	assert(t>0);
	ushort now = node->getCurrentTime();
	ushort nav_left = nav_end_time - now;
	if(nav_state ==  NAV_STATE_CLEAR || t > nav_left) {
		printf(PRINT_MAC, "updating NAV, left = %u", (unsigned)t);
		setNavTimeout(t);
		nav_state = NAV_STATE_BUSY;
		nav_end_time = t + now;
	}
}

void CsmaCA::navTimeout(){
	printf(PRINT_MAC, "NAV timer, medium clear now");
	nav_state = NAV_STATE_CLEAR;
	evalState();
}

int CsmaCA::headerLength() {
	/* To, from and type */
	return 5;
}

void CsmaCA::timeout(int which){
	switch(which) {
		case TIMER_PROTOCOL:
			protocolTimeout();
			break;
		case TIMER_NAV:
			navTimeout();
			break;
		default:
			assert(false); // unknown timer
	}
}

void CsmaCA::setProtocolTimeout(int t) {
	setTimeout(t, TIMER_PROTOCOL);
}

void CsmaCA::setNavTimeout(int t) {
	setTimeout(t, TIMER_NAV);
}

void CsmaCA::incBackoff() {
	rts_contend_time *= 2;
	if(rts_contend_time > MAX_RTS_CONTEND_TIME)
		rts_contend_time = MAX_RTS_CONTEND_TIME;
}

void CsmaCA::decBackoff() {
	rts_contend_time = RTS_CONTEND_TIME;
}

