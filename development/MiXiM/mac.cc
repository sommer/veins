#include "mac.h"

//Define_Module(Mac);

bool Mac::parametersInitialised = false, Mac::parametersPrinted;
int Mac::siftNodes;
Mac::SiftData *Mac::siftDistribution = NULL;
int Mac::maxSiftDistributions = 0;
int Mac::siftDistributions = 0;

vector<Mac::ParameterPair> Mac::parameterList;

void Mac::initialize() {
	MiximBaseModule::initialize();

	printfNoInfo(PRINT_INIT, "\tMac super layer initializing...");
	cModule* appContainer = parentModule();
	cModule* nodeContainer = appContainer->parentModule();
	node =  (Node*) nodeContainer->submodule("node");
	radio = (Radio*) nodeContainer->submodule("radio");

	if (!parametersInitialised) {
		parametersInitialised = true;
		siftNodes = par("siftNodes");
		parametersPrinted = false;
	}

	//FIXME: make these static?
	lpl = par("lpl");
	if (lpl) {
		initTimers(TIMERS + 1); // +1 for internal timeout
		on_time = par("lpl_on");
		off_time = par("lpl_off");
		if (on_time == 0) on_time = ON_TIME;
		if (off_time == 0) off_time = OFF_TIME;
		setInternalTimeout((int) intuniform(1, off_time, RNG_MAC));

		// preamble should be long enough that enough is always received
		preamble_time = PREAMBLE_TIME + (on_time + off_time) / 32768.0;
	} else {
		initTimers(TIMERS);
		preamble_time = PREAMBLE_TIME;
	}

	send_state = false;
	
	stat_tx = 0;
	stat_rx = 0;
	stat_tx_drop = 0;

	stat_time_rx_data =
		stat_time_rx_overhead =
		stat_time_rx_overhear =
		stat_time_tx_data =
		stat_time_tx_overhead = 0.0;

	rssi = 0.0;
	recv_state = SENSE;
	send_state = 0;
	tx_preferred = 0;
	force = FORCE_NORMAL;
	force_sleep = false;
}

void Mac::finish() {
	
	printfNoInfo(PRINT_INIT, "\tMac super layer ending...");
	printf(PRINT_STATS, "stats: rx=%u tx=%u tx_drop=%u "
			"s_rx_data=%.4lf s_rx_overhead=%.4lf "
			"s_rx_overhear=%.4lf "
			"s_tx_data=%.4lf s_tx_overhead=%.4lf",
		stat_rx, stat_tx, stat_tx_drop,
		stat_time_rx_data, stat_time_rx_overhead,
		stat_time_rx_overhear,
		stat_time_tx_data, stat_time_tx_overhead);

	if (!parametersPrinted) {
		printf_nr(PRINT_STATS, "mac=%s lpl=%s", className(), lpl ? "true" : "false");
		for (vector<ParameterPair>::iterator iter = parameterList.begin(); iter != parameterList.end(); iter++)
			printf_clr(PRINT_STATS, " %s=%s", iter->first, iter->second.c_str());
		printf_clr(PRINT_STATS, "\n");
		parametersPrinted = true;
	}

	MiximBaseModule::finish();
}

Mac::~Mac() {
	int i;
	
	if (parametersInitialised) {
		parametersInitialised = false;

		for (i = 0; i < siftDistributions; i++)
			free(siftDistribution[i].chances);
		
		parameterList.clear();
	}
}

double Mac::getRssi() {
	return rssi;
}

bool Mac::isReceiving() {
	return recv_state == FRAME;
}

const ForceState Mac::getForce() {
	return force;
}

void Mac::startTransmit(Packet* msg) {
	assert(!send_state); // not already transmitting
	assert(msg);

	msg->setPreambleTime(preamble_time);
	// send a copy, not the packet itself
// disabled: why a copy??
// Packet *d = (Packet*)msg->dup();
	assert_type(msg, Packet*);

	send_state = true;

	msg->increaseLength(headerLength()); // add protocol overhead

	// transfer to L1
	msg->setKind(TX);
	assert(msg->isName(NULL) == false);
	send(msg, findGate("toRadio"));
}

void Mac::txPacketDone(Packet *msg) {
	tx_preferred = 0;
	send(msg, findGate("toRouting"));
}

void Mac::reg_tx_data(Packet* p) {
	stat_time_tx_data += frameTotalTime(p->length()+headerLength());
}

void Mac::cancelTimeout(int which) {
	assert(which >= 0 && which < TIMERS);
	cancelTimer(which);
}

void Mac::reg_rx_overhead(Packet* p) {
	stat_time_rx_overhead += frameTotalTime(p->length()+headerLength());
}

void Mac::reg_rx_overhear(Packet* p) {
	stat_time_rx_overhear += frameTotalTime(p->length()+headerLength());
}

void Mac::reg_rx_data(Packet* p) {
	stat_time_rx_data += frameTotalTime(p->length()+headerLength());
}

