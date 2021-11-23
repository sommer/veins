//
// Copyright (C) 2018-2018 Max Schettler <max.schettler@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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
#include "veins/modules/utility/TimerManager.h"

#include <algorithm>

using omnetpp::simTime;
using omnetpp::simtime_t;
using veins::TimerManager;
using veins::TimerMessage;
using veins::TimerSpecification;

struct veins::TimerMessage : public omnetpp::cMessage {
    TimerMessage(const std::string& name)
        : omnetpp::cMessage(name.c_str())
    {
    }
};

TimerSpecification::TimerSpecification(std::function<void()> callback)
    : start_mode_(StartMode::immediate)
    , end_mode_(EndMode::open)
    , callback_([callback](TimerManager::TimerHandle) { callback(); })
{
}

TimerSpecification::TimerSpecification(std::function<void(TimerManager::TimerHandle)> callback)
    : start_mode_(StartMode::immediate)
    , end_mode_(EndMode::open)
    , callback_(callback)
{
}

TimerSpecification& TimerSpecification::interval(simtime_t interval)
{
    ASSERT(interval > 0);
    period_generator_ = [interval]() { return interval; };
    return *this;
}

TimerSpecification& TimerSpecification::interval(std::function<omnetpp::simtime_t()> generator)
{
    period_generator_ = generator;
    return *this;
}

TimerSpecification& TimerSpecification::relativeStart(simtime_t start)
{
    start_mode_ = StartMode::relative;
    start_ = start;
    return *this;
}

TimerSpecification& TimerSpecification::absoluteStart(simtime_t start)
{
    start_mode_ = StartMode::absolute;
    start_ = start;
    return *this;
}

TimerSpecification& TimerSpecification::relativeEnd(simtime_t end)
{
    end_mode_ = EndMode::relative;
    end_time_ = end;
    return *this;
}

TimerSpecification& TimerSpecification::absoluteEnd(simtime_t end)
{
    end_mode_ = EndMode::absolute;
    end_time_ = end;
    return *this;
}

TimerSpecification& TimerSpecification::repetitions(size_t n)
{
    end_mode_ = EndMode::repetition;
    end_count_ = n;
    return *this;
}

TimerSpecification& TimerSpecification::openEnd()
{
    end_mode_ = EndMode::open;
    return *this;
}

TimerSpecification& TimerSpecification::oneshotIn(omnetpp::simtime_t in)
{
    return this->relativeStart(in).interval(1).repetitions(1);
}

TimerSpecification& TimerSpecification::oneshotAt(omnetpp::simtime_t at)
{
    return this->absoluteStart(at).interval(1).repetitions(1);
}

void TimerSpecification::finalize()
{
    switch (start_mode_) {
    case StartMode::relative:
        start_ += simTime();
        start_mode_ = StartMode::absolute;
        break;
    case StartMode::absolute:
        break;
    case StartMode::immediate:
        start_ = simTime() + period_generator_();
        break;
    }

    if (end_mode_ == EndMode::relative) {
        end_time_ += simTime();
        end_mode_ = EndMode::absolute;
    }
}

bool TimerSpecification::validOccurence(simtime_t time) const
{
    const bool afterStart = time >= start_;
    const bool beforeEnd = time <= end_time_;
    return afterStart && (beforeEnd || end_mode_ == EndMode::open || end_mode_ == EndMode::repetition);
}

omnetpp::simtime_t TimerSpecification::next()
{
    const auto next_relative = period_generator_();
    if (next_relative < 0) {
        return -1;
    }
    if (end_mode_ == EndMode::repetition) {
        end_count_ -= 1;
        if (end_count_ == 0) {
            return -1;
        }
    }
    const auto next_absolute = simTime() + next_relative;
    if ((end_mode_ != EndMode::open && end_mode_ != EndMode::repetition) && next_absolute > end_time_) {
        return -1;
    }
    return next_absolute;
}

TimerManager::TimerManager(omnetpp::cSimpleModule* parent)
    : parent_(parent)
{
    ASSERT(parent_);
}

TimerManager::~TimerManager()
{
    for (const auto& timer : timers_) {
        parent_->cancelAndDelete(timer.first);
    }
}

bool TimerManager::handleMessage(omnetpp::cMessage* message)
{
    auto* timerMessage = dynamic_cast<TimerMessage*>(message);
    if (!timerMessage) {
        return false;
    }
    ASSERT(timerMessage->isSelfMessage());
    std::string s = timerMessage->getName();

    auto timer = timers_.find(timerMessage);
    if (timer == timers_.end()) {
        return false;
    }
    ASSERT(timer->second.valid() && timer->second.validOccurence(simTime()));

    timer->second.callback_(timer->first->getId());

    if (timers_.find(timerMessage) != timers_.end()) { // confirm that the timer has not been cancelled during the callback
        const auto nextEvent = timer->second.next();
        if (nextEvent < 0) {
            parent_->cancelAndDelete(timer->first);
            timers_.erase(timer);
        }
        else {
            parent_->scheduleAt(nextEvent, timer->first);
        }
    }

    return true;
}

TimerManager::TimerHandle TimerManager::create(TimerSpecification timerSpecification, const std::string name)
{
    ASSERT(timerSpecification.valid());
    timerSpecification.finalize();

    const auto ret = timers_.insert(std::make_pair(new TimerMessage(name), std::move(timerSpecification)));
    ASSERT(ret.second);
    parent_->scheduleAt(ret.first->second.start_, ret.first->first);

    return ret.first->first->getId();
}

void TimerManager::cancel(TimerManager::TimerHandle handle)
{
    const auto entryMatchesHandle = [handle](const std::pair<TimerMessage*, TimerSpecification>& entry) { return entry.first->getId() == handle; };
    auto timer = std::find_if(timers_.begin(), timers_.end(), entryMatchesHandle);
    if (timer != timers_.end()) {
        parent_->cancelAndDelete(timer->first);
        timers_.erase(timer);
    }
}
