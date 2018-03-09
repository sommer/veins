//
// Copyright (C) 2018-2018 Max Schettler <max.schettler@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <omnetpp.h>

namespace Veins {

/**
 * A module that supports a clean interface for scheduling timers.
 *
 * @note This class overrides cSimpleModule::handleMessage such that you can not use cSimpleModule::activity if you subclass SimpleTimerModule.
 */
class SimpleTimerModule : public omnetpp::cSimpleModule
{
	public:
		/**
		 * Deletes this module.
		 */
		virtual ~SimpleTimerModule() override;

		/**
		 * Handle messages arriving at this class.
		 * 
		 * Timers be handled by calling their callbacks and enqueuing their next event, if applicable.
		 */
		virtual void handleMessage(omnetpp::cMessage *message) override;

	protected:
		/**
		 * A message which is used for triggering Timers.
		 *
		 * Its implementation is empty as it is only used to differentiate from other messages.
		 */
		struct TimerMessage : public omnetpp::cMessage {
			TimerMessage(const std::string &name) : omnetpp::cMessage(name.c_str()) {}
		};

		/**
		 * A class which specifies a Timer.
		 *
		 * This includes timing information as well as its callback.
		 */
		struct TimerSpec {
			/**
			 * Test whether the given time is a valid occurence.
			 */
			bool validOccurence(const omnetpp::simtime_t &time) const {
				return start <= time && (end < 0 || end >= time);
			}

			omnetpp::simtime_t start; ///< Time of the Timer's first occurence.
			omnetpp::simtime_t end; ///< Last possible time a Timer will trigger. Negative values encode never-ending Timers.
			omnetpp::simtime_t period; ///< Time between events.
			std::function<void()> callback; ///< The function to be called when the Timer is triggered.
		};

		typedef std::map<TimerMessage*, const TimerSpec> TimerList;
		typedef TimerList::iterator Timer;

		/**
		 * Possible interpretations of a time value.
		 */
		enum class TimeInterpretation {
			RELATIVE_TIME, ///< Time will be interpreted relative to the current one.
			ABSOLUTE_TIME ///< Time describes a fixed point in time, independent of the current clock.
		};

		/**
		 * Creates a new one-shot Timer.
		 *
		 * The given callback will be executed exactly once.
		 * By default, the time is relative to the current time, i.e. a call with time=7 will be triggered in seven simulation seconds.
		 * When passing time_scope=ABSOLUTE_TIME, the given time is interpreted as an absolute value and a call with time=7 will create a timer which triggers the given callback when simTime() equals seven seconds.
		 *
		 * @param name The Timer's name. Used for its message.
		 * @param callback Function which is called when the Timer triggers.
		 * @param time Time at which the Timer triggers.
		 * @param time_interpretation Flag to indicate the semantics of the given time, see TimeScope.
		 */
		const Timer addOneshotTimer(const std::string &name, std::function<void()> callback, omnetpp::simtime_t time, TimeInterpretation time_interpretation=TimeInterpretation::RELATIVE_TIME);

		/**
		 * Creates a Timer which will be triggered n-times.
		 *
		 * @param name The Timer's name. Used for its message.
		 * @param callback Function which is called when the Timer triggers.
		 * @param period Interval between two Timer occurences.
		 * @param count Number of times the Timer will trigger.
		 * @param start Time at which the Timer triggers first. Negative values will set start to the current simulation time plus one period.
		 * @param time_interpretation Flag to indicate the semantics of the given time, see TimeScope.
		 */
		const Timer addRepeatingTimer(const std::string &name, std::function<void()> callback, omnetpp::simtime_t period, unsigned count, omnetpp::simtime_t start=-1, TimeInterpretation time_interpretation=TimeInterpretation::RELATIVE_TIME);

		/**
		 * Creates a Timer which will be triggered repeatedly in a specified interval.
		 *
		 * @param name The Timer's name. Used for its message.
		 * @param callback Function which is called when the Timer triggers.
		 * @param period Interval between two Timer occurences.
		 * @param start Time at which the Timer triggers first. Negative values will set start to the current simulation time plus one period.
		 * @param end Time at which the Timer triggers latest. Negative values indicate open-ended Timers.
		 * @param time_interpretation Flag to indicate the semantics of the given time, see TimeScope.
		 */
		const Timer addRepeatingTimer(const std::string &name, std::function<void()> callback, omnetpp::simtime_t period, omnetpp::simtime_t start=-1, omnetpp::simtime_t end=-1, TimeInterpretation time_interpretation=TimeInterpretation::RELATIVE_TIME);

		/**
		 * Cancels the Timer which is specified by the given TimerMessage and prevents any further executions.
		 *
		 * @param timer_message The Timer which will be canceled.
		 */
		void cancelTimer(Timer timer);

	private:
		TimerList timers_; ///< A list of all Timers which are currently active.
};

} // namespace Veins