void Mac::setTimeout(int ticks, int which) {
	assert(ticks > 0);
	assert(which >= 0 && which < TIMERS);
	// the actual timeout occurs at the next tb tick
	double thenTime = node->getThenTime(ticks);
	assert(thenTime > simTime());
	setTimer(which, thenTime - simTime());
}

void Mac::rxPacket(Packet* msg) {
	send(msg, findGate("toRouting"));
}

int Mac::headerLength() {
	// override this!
	assert(0);
	return -1;
}

void Mac::handleTimer(unsigned int index) {
	if (index < TIMERS)
		timeout(index);
	else	
		internalTimeout();
}

void Mac::handleEvent(cMessage *msg) {
	assert(msg);
	Packet *packet = dynamic_cast<Packet *>(msg);

	switch(msg->kind()) {
		case TX:
			assert(packet);
			assert(packet->local_to != node->getNodeId());
			if (packet->serial != -1)
				printf(PRINT_MAPPER, "MAPPER: TX, %d %f %d", node->getNodeId(), simTime(), packet->serial);
			txPacket(packet); // relay to implementation
			break;
		case RX:
			assert(packet);
			if (packet->serial != -1)
				printf(PRINT_MAPPER, "MAPPER: RX, %d %f %d %d", node->getNodeId(), simTime(), packet->serial, packet->local_from);
			rx((Packet *)packet);
			break;
		case RX_HDR:
			assert(packet);
			rxHeader(packet);
			delete packet;
			break;
		case TX_DONE:
			txDone();
			delete msg;
			break;
		case RX_START:
			rxStart();
			delete msg;
			break;
		case RX_FAIL:
			//~ printf(PRINT_MAC, "RX_FAIL");
			rxFail();
			delete msg;
			break;
		case RX_COLLISION:
			// tell mac about packets not received due to
			// collisions normally this would go through
			// RX_FAIL messages but because preambles aren't
			// part of the message we have to do this as well
			//rxFailed();
			delete msg;
			break;
		case SET_RSSI:
			assert_type(msg, RssiMessage *);
			rssi = ((RssiMessage *) msg)->rssi;
			if (lpl && rssi < 0.5 && recv_state == PREAMBLE_DETECT) {
				cancelInternalTimeout();
				internalTimeout();
			}
			delete msg;
			break;
		case PREFER_TX:
			tx_preferred = 1;
			delete msg;
			break;
		case FORCE_AWAKE_REQ:
			forceRequest();
			delete msg;
			break;
		case FORCE_END:
			if (force == FORCE_SLEEP_WANTED) {
				cMessage *newMessage = new cMessage("setradiosleep", SET_SLEEP);
				send(newMessage, findGate("toRadio"));
			}
			force = FORCE_NORMAL;
			printf(PRINT_MAC, "Ending forced state");
			endForce();
			delete msg;
			break;
		default:
			assert(false); // unkown
	}
}

// default implementation
void Mac::timeout() {
}

void Mac::timeout(int which) {
	if(which == 0)
		timeout();
}

void Mac::internalTimeout() {
	assert(lpl);
	force_sleep = false;
	switch (recv_state) {
		case SENSE:
			if (rssi < 0.5) {
				// nothing interesting detected
				setRadioSleepInternal();
				recv_state = SLEEP;
				setInternalTimeout(off_time);
			} else {
				// may have found a preamble
				printf(PRINT_MAC, "channel busy, starting preamble search");
				recv_state = PREAMBLE_DETECT;
				setInternalTimeout((int) (off_time+on_time+ceil(PREAMBLE_TIME*32768.0)));
			}
			break;
		case FRAME:
			// when a frame is detected the timer should be
			// cancelled
			assert(false);  
			break;
		case PREAMBLE_DETECT:
			// didn't find start symbol
			printf(PRINT_MAC, "didn't find start symbol, going back to sleep");
			// make sure we know we are not receiving anymore
			rxFailed();     
			setRadioSleepInternal();
			recv_state = SLEEP;
			setInternalTimeout(off_time);
			break;
		case SLEEP:
			// this will automatically set the right timeout
			setRadioListen();       
			break;
		default:
			assert(false);
	}
}

void Mac::setInternalTimeout(int ticks) {
	assert(lpl);
	assert(ticks > 0);
	double thenTime = node->getThenTime(ticks);
	assert(thenTime > simTime());
	setTimer(TIMERS, thenTime - simTime());
}

void Mac::setRadioSleepInternal() {
	if (force == FORCE_NOSLEEP) {
		force = FORCE_SLEEP_WANTED;
	} else {
		cMessage *msg = new cMessage("setradiosleep", SET_SLEEP);
		send(msg, findGate("toRadio"));
	}
}

void Mac::setRadioListen() {
	if (force == FORCE_SLEEP_WANTED)
		force = FORCE_NOSLEEP;
	cMessage *msg = new cMessage("setradiolisten", SET_LISTEN);
	send(msg, findGate("toRadio"));
	// for low power listening: if we are not in the process of receiving
	// either a frame or a possible preamble, go to SENSE state
	if (lpl && recv_state != FRAME && recv_state != PREAMBLE_DETECT && !force_sleep) {
		cancelInternalTimeout();
		setInternalTimeout(on_time);
		recv_state = SENSE;
	}
}

