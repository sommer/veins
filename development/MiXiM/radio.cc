#include "radio.h"

Define_Module(Radio);

double Radio::snr_threshold, Radio::noise_floor, Radio::rx_threshold, Radio::detect_threshold, Radio::max_tx_power;
bool Radio::parametersInitialised = false;

#define dB2W(x) pow(10.0, (double)(x) / 10.0) 

void Radio::initialize() {
	MiximBaseModule::initialize();
	printfNoInfo(PRINT_INIT, "\tRadio super layer initializing...");
	gate_to_mac = findGate("toMac");
	assert(gate_to_mac > 0);
	gate_to_air = findGate("toAir");
	assert(gate_to_air > 0);

	radio_state = RADIO_LISTEN;
	lp_mode = true;
	time_since_clear = time_since_listen = simTime();
	time_since_clear = last_stat_time = simTime();
	stat_time_sleep = stat_time_tx = stat_time_rx 
		= stat_time_collision = 0.0;
	stat_time_lb_rx = stat_time_lb_tx = 0.0;
	
	rx_state = RX_IDLE;
	scheduled_rx = scheduled_hdr_rx = NULL;
	
	if (!parametersInitialised) {
		parametersInitialised = true;
		noise_floor = dB2W(getDoubleParameter("noiseFloor", -116.1)) * 0.001;
		snr_threshold = dB2W(getDoubleParameter("snrThreshold", 10.0));
		rx_threshold = dB2W(getDoubleParameter("rxThreshold", -86.0)) * 0.001;
		max_tx_power = dB2W(getDoubleParameter("maxTxPower", 5.0)) * 0.001;
		if (rx_threshold < noise_floor * snr_threshold)
			printf(PRINT_RADIO, "Warning: rxThreshold < noiseFloor * snrThreshold");
		detect_threshold = dB2W(getDoubleParameter("detectThreshold", -96.0)) * 0.001;
		if (detect_threshold < noise_floor || detect_threshold > rx_threshold)
			error("detectThreshold should be between noiseFloor and rxThreshold\n");
	}
	current_source = -1;
	sum_noise = noise_floor;
	inCollision = false;
}

void Radio::finish() {
	// record last part
	record_stats();

	printfNoInfo(PRINT_INIT, "\tRadio super layer ending...");
	printf(PRINT_STATS, "stats: sleep=%.04lf tx=%.04lf rx=%.04lf tx_lb=%.04f rx_lb=%.04f collision=%.04lf",
			stat_time_sleep, stat_time_tx, stat_time_rx, 
			stat_time_lb_tx, stat_time_lb_rx, stat_time_collision);
	MiximBaseModule::finish();
}

Radio::~Radio() {
	parametersInitialised = false;
	transmitters.clear();
}

