#include "smac.h"

Define_Module_Like( SMac, EyesMacLayer );

void SMac::initialize() {
	EyesMacLayer::initialize();
	printfNoInfo("\t\tSMAC initializing...");

	// get params
	listen_time = getLongParameter("listenTime", LISTEN_TIME);
	assert(listen_time > 0);
	frame_time = getLongParameter("frameTime", FRAME_TIME);
	assert(frame_time < 65536);
	assert(frame_time >= listen_time);
	sleep_time = frame_time - listen_time;

	// init data
	sched_count = 0;
	sched_state = SCHED_STATE_STARTUP;
	time_last_sched = 0;
	proto_state = PROTO_STATE_IDLE;
	proto_next_state = PROTO_STATE_INVALID;
	tx_msg = NULL;
	nav_state = NAV_STATE_CLEAR;
	nav_end_time = 0;
	rts_contend_time = RTS_CONTEND_TIME;

	// set a timer before sending own SYNC
	int syncat = (int) intuniform(frame_time, 10 * frame_time, RNG_MAC);
	setSchedTimeout(syncat);

	// start listening
	setRadioListen();
}

void SMac::finish() {
	EyesMacLayer::finish();
	printfNoInfo("\t\tSMac ending...");
}

void SMac::txPacket(MacPacket * msg) {
	assert(msg);
	assert(msg->local_to != macid());
	if (tx_msg)
	{
		printf("got message while busy");
		++stat_tx_drop;
		delete msg;
		return;
	}
	packet_retries = PACKET_RETRIES;
	tx_msg = msg;
	evalState();
}

void SMac::setMySchedule(ushort time) {
	// my frame will start in <time>, which means it
	// started at <time>-<frame_time>
	assert(time < frame_time);
	time_last_sched = getCurrentTime() + time - frame_time;
	must_send_sync = 1;
	resync_counter = NEW_RESYNC_COUNTER;
	printf("schedule: %d", time_last_sched % frame_time);
	calcSchedState();
}

void SMac::setIdle() {
	proto_state = PROTO_STATE_IDLE;
	if (sched_state != SCHED_STATE_STARTUP)
		calcSchedState();
	evalState();
}

void SMac::evalState() {
	if (proto_state == PROTO_STATE_IDLE && !isReceiving()) {
		// idling
		if (nav_state == NAV_STATE_CLEAR && sched_state != SCHED_STATE_SLEEP) {
			// listening / active state
			if (must_send_sync) {
				printf("preparing to send SYNC %s",tx_msg?"(data pending)":"");
				// start contending
				proto_next_state = PROTO_STATE_SEND_SYNC;
				startContending(SYNC_CONTEND_TIME);
				return;
			}
			if (tx_msg && sched_state == SCHED_STATE_OWN) {
				if (mustUseCA(tx_msg)) {
					printf("preparing to send RTS");
					// start contending
					proto_next_state = PROTO_STATE_SEND_RTS;
					startContending(rts_contend_time);
				} else {
					printf("preparing to send data");
					proto_next_state = PROTO_STATE_SEND_DATA;
					startContending(RTS_CONTEND_TIME);
				}
				return;
			}
			// nothing to do, listen
			printf("idle listening");
			setRadioListen();
		} else {
			// sleep state
			printf("idle sleeping");
			setRadioSleep();
		}
	}
}

int SMac::mustUseCA(MacPacket * pkt) {
	// use CA for all non-broadcast packets
	return pkt->local_to != BROADCAST;
}

void SMac::startContending(int time) {
	assert(proto_next_state >= 0);	// must have something todo
	assert(time >= 5);
	if (nav_state == NAV_STATE_BUSY) {
		printf("contend: skipping because nav is busy");
		proto_next_state = PROTO_STATE_INVALID;
		setIdle();
	} else {
		proto_state = PROTO_STATE_CONTEND;
		int ctime = (int) intuniform(5, time, RNG_MAC);
		printf("starting contention, will fire in %d", ctime);
		setRadioListen();
		setProtocolTimeout(ctime);
	}
}