void Mac::setRadioTransmit() {
	if (force == FORCE_SLEEP_WANTED)
		force = FORCE_NOSLEEP;
	cMessage *msg = new cMessage("setradiotransmit", SET_TRANSMIT);
	send(msg, findGate("toRadio"));
	if (lpl) {
		cancelInternalTimeout();
		recv_state = SLEEP;
		force_sleep = false;
	}
}

void Mac::setRadioSleep() {
	setRadioSleepInternal();
	if(lpl) {
		cancelInternalTimeout();
		recv_state = SLEEP;
		force_sleep = false;
	}
}

void Mac::cancelInternalTimeout() {
	assert(lpl);
	cancelTimer(TIMERS);
}

void Mac::txDone() {
	assert(send_state);
	send_state = false;
	transmitDone(); // relay to implementation
}

void Mac::rxStart() {
	//TODO: should it also allow the SENSE state ?
	assert(recv_state == PREAMBLE_DETECT || recv_state == SENSE);
	recv_state = FRAME;
	if (lpl)
		cancelInternalTimeout();
	rxStarted(); // relay to implementation
}

void Mac::rxFail() {
	if (lpl) {
		if (recv_state == FRAME) {
			recv_state = SLEEP;
			setRadioSleepInternal();
			setInternalTimeout(off_time);
			force_sleep = true;
		}
	} else
		recv_state = SENSE;
	rxFailed(); // relay to implementation
}

/**
 * received frame
 */
void Mac::rx(Packet * msg) {
	if (force == FORCE_SLEEP_WANTED) // we're meant to be asleep, so ignore
	{
		delete msg;
		return;
	}
		
	assert(recv_state == FRAME);
	int hdrlen = headerLength(); // get from implementation
	assert(msg->length() >= hdrlen);
	msg->decreaseLength(hdrlen); // strip header
	if (lpl) {
		// the transmitter will be on for a little while longer
		recv_state = SLEEP;
		setRadioSleepInternal();
		setInternalTimeout(2);
		force_sleep = true;
	} else
		recv_state = SENSE;

	rxFrame(msg); // relay to implementation
}

void Mac::reg_tx_overhead(Packet *p) {
	stat_time_tx_overhead += frameTotalTime(p->length() + headerLength());
}

void Mac::forceRequest()
{
	forceGrant(true);
}

void Mac::forceGrant(bool success)
{
	force = FORCE_NOSLEEP;
	printf(PRINT_MAC, "Forcing awake");
	Packet *packet = new Packet("Force grant",FORCE_AWAKE_GRANT);
	packet->serial = success;
	send(packet, findGate("toRouting"));
}

void Mac::eatCycles(unsigned cycles) {
	node->eatCycles(cycles);
}

int Mac::siftSlot(int low, int high) {
	int slots = high - low;
	int i, distribution;
	double picked;

	if (low >= high)
		return low;
	
	for (i = 0; i < siftDistributions; i++) {
		if (siftDistribution[i].slots == slots)
			break;
	}

	if (i == siftDistributions) {
		double alpha = pow((double) siftNodes, - (1.0 / (slots - 1)));
		double base = ((1.0 - alpha) * pow(alpha, slots)) / (1.0 - pow(alpha, slots));

		distribution = siftDistributions;

		if (siftDistributions == maxSiftDistributions) {
			int newMax = maxSiftDistributions == 0 ? 2 : maxSiftDistributions * 2;
			siftDistribution = (SiftData *) realloc(siftDistribution, newMax * sizeof(SiftData));
			if (siftDistribution == NULL) {
				fprintf(stderr, "Out of memory\n");
				exit(EXIT_FAILURE);
			}
			for (i = maxSiftDistributions; i < newMax; i++)
				siftDistribution[i].chances = NULL;

			maxSiftDistributions = newMax;
		}
		siftDistribution[distribution].slots = slots;
		siftDistribution[distribution].chances = (double *) malloc(slots * sizeof(double));
		if (siftDistribution[distribution].chances == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}

		// Create new distribution
		siftDistribution[distribution].chances[0] = base;
		for (i = 1; i < slots; i++)
			siftDistribution[distribution].chances[i] = base * pow(alpha, -i) + siftDistribution[distribution].chances[i - 1];

		siftDistributions++;
	} else {
		distribution = i;
	}

	picked = uniform(0.0, 1.0, RNG_MAC);

	/* This can be done quicker with a proper binary search, but for now I don't
	   care. */
	for (i = slots - 1; i >= 0; i--)
		if (siftDistribution[distribution].chances[i] < picked)
			return i + 1 + low;
	return low;
}

std::string Mac::getParameter(const char *parameter) {
	std::string result = MiximSoftwareModule::getParameter(parameter);

	for (vector<ParameterPair>::iterator iter = parameterList.begin(); iter != parameterList.end(); iter++)
		if (strcmp(iter->first, parameter) == 0 && iter->second == result)
			return result;

	parameterList.push_back(ParameterPair(parameter, result));
	return result;
}