void Radio::handleMessage(cMessage *msg) {
	assert(msg);
/* 	printf_nr(PRINT_RADIO, "radio-handlemessage name: %s pointer=%p gate:",msg->name(),msg);
	if (msg->arrivalGate() != NULL) {
		printf_clr(PRINT_RADIO, "%s", msg->arrivalGate()->name());
		if (dynamic_cast<Packet*>(msg) != NULL)
			printf_clr(PRINT_RADIO, "[%d]", dynamic_cast<Packet*>(msg)->from);
		printf_clr(PRINT_RADIO, "\n");
	} else {
		//assert(msg==scheduled_rx);
		printf_clr(PRINT_RADIO, "(No gate)\n");
	} */
	if (radio_state == RADIO_SLEEP && (msg->kind() == NEIGHBOUR_TX || msg->kind() == TX)) {
		printf(PRINT_RADIO, "trying to rx/tx while asleep!");
		if(msg->hasPar("message")) {
			printf(PRINT_RADIO, "we had a incoming payload of %s",((cMessage*)(void*)msg->par("message"))->name());
			//assert(1==0);
		}
		delete msg;
		return;
	}
	switch(msg->kind()){
		case TX:
			printf(PRINT_RADIO, "Radio superclass got tx");
			assert_type(msg, Packet*);
			tx((Packet *)msg);
			break;
		case SET_TRANSMIT:
			printf(PRINT_RADIO, "Radio superclass got set transmit");
			setTransmit();
			delete msg;
			break;
		case SET_LISTEN:
			printf(PRINT_RADIO, "Radio superclass got set listen");
			setListen();
			delete msg;
			break;
		case SET_SLEEP:
			printf(PRINT_RADIO, "Radio superclass got set sleep");
			setSleep();
			delete msg;
			break;
		case NEIGHBOUR_START: {
			assert_type(msg, Packet *);
			Packet *packet = (Packet *) msg;
			printf(PRINT_RADIO, "Radio superclass got neighbour start from %d", packet->local_from);
			neighbourStart(packet->local_from, packet->power);
			delete msg;
			break;
		}
		case NEIGHBOUR_STOP: {
			assert_type(msg, Packet *);
			Packet *packet = (Packet *) msg;
			printf(PRINT_RADIO, "Radio superclass got neighbour stop from %d", packet->local_from);
			neighbourStop(packet->local_from, packet->power);
			delete msg;
			break;
		}
		case NEIGHBOUR_TX:
			printf(PRINT_RADIO, "Radio superclass got neighbour tx from %d", ((Packet *) msg)->local_from);
			assert_type(msg, Packet *);
			neighbourTx((Packet *)msg);
			break;
		case RX_END:
			printf(PRINT_RADIO, "Radio superclass got rx end");
			assert_type(msg, Packet *);
			rxEnd((Packet*)msg);
			break;
		case RX_HDR:
			printf(PRINT_RADIO, "Radio superclass got rx hdr");
			assert_type(msg, Packet *);
			rxHdr((Packet *)msg);
			break;
		case SET_NORMAL_POWER:
			printf(PRINT_RADIO, "Radio superclass got set normal power");
			record_stats();
			lp_mode = false;
			delete msg;
			break;
		case SET_LOW_POWER:
			printf(PRINT_RADIO, "Radio superclass got set low power");
			record_stats();
			lp_mode = true;
			delete msg;
			break;
		default:
			printf(PRINT_RADIO, "Radio got unknown msg kind");
			// unknown msg kind
			assert(false);
			delete msg;
	}
}

void Radio::record_stats() {
	double st = simTime();
	double dt = st - last_stat_time;
	last_stat_time = st;
	switch(radio_state) {
		case RADIO_LISTEN:
			if (lp_mode)
				stat_time_lb_rx += dt;
			else
				stat_time_rx += dt;
			break;
		case RADIO_TRANSMIT:
			if (lp_mode)
				stat_time_lb_tx += dt;
			else
				stat_time_tx += dt;
			break;
		case RADIO_SLEEP:
			stat_time_sleep += dt;
			break;
	}
}

