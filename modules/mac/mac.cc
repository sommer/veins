#include "mac.h"
#include "FindModule.h"
#include "NicControlType.h"
#include "MacControlInfo.h"

using namespace std;

Define_Module(EyesMacLayer);

bool EyesMacLayer::parametersInitialised = false, EyesMacLayer::parametersPrinted;
int EyesMacLayer::siftNodes;
EyesMacLayer::SiftData *EyesMacLayer::siftDistribution = NULL;
int EyesMacLayer::maxSiftDistributions = 0;
int EyesMacLayer::siftDistributions = 0;

vector<EyesMacLayer::ParameterPair> EyesMacLayer::parameterList;

void EyesMacLayer::initialize(int stage) {
	BaseMacLayer::initialize(stage);
	if (stage!=3)
		return;

	printfNoInfo("\tMac super layer initializing...");
	//cModule* appContainer = getParentModule();
	node = (BaseModule*)(getNode());
	radio = FindModule<BasePhyLayer*>::findSubModule(node);
	assert(radio!=NULL);

	if (!parametersInitialised) {
		parametersInitialised = true;
		if (hasPar("siftNodes"))
			siftNodes = par("siftNodes");
		else
			siftNodes = 100;
		parametersPrinted = false;
	}

	//FIXME: make these static?
	lpl = hasPar("lpl") && par("lpl");
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
	EV << "recv state " << (int)recv_state << endl;
	send_state = 0;
	force_sleep = false;
	initialize();
}

void EyesMacLayer::finish() {

	printfNoInfo("\tMac super layer ending...");
	printf("stats: rx=%u tx=%u tx_drop=%u "
			"s_rx_data=%.4lf s_rx_overhead=%.4lf "
			"s_rx_overhear=%.4lf "
			"s_tx_data=%.4lf s_tx_overhead=%.4lf",
		stat_rx, stat_tx, stat_tx_drop,
		stat_time_rx_data, stat_time_rx_overhead,
		stat_time_rx_overhear,
		stat_time_tx_data, stat_time_tx_overhead);

	if (!parametersPrinted) {
		printf_nr("mac=%s lpl=%s", getClassName(), lpl ? "true" : "false");
		for (vector<ParameterPair>::iterator iter = parameterList.begin(); iter != parameterList.end(); iter++)
			printf_clr(" %s=%s", iter->first, iter->second.c_str());
		printf_clr("\n");
		parametersPrinted = true;
	}

	BaseMacLayer::finish();
}

EyesMacLayer::~EyesMacLayer() {
	int i;

	if (parametersInitialised) {
		parametersInitialised = false;

		for (i = 0; i < siftDistributions; i++)
			free(siftDistribution[i].chances);

		parameterList.clear();
	}
}

double EyesMacLayer::getRssi() {
	return rssi;
}

bool EyesMacLayer::isReceiving() {
	return recv_state == FRAME;
}

void EyesMacLayer::startTransmit(MacPacket* msg) {
	assert(!send_state); // not already transmitting
	assert(msg);

	msg->setPreambleTime(preamble_time);
	// send a copy, not the packet itself
// disabled: why a copy??
// MacPacket *d = (MacPacket*)msg->dup();
	//assert_type(msg, MacPacket*);

	send_state = true;

	msg->increaseLength(headerLength()); // add protocol overhead

	// transfer to L1
	msg->setKind(TX);
	assert(msg->isName(NULL) == false);
	sendDown(msg);
}

void EyesMacLayer::txPacketDone(MacPacket *msg) {
	msg->setKind(NicControlType::TX_END);
	sendControlUp(msg);
}

void EyesMacLayer::txPacketFail(MacPacket *msg) {
	msg->setKind(NicControlType::TX_FAIL);
	sendControlUp(msg);
}

void EyesMacLayer::reg_tx_data(MacPacket* p) {
	stat_time_tx_data += frameTotalTime(p->getBitLength()+headerLength());
}

