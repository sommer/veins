#include "FrameTimerGenerator.h"
#include "FrameTimer.h"

#include <assert.h>

Define_Module_Like(FrameTimerGenerator,Trivial);

void FrameTimerGenerator::init(FrameTimer *parent)
{
	Enter_Method_Silent();
	char * lookupname;
	ft = parent;
	if (parent->owner->hasPar("GlobalTime"))
		lookupname = strdup(parent->owner->par("GlobalTime"));
	else
		lookupname = strdup("GlobalTime");
	gt = dynamic_cast<GlobalTime*>(parent->owner->getNode()->submodule(lookupname));
	if (gt == NULL)
		gt = dynamic_cast<GlobalTime*>(findModuleType(lookupname)->createScheduleInit(lookupname,parent->owner->getNode()));
	free(lookupname);	
	timer_count=0;
	ev << "Frame timer path is "<<fullPath()<<endl;
}

void FrameTimerGenerator::initFrameTimers(unsigned int count)
{
	unsigned int i;
	Enter_Method_Silent();
	if (timer_count > 0)
	{
		delete[] timers;
		delete[] frames;
	}

	timer_count = count;
	timers = new cMessage[timer_count];
	frames = new double[timer_count];
	for (i = 0; i < count; i++)
	{
		char buffer[255];
		sprintf(buffer,"frame timer %d",i);
		timers[i].setKind(i);
		timers[i].setName(buffer);
		frames[i] = 0;
	}
}

void FrameTimerGenerator::nextFrame(unsigned int index)
{
	simtime_t now = gt->currentGlobalTime();
	if (frames[index] == 0)
		return;
	double count = floor(now/frames[index]);
	assert(index < timer_count);
	assert(frames[index]>0);
	ev <<"scheduling at "<<count<<"," <<((count+1)*frames[index])<<endl;
	scheduleAt((count+1)*frames[index],&timers[index]);
}

void FrameTimerGenerator::setFrameTimer(unsigned int index, double period)
{
	Enter_Method_Silent();
	cancelFrameTimer(index);
	assert(index < timer_count);
	frames[index] = period;
	assert(frames[index]>0);
	nextFrame(index);
}

void FrameTimerGenerator::cancelFrameTimer(unsigned int index)
{
	assert(index < timer_count);
	if (timers[index].isScheduled())
		cancelEvent(&timers[index]);
	frames[index] = 0;
}

void FrameTimerGenerator::handleMessage(cMessage* msg)
{
	ft->handleFrameTimer(msg->kind());
	nextFrame(msg->kind());
}

FrameTimerGenerator::~FrameTimerGenerator()
{
	for (unsigned int i=0;i<timer_count;i++)
		cancelFrameTimer(i);
	delete [] timers;
	delete [] frames;
}