void Radio::neighbourStart(int neigh, double power) {
	NeighbourInfo neighbourInfo;

	if (current_source != -1) {
		if (current_power / (sum_noise + power) < snr_threshold ) {
			sum_noise += current_power;
			neighbourInfo.id = current_source;
			neighbourInfo.power = current_power;
			transmitters.push_back(neighbourInfo);
			current_source = -1;
			if (rx_state == RX_IN_FRAME) {
				inCollision = true;
				printf(PRINT_RADIO, "collision causes corrupted frame %s", scheduled_rx->name());
				cancelRx();
				collision_start_time = simTime();
				neighbourInfo.id = neigh;
				neighbourInfo.power = power;
				transmitters.push_back(neighbourInfo);
				sum_noise += power;
			} else {
				if (power / sum_noise > snr_threshold) {
					current_source = neigh;
					current_power = power;
					time_since_clear = simTime();
				} else {
					printf(PRINT_RADIO, "collision before startsym");
					neighbourInfo.id = neigh;
					neighbourInfo.power = power;
					transmitters.push_back(neighbourInfo);
					sum_noise += power;
					collision_start_time = simTime();
					inCollision = true;
				}
			}
		} else {
			neighbourInfo.id = neigh;
			neighbourInfo.power = power;
			transmitters.push_back(neighbourInfo);
			sum_noise += power;
		}
	} else {
		if (!inCollision && power / sum_noise > snr_threshold && power + sum_noise > rx_threshold) {
			current_source = neigh;
			current_power = power;
			time_since_clear = simTime();
		} else {
			if (!inCollision && power / noise_floor > snr_threshold && power + sum_noise > rx_threshold) {
				inCollision = true;
				collision_start_time = simTime();
			}
			neighbourInfo.id = neigh;
			neighbourInfo.power = power;
			transmitters.push_back(neighbourInfo);
			sum_noise += power;
		}
	}
	sendRssi();
/*if (((Node *)parentModule()->submodule("node"))->getNodeId() == 36) {
	printf(PRINT_RADIO, "neigh %d, power %e, current_source %d, current_power %e, #transmitters %d, inCollision %d", neigh, power, current_source, current_power, transmitters.size(), inCollision);
}*/

/*	
	if(rx_state == RX_IN_FRAME) {
		assert(transmitters.size() == 1);
		assert(radio_state == RADIO_LISTEN);
		// frame interrupted
		printf(PRINT_RADIO, "collision causes corrupted frame %s", scheduled_rx->name());

		cancelRx();
	}
	if(transmitters.size() == 1 && radio_state == RADIO_LISTEN) {
		printf(PRINT_RADIO, "collision start");
		collision_start_time = simTime();
	}
	printf(PRINT_RADIO, "transmitter started from %d",neigh);
	transmitters.insert(transmitters.begin(),neigh);
	if(transmitters.size() == 1) { // clear from now
		time_since_clear = simTime();
	}
	sendRssi(); // calc rssi value
*/
}

void Radio::sendRssi() {
	double rssi;

	if (current_source != -1)
		rssi = 1.0;
	else if (sum_noise < detect_threshold)
		rssi = 0.0;
	else if (sum_noise < rx_threshold)
		rssi = 0.55;
	else
		rssi = 0.8;

/* 	if(transmitters.size() == 0) {
		rssi = 0.0; // no transmitters
	} else if(transmitters.size() == 1) {
		rssi = 1.0; // one clear transmission
	} else {
		rssi = 1.0 - uniform(0.0,0.2,RNG_RADIO); // some interference
	} */
	//printf(PRINT_RADIO, "rssi value now %.2lf",rssi);

	RssiMessage * msg = new RssiMessage("set_rssi", rssi);
	send(msg, gate_to_mac);
}

void Radio::cancelRx() {
	assert(scheduled_rx);
	if (scheduled_hdr_rx)
		delete cancelEvent(scheduled_hdr_rx);
	delete cancelEvent(scheduled_rx);
	scheduled_hdr_rx = scheduled_rx = NULL;
	rx_state = RX_IDLE;
	// notify mac of failed rx
	send(new cMessage("rxfail", RX_FAIL), gate_to_mac);
	record_stats();
	lp_mode = true;
}

/* Boolean returned indicates whether a signal could have been received in
   clear channel situation. */
void Radio::checkLoudestNoise() {
	vector<NeighbourInfo>::iterator iter, retval;
	
	if (transmitters.size() == 0) {
		sum_noise = noise_floor;
		return;
	}

	retval = iter = transmitters.begin();

	if (transmitters.size() > 1) {
		iter++;

		for (; iter != transmitters.end(); iter++) {
			if (retval->power < iter->power)
				retval = iter;
		}
	}

	double new_sum_noise = sum_noise - retval->power;
	if (new_sum_noise < noise_floor)
		new_sum_noise = noise_floor;
	
	/* sum_noise = new_sum_noise + retval->power, therefore we use it to check against rx_threshold */
	if (retval->power / new_sum_noise > snr_threshold && sum_noise > rx_threshold) {
		current_source = retval->id;
		current_power = retval->power;
		transmitters.erase(retval);
		sum_noise = new_sum_noise;
		time_since_clear = simTime();
	}
}

