#include "veins/modules/utility/SimpleTimerModule.h"

using omnetpp::cMessage;
using omnetpp::simtime_t;
using omnetpp::simTime;
using Veins::SimpleTimerModule;

SimpleTimerModule::~SimpleTimerModule() {
	for (const auto &timer : timers_) {
		cancelAndDelete(timer.first);
	}
}

void SimpleTimerModule::handleMessage(cMessage *message) {
	auto *timer_message = dynamic_cast<SimpleTimerModule::TimerMessage*>(message);
	if (!message->isSelfMessage() || !timer_message) {
		EV_DEBUG << "Received non-timer-message or message from different module";
		return;
	}

	auto timer = timers_.find(timer_message);
	ASSERT(timer != timers_.end() && timer->second.validOccurence(simTime()));

	timer->second.callback();

	const auto next_event = simTime() + timer->second.period;
	if (timer->second.validOccurence(next_event)) {
		scheduleAt(next_event, timer->first);
	} else {
		cancelTimer(timer);
	}
}

const SimpleTimerModule::Timer SimpleTimerModule::addOneshotTimer(const std::string &name, const std::function<void()> callback, const omnetpp::simtime_t time, const TimeInterpretation time_interpretation) {
	return addRepeatingTimer(name, callback, 1, 1, time, time_interpretation);
}

const SimpleTimerModule::Timer SimpleTimerModule::addRepeatingTimer(const std::string &name, const std::function<void()> callback, const omnetpp::simtime_t period, const unsigned count, omnetpp::simtime_t start, const TimeInterpretation time_interpretation) {
	ASSERT(count > 0);
	if (start < 0) {
		start = simTime() + period;
	} else if (time_interpretation == TimeInterpretation::RELATIVE_TIME) {
		start += simTime();
	}
	const auto end = start + (count - 1) * period;
	return addRepeatingTimer(name, callback, period, start, end, TimeInterpretation::ABSOLUTE_TIME);
}

const SimpleTimerModule::Timer SimpleTimerModule::addRepeatingTimer(const std::string &name, const std::function<void()> callback, const omnetpp::simtime_t period, omnetpp::simtime_t start, omnetpp::simtime_t end, const TimeInterpretation time_interpretation) {
	if (start < 0) {
		start = simTime() + period;
	} else if (time_interpretation == TimeInterpretation::RELATIVE_TIME) {
		start += simTime();
	}
	if (end >= 0 && time_interpretation == TimeInterpretation::RELATIVE_TIME) {
		end += simTime();
	}

	auto ret = timers_.insert({new SimpleTimerModule::TimerMessage(name), {start, end, period, callback}});
	ASSERT(ret.second);
	scheduleAt(start, ret.first->first);

	return ret.first;
}

void SimpleTimerModule::cancelTimer(Timer timer) {
	cancelEvent(timer->first);
	timers_.erase(timer);
}
