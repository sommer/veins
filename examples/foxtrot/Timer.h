#ifndef TIMER_H
#define TIMER_H 1

#include <omnetpp.h>

#include "TimerCore.h"

class Timer
{
	friend class TimerCore;
	protected:
		TimerCore *ct;
		cModule *owner;

	public:
		virtual void init(cModule *parent);

	/** Initialise a set of timers for this protocol layer
		@param count Number of timers used by this layer
	 */	
	void initTimers(unsigned int count){ct->initTimers(count);}

	/** Set one of the timers to fire at a point in the future.
		If the timer has already been set then this discards the old information.
		Must call @b initTimers() before using.
		@param index Timer number to set. Must be between 0 and the value given to @b initTimers()
		@param when Time in seconds in the future to fire the timer
	 */
	void setTimer(unsigned int index, double when){ct->setTimer(index,when);}

	/** Cancel an existing timer set by @b setTimer()
		If the timer has not been set, or has already fires, this does nothing
		Must call @b initTimers() before using.
		@param index Timer to cancel. Must be between 0 and the value given to @b initTimers()
	 */
	void cancelTimer(unsigned int index){ct->cancelTimer(index);}

	/** Fires on expiration of a timer.
		Fires after a call to @b setTimer(). Subclasses should override this.
		@param index Timer number that fired. Will be between 0 and the value given to @b initTimers()
	*/	

	float remainingTimer(unsigned int index) {return ct->remainingTimer(index);}

	virtual void handleTimer(unsigned int count)=0;
};

#endif