void Radio::neighbourStop(int neigh, double power) {
	assert(transmitters.size() > 0 || current_source != -1);
/* if (((Node *)parentModule()->submodule("node"))->getNodeId() == 52) {
	printf(PRINT_RADIO, "neigh %d, current_source %d, #transmitters %d", neigh, current_source, transmitters.size());
} */
	if (neigh == current_source) {
		if (rx_state == RX_IN_FRAME) {
			assert(radio_state == RADIO_LISTEN);
			cancelRx();
			printf(PRINT_RADIO, "sender stopped halfway frame");
		}

		// Check if we can now receive some other node
		current_source = -1;
		checkLoudestNoise();
	} else {
		assert(transmitters.size() > 0);
		vector<NeighbourInfo>::iterator iter;
		// Remove this neighbour from noise
		for (iter = transmitters.begin(); iter != transmitters.end(); iter++) {
			if (iter->id == neigh)
				break;
		}
		assert(iter != transmitters.end());

		sum_noise -= iter->power;
		if (sum_noise < noise_floor)
			sum_noise = noise_floor;

		transmitters.erase(iter);

		// Check if we can now receive some other node
		if (current_source == -1) {
			checkLoudestNoise();
			if (current_source != -1 && inCollision) {
				inCollision = false;
				printf(PRINT_RADIO, "collision end");
				if (radio_state == RADIO_LISTEN)
					stat_time_collision += simTime() - collision_start_time;
				time_since_clear = simTime();
			}
		}
	}
	sendRssi();

/*
	if(rx_state == RX_IN_FRAME) {
		assert(transmitters.size() == 1);
		assert(radio_state == RADIO_LISTEN);
		cancelRx();
		printf(PRINT_RADIO, "sender stopped halfway frame");
	}
	transmitters.erase(transmitters.begin());
	if(transmitters.size() == 1) {
		//transmitters.clear();
		// collision ended, clear from now
		time_since_clear = simTime();
		if(radio_state == RADIO_LISTEN) {
			printf(PRINT_RADIO, "collision end");
			stat_time_collision += simTime() - collision_start_time;
		}
	}
	sendRssi(); // calc rssi value
*/
}

void Radio::setSleep() {
	record_stats();
	if(inCollision && radio_state == RADIO_LISTEN) {
		printf(PRINT_RADIO, "collision end");
		inCollision = false;
		stat_time_collision += simTime() - collision_start_time;
	}
	if(radio_state == RADIO_TRANSMIT) {
		cMessage *msg = new cMessage("stoptransmit", TX_STOP);
		assert(msg->isName(NULL) == false);
		send(msg, gate_to_air);
	}
	if(radio_state != RADIO_SLEEP){
		printf(PRINT_RADIO, "sleep");
		if(rx_state == RX_IN_FRAME) {
			printf(PRINT_RADIO, "warning: setSleep interrupts rx");
			cancelRx();
		}
		radio_state = RADIO_SLEEP;
	}
}

void Radio::setListen() {
	record_stats();
	if(radio_state == RADIO_TRANSMIT) {
		cMessage *msg = new cMessage("stoptransmit", TX_STOP);
		assert(msg->isName(NULL) == false);
		send(msg, gate_to_air);
	}
	if(radio_state != RADIO_LISTEN){
		printf(PRINT_RADIO, "listen");
		time_since_listen = simTime();
		if(inCollision) {
			collision_start_time = simTime();
			printf(PRINT_RADIO, "collision start");
		}
		radio_state = RADIO_LISTEN;
		lp_mode = true;
	}
}

