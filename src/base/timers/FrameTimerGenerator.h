#ifndef FRAME_TIMER_GENERATOR_H
#define FRAME_TIMER_GENERATOR_H 1

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "GlobalTime.h"

class FrameTimer;

/* frame timers are repeating timers (ala TimeSync in TinyOS)
 * Frame timers can also be assumed to be firing at the same time on 
 * different nodes (i.e. global time is being used)
 * Default implementation of this cheats and uses simTime() to calculate
 * the global time points, but eventually this could be implemented using 
 * the standard timers and a proper global time implementation */

class MIXIM_API FrameTimerGenerator: public cSimpleModule
{
	protected:
		FrameTimer *ft;
		GlobalTime *gt;
		std::map<unsigned int,cMessage *> *timers;
		std::map<unsigned int,double> *frames;
		void nextFrame(unsigned int index);
		virtual void handleMessage(cMessage* msg);
	public:	
	    //Module_Class_Members(FrameTimerGenerator, cSimpleModule, 0);
		~FrameTimerGenerator();
		virtual void init(FrameTimer *parent);
		void setFrameTimer(unsigned int index, double period);
		unsigned int setFrameTimer(double period);
		void cancelFrameTimer(unsigned int index);
};

#endif
