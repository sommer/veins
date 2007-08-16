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
	    Timer(){ct = NULL;owner=NULL;}
		virtual ~Timer(){delete ct;}
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

	/** Set a "context pointer" refering to some piece of opaque useful data
	 * @param index Timer number
	 * @param data Opaque pointer. Never free'd or dereferenced
	 */
	void setContextPointer(unsigned int index,void * data) {ct->setContextPointer(index,data);}

	/** Retreive a "context pointer" refering to some piece of opaque useful data
	 * @param index Timer number
	 * @return Opaque pointer from @setContextPointer
	 */
	void * contextPointer(unsigned int index) {return ct->contextPointer(index);}

	/** Provide a destructor function for a "context pointer" such that we can
	 *  do complete cleanup even if there are still timers remaining at the end of a
	 *  simulation. Called on end of sim for still scheduled timers.
	 *  @param index Timer number
	 */
	void setContextDestructor(unsigned int index, void (*func)(void * data)){ct->setContextDestructor(index,func);}

	/* Mark the first @count pointer ids (from 0 to @count-1) as allocated, so they don't get
	 * auto-allocated by setTimer
	 * @param count Number of timers to allocate
	 */
	void allocateTimers(unsigned int count) {ct->allocateTimers(count);}

	/* Delete a timer. Useful for auto-allocated timers that you don't need any more to 
	 * reduce memory usage. Does nothing if the timer doesn't exist
	 * @param index Timer to wipe
	 */
	void deleteTimer(unsigned int index) {ct->deleteTimer(index);}

};

#endif
