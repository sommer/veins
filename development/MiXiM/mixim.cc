#include "mixim.h"

bool MiximBaseModule::globalsInitialised = false;
int MiximBaseModule::prints =
	  PRINT_CRIT
	| PRINT_STATS
//	| PRINT_PROP
//	| PRINT_MAC
//	| PRINT_RADIO
	| PRINT_ROUTING
	| PRINT_APP
	| PRINT_INIT
	;

void MiximBaseModule::internal_printf(const char* fmt, va_list list, bool newline) {
	char pbuf[1000];
	vsnprintf(pbuf, 1000, fmt, list);
	ev.printf("%20s %s: %s%c", simtimeToStr(simTime()),
#if (OMNETPP_VERSION>=0x0300)
		fullPath().c_str()
#else
		fullPath()
#endif
		, pbuf, 
		newline? '\n' : ' '
	);
}

void MiximBaseModule::printf_nr(int which, const char *fmt, ...) {
	if(which & prints) {
		va_list va;
		va_start(va, fmt);
		internal_printf(fmt, va, false);
		va_end(va);
	}
}

void MiximBaseModule::printf(int which, const char *fmt, ...) {
	if(which & prints) {
		va_list va;
		va_start(va, fmt);
		internal_printf(fmt, va, true);
		va_end(va);
	}
}

void MiximBaseModule::printf_clr(int which, const char *fmt, ...) {
	if(which & prints) {
		va_list va;
		char pbuf[1000];
		va_start(va, fmt);
		vsnprintf(pbuf, 1000, fmt, va);
		va_end(va);

		ev << pbuf;
	}
}

void MiximBaseModule::printfNoInfo(int which, const char *fmt, ...) {
	if(which & prints) {
		va_list va;
		char pbuf[1000];

		va_start(va, fmt);
		vsnprintf(pbuf, 1000, fmt, va);
		ev << pbuf << endl;
		va_end(va);
	}
}

void MiximBaseModule::printfNoInfo_nr(int which, const char *fmt, ...) {
	if(which & prints) {
		va_list va;
		char pbuf[1000];

		va_start(va, fmt);
		vsnprintf(pbuf, 1000, fmt, va);
		va_end(va);
		ev << pbuf;
	}
}

double MiximBaseModule::absDistance(Position one, Position two) {
	// gcc 3.3.2/Fedora needs first arg to be double to choose from overloaded pow functions
	return sqrt(pow((double)one.x - two.x, 2) + pow((double)one.y - two.y, 2) + pow((double)one.z - two.z, 2));
}

void MiximBaseModule::powerConsumed(double joule) {
	power_consumed += joule;
}

void MiximBaseModule::initialize() {
	cSimpleModule::initialize();
	power_consumed = 0.0;
	timer_count = 0;
	if (!globalsInitialised) {
		string result = ev.getParameter(simulation.runNumber(), "mixim.prints");

		/* FIXME: should allow for string value like "stats, crit, radio, mac",
		   probably with strtok. */
		if (!result.empty())
			prints = strtol(result.c_str(), NULL, 0);
		
		if (ev.numRNGs() < RNG_MAX)
			error("Number of random number generators set too low. Add \"num-rngs=%d\" to the [General] section of your ini file\n", RNG_MAX);
	}
}

void MiximBaseModule::finish() {
	cSimpleModule::finish();
	printf(PRINT_STATS, "consumed %f joule", power_consumed);
}

MiximBaseModule::~MiximBaseModule() {
	globalsInitialised = false;
	if (timer_count > 0)
	{
		unsigned int i;
		for (i = 0; i < timer_count; i++)
			cancelTimer(i);
		delete[] timers;
		timer_count = 0;
	}
}

void MiximBaseModule::initTimers(unsigned int count)
{	
	unsigned int  i;
	if (timer_count > 0)
		delete[] timers;

	timer_count = count;
	timers = new cMessage[timer_count];
	for (i = 0; i < count; i++)
		timers[i].setKind(i);
}

void MiximBaseModule::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage()) {
		handleTimer(msg->kind());
	} else {
		handleEvent(msg);
	}
}

void MiximBaseModule::setTimer(unsigned int index, double when)
{
	if (timer_count <= index)
		error("setTimer: timer index %u out of range", index);
	if (timers[index].isScheduled())
		cancelEvent(&timers[index]);

	scheduleAt(simTime() + when, &timers[index]);	
}

void MiximBaseModule::cancelTimer(unsigned int index)
{
	if (timer_count <= index)
		error("cancelTimer: timer index %u out of range", index);
	if (timers[index].isScheduled())
		cancelEvent(&timers[index]);
}

void MiximBaseModule::handleTimer(unsigned int count) {assert(0);}
void MiximBaseModule::handleEvent(cMessage *msg) {assert(0);}

std::string MiximBaseModule::getParameter(const char *parameter) {
	std::string param = "mixim.";

	param.append(className());
	param.append(".");
	param.append(parameter);
	return ev.getParameter(simulation.runNumber(), param.c_str());
}

std::string MiximBaseModule::getStringParameter(const char *parameter, std::string defaultValue) {
	std::string result = getParameter(parameter);

	return result.empty() ? defaultValue : result;
}

double MiximBaseModule::getTimeParameter(const char *parameter, double defaultValue) {
	std::string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;
	
	return strToSimtime(result.c_str());
}

double MiximBaseModule::getDoubleParameter(const char *parameter, double defaultValue) {
	std::string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;
	
	return strtod(result.c_str(), NULL);
}

long MiximBaseModule::getLongParameter(const char *parameter, long defaultValue) {
	std::string result = getParameter(parameter);

	if (result.empty())
		return defaultValue;
	
	return strtol(result.c_str(), NULL, 0);
}

bool MiximBaseModule::getBoolParameter(const char *parameter, bool defaultValue) {
	std::string result = getParameter(parameter);

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

