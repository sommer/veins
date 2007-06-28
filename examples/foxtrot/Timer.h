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
		~Timer(){delete ct;}
		virtual void init(cModule *parent);

	/** Set a timer to fire at a point in the future.
		If the timer with that id has already been set then this discards the old information.
		@param index Timer number to set.
		@param when Time in seconds in the future to fire the timer
	 */
	void setTimer(unsigned int index, double when){ct->setTimer(index,when);}

	/** Set a timer to fire at a point in the future.
	    Auto-generates a timer id that's guaranteed not to have been used by anyone else.
		@param when Time in seconds in the future to fire the timer
		@return Timer id
	 */
	unsigned int setTimer(double when){return ct->setTimer(when);}

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
