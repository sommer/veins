#include "TimerCore.h"
#include "Timer.h"

#include <assert.h>

//Define_Module_Like(TimerCore,Trivial);

void TimerCore::checkExists(unsigned int index)
{
	if (timers->find(index)==timers->end())
		error(" timer index %u doesn't exist", index);
}

void TimerCore::handleMessage(cMessage* msg)
{
	assert(msg->isSelfMessage());
	//simulation.setContextModule(timer->owner);	
	timer->handleTimer(msg->getKind());
}

void TimerCore::init(Timer *owner)
{
	timer = owner;
	timers = new std::map<unsigned int,cMessage *>();
	destructors = new std::map<unsigned int,cleanup *>();
}

unsigned int TimerCore::setTimer(simtime_t_cref when)
{
	unsigned int key = timers->size();
	while (timers->find(key)!=timers->end())
		key++;
	setTimer(key,when);
	return key;
}

void TimerCore::setTimer(unsigned int index, simtime_t_cref when)
{
	Enter_Method_Silent();
	cMessage *timer;
	if (timers->find(index)==timers->end())
	{
		timer = new cMessage("timer");
		timer->setKind(index);
		(*timers)[index] = timer;
		(*destructors)[index] = NULL;
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
	checkExists(index);
	if ((*timers)[index]->isScheduled())
		cancelEvent((*timers)[index]);
}

float TimerCore::remainingTimer(unsigned int index)
{
	checkExists(index);
	if ((*timers)[index]->isScheduled())
		return SIMTIME_DBL((*timers)[index]->getArrivalTime()-simTime());
	else
		return -1;
}

void TimerCore::setContextDestructor(unsigned int index, cleanup *c)
{
	checkExists(index);
	(*destructors)[index] = c;
}

TimerCore::~TimerCore()
{
	for (std::map<unsigned int,cMessage*>::const_iterator p=timers->begin();p!=timers->end();p++)
	{
		unsigned int index = p->second->getKind();
		checkExists(index);
		if ((*timers)[index]->isScheduled())
		{
			if ((*destructors)[index]!=NULL)
				(*destructors)[index](contextPointer(index));
			cancelEvent((*timers)[index]);
		}
		delete p->second;
	}
	delete timers;
	delete destructors;
}

/** Set a "context pointer" refering to some piece of opaque useful data
 * @param index Timer number
 * @param data Opaque pointer. Never free'd or dereferenced
 */
void TimerCore::setContextPointer(unsigned int index,void * data)
{
	checkExists(index);
	(*timers)[index]->setContextPointer(data);
}

/** Retreive a "context pointer" refering to some piece of opaque useful data
 * @param index Timer number
 * @return Opaque pointer from @setContextPointer
 */
void * TimerCore::contextPointer(unsigned int index)
{
	checkExists(index);
	return (*timers)[index]->getContextPointer();
}

/* Mark the first @count pointer ids (from 0 to @count-1) as allocated, so they don't get
 * auto-allocated by setTimer
 * @param count Number of timers to allocate
 */
void TimerCore::allocateTimers(unsigned int count)
{
	Enter_Method_Silent();
	for (unsigned int i=0;i<count;i++)
	{
		cMessage *timer;
		if (timers->find(i)==timers->end())
		{
			timer = new cMessage("timer");
			timer->setKind(i);
			(*timers)[i] = timer;
		}
	}
}

/* Delete a timer. Useful for auto-allocated timers that you don't need any more to 
 * reduce memory usage. Does nothing if the timer doesn't exist
 * @param index Timer to wipe
 */
void TimerCore::deleteTimer(unsigned int index)
{
	if (timers->find(index)!=timers->end())
	{
		cancelTimer(index);
		delete timers->find(index)->second;
		timers->erase(timers->find(index));
	}
}

