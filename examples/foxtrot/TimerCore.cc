#include "TimerCore.h"
#include "Timer.h"

#include <assert.h>

Define_Module_Like(TimerCore,Trivial);

void TimerCore::handleMessage(cMessage* msg)
{
	assert(msg->isSelfMessage());
	simulation.setContextModule(timer->owner);	
	timer->handleTimer(msg->kind());
}

void TimerCore::init(Timer *owner)
{
	timer = owner;
	timer_count = 0;
}

void TimerCore::initTimers(unsigned int count)
{	
	unsigned int i;
	Enter_Method_Silent();
	if (timer_count > 0)
		delete[] timers;

	timer_count = count;
	timers = new cMessage[timer_count];
	for (i = 0; i < count; i++)
		timers[i].setKind(i);
}

void TimerCore::setTimer(unsigned int index, double when)
{
	Enter_Method_Silent();
	if (timer_count <= index)
		error("setTimer: timer index %u out of range", index);
	if (timers[index].isScheduled())
		cancelEvent(&timers[index]);

	scheduleAt(simTime() + when, &timers[index]);	
}

void TimerCore::cancelTimer(unsigned int index)
{
	Enter_Method_Silent();
	if (timer_count <= index)
		error("cancelTimer: timer index %u out of range", index);
	if (timers[index].isScheduled())
		cancelEvent(&timers[index]);
}

float TimerCore::remainingTimer(unsigned int index)
{
	//Enter_Method_Silent();
	if (timer_count <= index)
		error("remainingTimer: timer index %u out of range", index);
	if (timers[index].isScheduled())
		return timers[index].arrivalTime()-simTime();
	else
		return -1;
}

TimerCore::~TimerCore()
{
	for (unsigned int i=0;i<timer_count;i++)
		cancelTimer(i);
	delete [] timers;	
}

