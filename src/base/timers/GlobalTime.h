#ifndef GLOBAL_TIME_H
#define GLOBAL_TIME_H 1

#include <omnetpp.h>

#include "MiXiMDefs.h"

/*
 * GlobalTime provides information about the local opinion of the global time value
 * Default implementation cheats and just uses simTime, but subclasses should do
 * proper network-wide synchronisation
 */

class MIXIM_API GlobalTime: public cSimpleModule
{
	public:	
	    //Module_Class_Members(GlobalTime, cSimpleModule, 0);
		virtual const simtime_t currentGlobalTime() const {return simTime();}
};
#endif