void EyesMacLayer::reg_rx_overhead(MacPacket* p) {
	stat_time_rx_overhead += frameTotalTime(p->getBitLength()+headerLength());
}

void EyesMacLayer::reg_rx_overhear(MacPacket* p) {
	stat_time_rx_overhear += frameTotalTime(p->getBitLength()+headerLength());
}

void EyesMacLayer::reg_rx_data(MacPacket* p) {
	stat_time_rx_data += frameTotalTime(p->getBitLength()+headerLength());
}

simtime_t EyesMacLayer::getThenTime(unsigned short ticks) {
	simtime_t then = simTime() * 32768 /*+ clock_skew*/;
	then += ticks;
	return (then /*- clock_skew*/) / 32768;
}

void EyesMacLayer::setTimeout(int ticks, int which) {
	assert(ticks > 0);
	assert(which >= 0 && which < TIMERS);
	// the actual timeout occurs at the next tb tick
	simtime_t thenTime = getThenTime(ticks);
	assert(thenTime > simTime());
	simtime_t val = thenTime - simTime();
	setTimer(which, val);
	EV << "setting timer "<<which<<" for " << thenTime<<endl;
}

void EyesMacLayer::rxPacket(MacPacket* msg) {
	sendUp(msg->decapsulate());
	delete msg;
}

int EyesMacLayer::headerLength() {
	// override this!
	assert(0);
	return -1;
}

void EyesMacLayer::handleTimer(unsigned int index) {
	if (index < TIMERS)
		timeout(index);
	else
		internalTimeout();
}

void EyesMacLayer::handleUpperMsg(cPacket * msg) {
	MacPacket *packet = new MacPacket(this,"upper msg", TX);
	MacControlInfo *cinfo = dynamic_cast<MacControlInfo*>(msg->getControlInfo());
	packet->encapsulate(msg);
	packet->setLocalTo(cinfo->getNextHopMac());
	packet->setLocalFrom(macid());
	txPacket(packet);
}

void EyesMacLayer::handleLowerControl(cMessage * msg)
{
	assert(msg->getFullPath().find("TimerCore")==std::string::npos);
	switch(msg->getKind()) {
		case NicControlType::RX_HDR:
		{
			MacPacket *packet = dynamic_cast<MacPacket*>(msg);
			assert(packet);
			rxHeader(packet);
			delete packet;
			break;
		}
		case NicControlType::TX_END:
			txDone();
			delete msg;
			break;
		case NicControlType::RX_START:
			rxStart();
			delete msg;
			break;
		case NicControlType::RX_FAIL:
			//~ printf("RX_FAIL");
			rxFail();
			delete msg;
			break;
		case NicControlType::SET_RSSI:
			//assert_type(msg, RssiMessage *);
			rssi = ((RssiMessage *) msg)->rssi;
			if (lpl && rssi < 0.5 && recv_state == PREAMBLE_DETECT) {
				cancelInternalTimeout();
				internalTimeout();
			}
			delete msg;
			break;
		default:
			assert(false); // unknown
	}
}

void EyesMacLayer::handleLowerMsg(cMessage* msg)
{
	assert(msg);
	MacPacket *packet = (MacPacket *)(msg);

	assert(packet);
	if (packet->serial != -1)
		printf("MAPPER: RX, %d %s %d %d", macid(), SIMTIME_STR(simTime()), packet->serial, packet->local_from);
	rx(packet);
}

// default implementation
void EyesMacLayer::timeout() {
}

void EyesMacLayer::timeout(int which) {
	if(which == 0)
		timeout();
}

