#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H 1

#include <omnetpp.h>
#include "FrameTimerGenerator.h"
#include "BaseModule.h"

/* frame timers are repeating timers (ala TimeSync in TinyOS)
 * Frame timers can also be assumed to be firing at the same time on 
 * different nodes (i.e. global time is being used)
 * Default implementation of this cheats and uses simTime() to calculate
 * the global time points, but eventually this could be implemented using 
 * the standard timers and a proper global time implementation */

class FrameTimer
{
	friend class FrameTimerGenerator;
	protected:
		FrameTimerGenerator *tg;
		BaseModule *owner;
	public:	
		virtual void init(BaseModule *parent);
		void initFrameTimers(unsigned int count){tg->initFrameTimers(count);}
		void setFrameTimer(unsigned int index, double period) {tg->setFrameTimer(index,period);}
		void cancelFrameTimer(unsigned int index){tg->cancelFrameTimer(index);}
		virtual void handleFrameTimer(unsigned int index)=0;
};

#endif
