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
	timers = new std::map<unsigned int,cMessage *>();
}

unsigned int TimerCore::setTimer(double when)
{
	unsigned int key = timers->size();
	while (timers->find(key)!=timers->end())
		key++;
	setTimer(key,when);
	return key;
}

void TimerCore::setTimer(unsigned int index, double when)
{
	Enter_Method_Silent();
	cMessage *timer;
	if (timers->find(index)==timers->end())
	{
		timer = new cMessage("timer");
		timer->setKind(index);
		(*timers)[index] = timer;
	}
	else
	{
		timer = (*timers)[index];
		if (timer->isScheduled())
			cancelEvent(timer);
	}

	scheduleAt(simTime() + when, timer);	
}

void TimerCore::cancelTimer(unsigned int index)
{
	Enter_Method_Silent();
	if (timers->find(index)==timers->end())
		error("cancelTimer: timer index %u doesn't exist", index);
	if ((*timers)[index]->isScheduled())
		cancelEvent((*timers)[index]);
}

float TimerCore::remainingTimer(unsigned int index)
{
	if (timers->find(index)==timers->end())
		error("remainingTimer: timer index %u doesn't exist", index);
	if ((*timers)[index]->isScheduled())
		return (*timers)[index]->arrivalTime()-simTime();
	else
		return -1;
}

TimerCore::~TimerCore()
{
	for (std::map<unsigned int,cMessage*>::const_iterator p=timers->begin();p!=timers->end();p++)
	{
		cancelTimer(p->second->kind());
		delete p->second;
	}
	delete timers;	
}