void SMac::rxFrame(MacPacket * msg) {
	assert(msg);
	if (sched_state == SCHED_STATE_SLEEP)
		return; // we're asleep, drop it 
	if (proto_state == PROTO_STATE_WFCTS && (PKT_KIND(msg) != KIND_CTS || msg->local_to != macid())) {
		printf("received packet, but not cts we want");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
		packet_retries--;
		if (packet_retries == 0)
			txDone(false);
	}

	if (proto_state == PROTO_STATE_WFACK && (PKT_KIND(msg) != KIND_ACK || msg->local_to != macid())) {
		printf("received packet, but not ack we want");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
		packet_retries--;
		if (packet_retries == 0)
			txDone(false);
	}

	assert(sched_state != SCHED_STATE_SLEEP);
	switch (PKT_KIND(msg)) {
		case KIND_SYNC:
			receiveSync(msg);
			break;
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
			assert(false);		// unknown msg
	}
	evalState();
}

void SMac::transmitDone() {
	printf("transmitDone");
	switch (proto_state) {
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
			if (tx_msg->local_to == BROADCAST) {
				txDone(true);
				setIdle();
			} else {
				proto_state = PROTO_STATE_WFACK;
				setProtocolTimeout(TIMEOUT_WFACK);
				setRadioListen();
			}
			break;
		case PROTO_STATE_SEND_SYNC:
		case PROTO_STATE_SEND_ACK:
			setIdle();
			break;
		default:
			assert(false);		// unknown
	}
}

void SMac::txDone(bool success) {
	if (success)
	{
		++stat_tx;
		txPacketDone(tx_msg);
	}
	else	
	{
		stat_tx_drop++;
		txPacketFail(tx_msg);
	}

	tx_msg = NULL;
}

void SMac::rxFailed() {
	if (sched_state != SCHED_STATE_SLEEP)
		evalState();
}

void SMac::rxStarted() {
	// if we were contending, cancel it
	if (proto_state == PROTO_STATE_CONTEND) {
		printf("reception started, cancelling contention");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
		proto_next_state = PROTO_STATE_INVALID;	// none
	} else if (proto_state == PROTO_STATE_WFDATA) {
		printf("received start of packet, cancelling wait-for-data");
		cancelTimeout(TIMER_PROTOCOL);
		proto_state = PROTO_STATE_IDLE;
	}
}

void SMac::protocolTimeout() {
	int next_state;
	switch (proto_state) {
		case PROTO_STATE_CONTEND:
			assert(proto_next_state >= 0);
			assert(!isReceiving());	// should be cancelled
			assert(nav_state == NAV_STATE_CLEAR);
			// take a rssi sample, to be sure
			setRadioListen(); // make sure we sample the ether GPH
			if(getRssi()>0.5) { // someone in the air, restart 
				printf(
			"sensed communication, cancelling");
				setIdle();
				return;
			}
			// start the next state
			next_state = proto_next_state;
			proto_next_state = PROTO_STATE_INVALID;
			switch (next_state) {
				case PROTO_STATE_SEND_SYNC:
					sendSync();
					break;
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
				default:
					assert(false);	// invalid
			}
			break;
		case PROTO_STATE_WFCTS:
			printf("wait-for-cts timeout");
			packet_retries--;
			if (packet_retries == 0)
				txDone(false);
			setIdle();			// retry
			break;
		case PROTO_STATE_WFDATA:
			printf("wait-for-data timeout");
			setIdle();
			break;
		case PROTO_STATE_WFACK:
			printf("wait-for-ack timeout");
			packet_retries--;
			if (packet_retries == 0)
				txDone(false);
			setIdle();
			break;

		default:
			assert(false);		// invalid
	}
}