void EyesMacLayer::internalTimeout() {
	assert(lpl);
	force_sleep = false;
	switch (recv_state) {
		case SENSE:
			if (rssi < 0.5) {
				// nothing interesting detected
				setRadioSleepInternal();
				recv_state = SLEEP;
				EV << "recv state " << (int)recv_state << endl;
				setInternalTimeout(off_time);
			} else {
				// may have found a preamble
				printf("channel busy, starting preamble search");
				recv_state = PREAMBLE_DETECT;
				EV << "recv state " << (int)recv_state << endl;
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
			printf("didn't find start symbol, going back to sleep");
			// make sure we know we are not receiving anymore
			rxFailed();
			setRadioSleepInternal();
			recv_state = SLEEP;
			EV << "recv state " << (int)recv_state << endl;
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

void EyesMacLayer::setInternalTimeout(int ticks) {
	assert(lpl);
	assert(ticks > 0);
	simtime_t thenTime = getThenTime(ticks);
	assert(thenTime > simTime());
	setTimer(TIMERS, thenTime - simTime());
}

void EyesMacLayer::setRadioSleepInternal() {
	cMessage *msg = new cMessage("setradiosleep", NicControlType::SET_SLEEP);
	sendControlDown(msg);
}

void EyesMacLayer::setRadioListen() {
	cMessage *msg = new cMessage("setradiolisten", NicControlType::SET_LISTEN);
	sendControlDown(msg);
	// for low power listening: if we are not in the process of receiving
	// either a frame or a possible preamble, go to SENSE state
	if (lpl && recv_state != FRAME && recv_state != PREAMBLE_DETECT && !force_sleep) {
		cancelInternalTimeout();
		setInternalTimeout(on_time);
		recv_state = SENSE;
		EV << "recv state " << (int)recv_state << endl;
	}
}

void EyesMacLayer::setRadioTransmit() {
	cMessage *msg = new cMessage("setradiotransmit", NicControlType::SET_TRANSMIT);
	sendControlDown(msg);
	if (lpl) {
		cancelInternalTimeout();
		EV << "recv state " << (int)recv_state << endl;
		recv_state = SLEEP;
		force_sleep = false;
	}
}

void EyesMacLayer::setRadioSleep() {
	setRadioSleepInternal();
	if(lpl) {
		cancelInternalTimeout();
		EV << "recv state " << (int)recv_state << endl;
		recv_state = SLEEP;
		force_sleep = false;
	}
}

void EyesMacLayer::cancelInternalTimeout() {
	assert(lpl);
	cancelTimer(TIMERS);
}

void EyesMacLayer::txDone() {
	assert(send_state);
	send_state = false;
	transmitDone(); // relay to implementation
}

void EyesMacLayer::rxStart() {
	//TODO: should it also allow the SENSE state ?
	assert(recv_state == PREAMBLE_DETECT || recv_state == SENSE);
	EV << "recv state " << (int)recv_state << endl;
	EV << "starting rx"<<endl;
	recv_state = FRAME;
	if (lpl)
		cancelInternalTimeout();
	rxStarted(); // relay to implementation
}

void EyesMacLayer::rxFail() {
	if (lpl) {
		if (recv_state == FRAME) {
			EV << "recv state " << (int)recv_state << endl;
			recv_state = SLEEP;
			setRadioSleepInternal();
			setInternalTimeout(off_time);
			force_sleep = true;
		}
	} else
	{
		EV << "recv state " << (int)recv_state << endl;
		recv_state = SENSE;
	}
	rxFailed(); // relay to implementation
}

/**
 * received frame
 */
void EyesMacLayer::rx(MacPacket * msg) {
	assert(recv_state == FRAME);
	int hdrlen = headerLength(); // get from implementation
	assert(msg->getBitLength() >= hdrlen);
	msg->decreaseLength(hdrlen); // strip header
	if (lpl) {
		// the transmitter will be on for a little while longer
		recv_state = SLEEP;
		EV << "recv state " << (int)recv_state << endl;
		setRadioSleepInternal();
		setInternalTimeout(2);
		force_sleep = true;
	} else
	{
		recv_state = SENSE;
		EV << "recv state " << (int)recv_state << endl;
	}

	rxFrame(msg); // relay to implementation
}

void EyesMacLayer::reg_tx_overhead(MacPacket *p) {
	stat_time_tx_overhead += frameTotalTime(p->getBitLength() + headerLength());
}

int EyesMacLayer::siftSlot(int low, int high) {
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

string EyesMacLayer::getParameter(const char *parameter) {
	if (!hasPar(parameter))
		return string("");
	string result = par(parameter).stringValue();

	for (vector<ParameterPair>::iterator iter = parameterList.begin(); iter != parameterList.end(); iter++)
		if (strcmp(iter->first, parameter) == 0 && iter->second == result)
			return result;

	parameterList.push_back(ParameterPair(parameter, result));
	return result;
}

void EyesMacLayer::internal_printf(const char* fmt, va_list list, bool newline) {
	char *pbuf = NULL;
	//TODO: vasprintf is not defined within windows, fix this somehow
	//vasprintf(&pbuf, fmt, list);
	if (newline)
		EV << pbuf << endl;
	else
		EV << pbuf;
	free(pbuf);
}

void EyesMacLayer::printf_nr(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	internal_printf(fmt, va, false);
	va_end(va);
}

void EyesMacLayer::printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	internal_printf(fmt, va, true);
	va_end(va);
}

void EyesMacLayer::printf_clr(const char *fmt, ...) {
	va_list va;
	char *pbuf=NULL;
	va_start(va, fmt);
	//TODO: vasprintf is not defined within windows, fix this somehow
	//vasprintf(&pbuf, fmt, va);
	va_end(va);

	EV/*_clear*/ << pbuf;
	free(pbuf);
}

void EyesMacLayer::printfNoInfo(const char *fmt, ...) {
	va_list va;
	char *pbuf;

	va_start(va, fmt);
	//TODO: vasprintf is not defined within windows, fix this somehow
	//vasprintf(&pbuf, fmt, va);
	EV << pbuf << endl;
	va_end(va);
	free(pbuf);
}

void EyesMacLayer::printfNoInfo_nr(const char *fmt, ...) {
	va_list va;
	char *pbuf;

	va_start(va, fmt);
	//TODO: vasprintf is not defined within windows, fix this somehow
	//vasprintf(&pbuf, fmt, va);
	va_end(va);
	EV << pbuf;
	free(pbuf);
}

string EyesMacLayer::getStringParameter(const char *parameter, string defaultValue) {
	string result = getParameter(parameter);

	return result.empty() ? defaultValue : result;
}

simtime_t EyesMacLayer::getTimeParameter(const char *parameter, simtime_t defaultValue) {
	string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;

	return STR_SIMTIME(result.c_str());
}

double EyesMacLayer::getDoubleParameter(const char *parameter, double defaultValue) {
	string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;

	return strtod(result.c_str(), NULL);
}

long EyesMacLayer::getLongParameter(const char *parameter, long defaultValue) {
	string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;

	return strtol(result.c_str(), NULL, 0);
}

bool EyesMacLayer::getBoolParameter(const char *parameter, bool defaultValue) {
	string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;

	/* Shameless copy from omnet code. */
	const char *s = result.c_str();
	char b[16]; int i;
	for (i = 0; i < 15 && s[i] && s[i]!=' ' && s[i]!='\t'; i++)
		b[i]=s[i];
	b[i]=0;

	bool val;
	if (strcmp(b, "yes") == 0 || strcmp(b, "true") == 0 || strcmp(b, "on") == 0 || strcmp(b, "1") == 0) {
		val = 1;
	} else if (strcmp(b, "no") ==0 || strcmp(b, "false") == 0 || strcmp(b, "off") == 0 || strcmp(b, "0") == 0) {
		val = 0;
	} else {
		ev.printf("`%s' is not a valid bool value, use true/false, on/off, yes/no or 0/1\n", s);
		val = defaultValue;
	}

	return val;
}

unsigned short EyesMacLayer::getCurrentTime() {
	return (unsigned short)( simTime().raw() * 32768 /*+ clock_skew */);
}