void Radio::setTransmit() {
	record_stats();
	if(inCollision && radio_state == RADIO_LISTEN) {
		printf(PRINT_RADIO, "collision end");
		stat_time_collision += simTime() - collision_start_time;
		inCollision = false;
	}
	if(radio_state != RADIO_TRANSMIT) {
		printf(PRINT_RADIO, "sending a transmit");
		if(rx_state == RX_IN_FRAME) {
			printf(PRINT_RADIO, "warning: setTransmit interrupts rx");
			cancelRx();
		}
		radio_state = RADIO_TRANSMIT;
		cMessage *msg = new cMessage("starttransmit", TX_START);
		assert(msg->isName(NULL) == false);
		send(msg, gate_to_air);
		lp_mode = true;
	}
}

void Radio::rxHdr(Packet *msg) {
	assert(scheduled_hdr_rx);
	assert(rx_state == RX_IN_FRAME);
	assert(radio_state == RADIO_LISTEN);
	assert(current_source != -1);
	send(msg, gate_to_mac);
	scheduled_hdr_rx = NULL;
}

void Radio::neighbourTx(Packet *msg) {
	printf(PRINT_RADIO, "msg incoming from %d", msg->local_from);

	assert(msg->local_from != current_source || rx_state == RX_IDLE);
	
	if (radio_state == RADIO_LISTEN &&
			simTime() - time_since_listen >= 0.9 * PREAMBLE_TIME &&
			simTime() - time_since_clear >= 0.9 * PREAMBLE_TIME &&
			current_source == msg->local_from) {
		Packet *hdr_msg;
		// set the receive state
		rx_state = RX_IN_FRAME;
		// notify the mac of this
		send(new cMessage("rxstart", RX_START), gate_to_mac);
		// schedule the reception of this message to self
		msg->setKind(RX_END);
		hdr_msg = (Packet*) (msg->dup());
		hdr_msg->setKind(RX_HDR);
		scheduleAt(simTime() + FRAME_DATA_TIME(((Mac*)(parentModule()->submodule("software")->submodule("mac")))->headerLength()-1), hdr_msg);
		scheduleAt(simTime() + FRAME_DATA_TIME(msg->length()), msg);
		// remember which msg we scheduled
		scheduled_rx = msg;
		scheduled_hdr_rx = hdr_msg;
		record_stats();
		lp_mode = false;
	} else {
		assert(radio_state != 0);
		delete msg; // nope can't hear that
	}
}

void Radio::rxEnd(Packet *msg) {
	assert(msg == scheduled_rx);
	assert(rx_state == RX_IN_FRAME);
	assert(radio_state == RADIO_LISTEN);
	assert(current_source != -1);
	// the packet was received successfully, forward to mac
	msg->setKind(RX);
	send(msg, gate_to_mac);
	// update state
	rx_state = RX_IDLE;
	scheduled_rx = NULL;
	record_stats();
	lp_mode = true;
}

void Radio::tx(Packet *msg) {
	assert(radio_state == RADIO_TRANSMIT);

	printf(PRINT_RADIO, "starting transmission");

	scheduleAt(simTime() + msg->getPreambleTime(), new cMessage("set normal power", SET_NORMAL_POWER));

	// don't actually transmit the preamble, but take
	// the preamble time into account
	printf(PRINT_RADIO, "name: %s gate: %s", msg->name(), msg->arrivalGate()->name());
	assert(msg->isName(NULL) == false);
	//~ sendDelayed(msg, PREAMBLE_TIME, gate_to_air);
	printf(PRINT_RADIO, "1 about to delay for %f sec", msg->getPreambleTime());
	assert(msg->getPreambleTime() > 0);
	sendDelayed(msg, msg->getPreambleTime(), gate_to_air);
	// after the frame is sent, report back
	// remember that turning off the radio is
	// based on a timer (extra transmit time)
	//~ double frame_time = PREAMBLE_TIME
	double frame_time = msg->getPreambleTime()
		+ FRAME_DATA_TIME(msg->length())
		+ EXTRA_TRANSMIT_TIME;
	scheduleAt(simTime() + frame_time, new cMessage("set low power", SET_LOW_POWER));

	cMessage *dmsg = new cMessage("txdone", TX_DONE);
	sendDelayed(dmsg, frame_time, gate_to_mac);
}