void SMac::sendSync() {
	Header header;
	
	printf("sending sync");
	must_send_sync = 0;
	proto_state = PROTO_STATE_SEND_SYNC;
	MacPacket *msg = new MacPacket(this,"sync");
	msg->local_from = macid();
	msg->local_to = BROADCAST;
	header.kind = KIND_SYNC;
	ushort now = getCurrentTime();
	
	ushort stime = time_last_sched + frame_time - now;
	/* Sometimes the end of the sync will overrun the end of the frame
	   (multiple schedule issues). Then advertise the next next frame instead
	   of the next frame. */
	if (stime <= EST_SYNC_TIME)
		stime += frame_time;
	stime -= EST_SYNC_TIME;

	assert(stime > 0 && stime < frame_time);
	header.sync = stime;
	msg->setData(&header, sizeof(header), SYNC_SIZE);

	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void SMac::sendRts() {
	Header header;

	assert(tx_msg);
	assert(tx_msg->local_to != macid());
	printf("sending rts -> %d", tx_msg->local_to);
	proto_state = PROTO_STATE_SEND_RTS;
	MacPacket *msg = new MacPacket(this,"rts");
	msg->local_from = macid();
	msg->local_to = tx_msg->local_to;
	header.kind = KIND_RTS;
	header.nav = NAV_RTS(tx_msg->length());
	msg->setData(&header, sizeof(header), 0);

	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void SMac::sendCts() {
	Header header;

	printf("sending cts");
	proto_state = PROTO_STATE_SEND_CTS;
	MacPacket *msg = new MacPacket(this,"cts");
	msg->local_from = macid();
	assert(cts_to >= 0);
	msg->local_to = cts_to;
	header.kind = KIND_CTS;
	header.nav = (ushort) (cts_nav_end - getCurrentTime());
	assert(header.nav > 0 && header.nav < 100000);
	msg->setData(&header, sizeof(header), 0);

	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void SMac::sendData() {
	Header header;

	printf("sending data");
	proto_state = PROTO_STATE_SEND_DATA;
	assert(tx_msg);
	assert(tx_msg->local_to != macid());
	MacPacket *msg = (MacPacket *) tx_msg->dup();
	msg->local_from = macid();
	header.kind = KIND_DATA;
	msg->setData(&header, sizeof(header), 0);

	setRadioTransmit();
	reg_tx_data(msg);
	startTransmit(msg);
}

void SMac::sendAck() {
	Header header;

	printf("sending ack");
	proto_state = PROTO_STATE_SEND_ACK;
	MacPacket *msg = new MacPacket(this,"ack");
	msg->local_from = macid();
	assert(ack_to >= 0);
	msg->local_to = ack_to;
	header.kind = KIND_ACK;
	msg->setData(&header, sizeof(header), 0);

	setRadioTransmit();
	reg_tx_overhead(msg);
	startTransmit(msg);
}

void SMac::receiveSync(MacPacket * msg) {
	assert(msg);
	reg_rx_overhead(msg);
	ushort stime = (ushort) PKT_SYNC(msg);
	delete msg;
	if (sched_state == SCHED_STATE_STARTUP) {
		cancelTimeout(TIMER_SCHED);
		printf("received SYNC, following");
		setMySchedule(stime);
		return;
	}
	// check offset
	int ftime = (int)getCurrentTime() + stime;	// frame start time
	if (isSameSchedule(ftime, time_last_sched)) {
		printf("received SYNC, my own schedule");
		return;
	}
	// check other schedules
	int i;
	for (i = 0; i < sched_count; i++) {
		if (isSameSchedule(ftime, time_last_sched + sched_table[i])) {
			printf("received SYNC, known schedule");
			return;
		}
	}
	// new schedule
	printf("adopting foreign schedule");
	int offset = ftime - time_last_sched;
	if (offset < 0)
		offset += 65536;
	offset %= frame_time;
	assert(offset > 0);
	//assert(offset < (int)frame_time);
	adoptSchedule(offset);
}

void SMac::receiveRts(MacPacket * msg) {
	assert(msg->local_to != BROADCAST);
	if (msg->local_to == macid()) {
		printf("received RTS, preparing for cts");
		reg_rx_overhead(msg);
		cts_to = msg->local_from;
		cts_nav_end = getCurrentTime() + (ushort) PKT_NAV(msg);
		cts_nav_rcv = PKT_NAV(msg);
		cts_nav_t = getCurrentTime();
		proto_next_state = PROTO_STATE_SEND_CTS;
		startContending(CTS_CONTEND_TIME);
	} else {
		printf("received RTS for %d (not for me)", msg->local_to);
		reg_rx_overhear(msg);
		updateNav(PKT_NAV(msg));
	}
	delete msg;
}

void SMac::receiveCts(MacPacket * msg) {
	assert(msg->local_to != BROADCAST);
	if (msg->local_to == macid()) {
		reg_rx_overhead(msg);
		if (proto_state != PROTO_STATE_WFCTS || msg->local_from != tx_msg->local_to) {
			printf("ignoring unsoll. cts");
		} else {
			cancelTimeout(TIMER_PROTOCOL);
			decBackoff();
			printf("received CTS, preparing to send data");
			proto_next_state = PROTO_STATE_SEND_DATA;
			startContending(DATA_CONTEND_TIME);
		}
	} else {
		printf("received CTS for %d (not for me)", msg->local_to);
		reg_rx_overhear(msg);
		updateNav(PKT_NAV(msg));
	}
	delete msg;
}

void SMac::receiveAck(MacPacket * msg) {
	assert(msg->local_to != BROADCAST);
	if (msg->local_to == macid()) {
		reg_rx_overhead(msg);
		if (proto_state != PROTO_STATE_WFACK || msg->local_from != tx_msg->local_to) {
			printf("ignoring unsoll. ack");
		} else {
			cancelTimeout(TIMER_PROTOCOL);
			printf("received ack");
			txDone(true);
			setIdle();
		}
	} else {
		printf("received ack for %d (not me)", msg->local_to);
		reg_rx_overhear(msg);
	}
	delete msg;
}

void SMac::receiveData(MacPacket * msg) {
	if (msg->local_to == macid()) {
		printf("received unicast packet");
		ack_to = msg->local_from;

		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
		
		proto_next_state = PROTO_STATE_SEND_ACK;
		startContending(ACK_CONTEND_TIME);
	} else if (msg->local_to == BROADCAST) {
		printf("received broadcast packet");
		reg_rx_data(msg);
		rxPacket(msg);
		++stat_rx;
	} else {
		printf("overheard data packet");
		updateNav(NAV_ACK);
		reg_rx_overhear(msg);
		delete msg;
	}
}


void SMac::adoptSchedule(int offset) {
	assert(offset >= 0);
	if (sched_count < 10) {
		sched_table[sched_count++] = offset;
		must_send_sync = 1;
		switch (sched_state) {
			case SCHED_STATE_SLEEP:
				// ? how did i receive this
				assert(false);
			case SCHED_STATE_OWN:
			case SCHED_STATE_OTHER:
				// do nothing, calc at end of listen time
				break;
			case SCHED_STATE_STARTUP:
				assert(false);	// invalid state
		}
	}
}

void SMac::calcSchedState() {
	ushort t = getCurrentTime();
	int in_f = (ushort) (t - time_last_sched);
	assert(listen_time>0);
	// check if we are in our own send time (this frame)
	if ( /*in_f >=0 && */ (unsigned)in_f < listen_time) {
		sched_state = SCHED_STATE_OWN;
		ushort left = listen_time - in_f;
		assert(left <= listen_time);
		printf("calc_sched: in own frame, left = %u", (unsigned) left);
		setSchedTimeout(left);
		return;
	}
	// check if we are in the next listen frame
	if ((unsigned)in_f > frame_time - 5 && (unsigned)in_f < frame_time + listen_time) {
		time_last_sched += frame_time;
		in_f -= frame_time;
		if (in_f == 0) { //check for start of frame
			resync_counter--;
			if (resync_counter <= 0) {
				must_send_sync = 1;
				resync_counter = NEW_RESYNC_COUNTER;
			}
		}
		sched_state = SCHED_STATE_OWN;
		ushort left = listen_time - in_f;
		assert(left <= listen_time + 5);
		printf("calc_sched: in next frame, left = %u", (unsigned) left);
		setSchedTimeout(left);
		return;
	}
	// check if we are in any other listen frame
	assert((unsigned)in_f < frame_time);
	int i;
	for (i = 0; i < sched_count; i++) {
		if (in_f > sched_table[i] && (unsigned)in_f < sched_table[i] + listen_time) {
			// yep.
			sched_state = SCHED_STATE_OTHER;
			unsigned end_in_f = sched_table[i] + listen_time;
			if (end_in_f > frame_time) {
				// don't miss own interval
				end_in_f = frame_time;
			}
			ushort left = end_in_f - in_f;
			assert(left <= listen_time + 5);
			printf("calc_sched: in foreign frame %d, left = %u", i, (unsigned) left);
			setSchedTimeout(left);
			return;
		}
	}
	// not in any listen frame. calc sleep time
	sched_state = SCHED_STATE_SLEEP;
	unsigned wake_at = frame_time;	// at most to own frame
	// check if there is a frame that starts earlier
	for (i = 0; i < sched_count; i++) {
		if (sched_table[i] > in_f && sched_table[i] < wake_at)
			wake_at = sched_table[i];
	}
	ushort left = wake_at - in_f;
	assert(left <= sleep_time);
	printf("calc_sched: in no frame, left = %u", (unsigned) left);
	setSchedTimeout(left);
	setRadioSleep();
	return;
}

void SMac::updateNav(ushort t) {
	assert(t > 0);
	ushort now = getCurrentTime();
	ushort nav_left = nav_end_time - now;
	if (nav_state == NAV_STATE_CLEAR || t > nav_left) {
		printf("updating NAV, left = %u", (unsigned) t);
		setNavTimeout(t);
		nav_state = NAV_STATE_BUSY;
		nav_end_time = t + now;
	}
}

void SMac::navTimeout() {
	printf("NAV timer, medium clear now");
	nav_state = NAV_STATE_CLEAR;
	evalState();
}

void SMac::schedTimeout() {
	switch (sched_state) {
		case SCHED_STATE_STARTUP:
			printf("no schedule heard, adopting my own");
			printf("own sched");
			setMySchedule(0);
			evalState();
			break;
		default:
			calcSchedState();
			evalState();
	}
}

int SMac::headerLength() {
	/* To, from and type fields */
	return 5;
}

void SMac::timeout(int which) {
	switch (which) {
		case TIMER_PROTOCOL:
			protocolTimeout();
			break;
		case TIMER_NAV:
			navTimeout();
			break;
		case TIMER_SCHED:
			schedTimeout();
			break;
		default:
			assert(false);		// unknown timer
	}
}

void SMac::setProtocolTimeout(int t) {
	setTimeout(t, TIMER_PROTOCOL);
}

void SMac::setNavTimeout(int t) {
	setTimeout(t, TIMER_NAV);
}

void SMac::setSchedTimeout(int t) {
	setTimeout(t, TIMER_SCHED);
}

int SMac::isSameSchedule(int time1, int time2) {
	int offset = time1 - time2;
	if (offset < 0)
		offset += 65536;
	return abs((int)(offset%frame_time)) < ALLOWED_DRIFT;
}

/*void SMac::incBackoff()
{
	rts_contend_time *= 2;
	if (rts_contend_time > MAX_RTS_CONTEND_TIME)
		rts_contend_time = MAX_RTS_CONTEND_TIME;
}*/

void SMac::decBackoff() {
	rts_contend_time = RTS_CONTEND_TIME;
}

void SMac::endForce() {
	if (sched_state == SCHED_STATE_SLEEP)
		setRadioSleep();

	calcSchedState();
	evalState();
}
