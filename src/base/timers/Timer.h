#ifndef TIMER_H
#define TIMER_H 1

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "TimerCore.h"

/* Usage:
 * Your class needs to be a subclass of Timer (as well as BaseWhatever),
 * call Timer::init(self) before doing anything, and override handleTimer()
 * to do whatever things need doing when timers fire.
 *
 * The Timer module will also auto-cleanup all leftover timers at the end of 
 * simulation, but deleteTimer can be used to reclaim memory.
 *
 * Example (for a localisation module extending BaseLocalisation):
 *
 * class MyClass: public BaseLocalisation, public Timer
 * {
 *     public:
 *	    //Module_Class_Members(MyClass, BaseLocalisation, 0)
 *     	void initialize();
 *     	void handleTimer(unsigned int index);
 * }
 *
 * void MyClass::initialize()
 * {
 * 	    Timer::init(self);
 * 	    setTimer(1.0);
 * }
 *
 * void MyClass::handleTimer(unsigned int index)
 * {
 * 		// Do timer stuff, which will happen 
 *      // 1.0s after startup (see the setTimer above)
 * }
 */

class MIXIM_API Timer
{
	friend class TimerCore;
	protected:
		TimerCore *ct;
		cModule *owner;

	public:
	    Timer(){ct = NULL;owner=NULL;}
		virtual ~Timer(){delete ct;}
	
	/** Modules using Timer should call init() with their "this" pointer, as otherwise
	    other Timer functions will fail.
		@param parent Timer-using module's "this" pointer
	 */
	virtual void init(cModule *parent);

	/** Set a timer to fire at a point in the future.
		If the timer with that id has already been set then this discards the old information.
		@param index Timer number to set.
		@param when Time in seconds in the future to fire the timer
	 */
	void setTimer(unsigned int index, simtime_t_cref when){checkCT();ct->setTimer(index,when);}

	/** Set a timer to fire at a point in the future.
	    Auto-generates a timer id. This will not be an id allocated to any other existing timer
		(either running or finished) but may be an id from a timer deleted by deleteTimer.
		@param when Time in seconds in the future to fire the timer
		@return Timer id
	 */
	unsigned int setTimer(simtime_t_cref when){checkCT();return ct->setTimer(when);}

	/** Cancel an existing timer set by @b setTimer()
		If the timer has not been set, or has already fires, this does nothing
		Must call @b initTimers() before using.
		@param index Timer to cancel. Must be between 0 and the value given to @b initTimers()
	 */
	void cancelTimer(unsigned int index){checkCT();ct->cancelTimer(index);}

	/** Fires on expiration of a timer.
		Fires after a call to @b setTimer(). Subclasses should override this.
		@param index Timer number that fired. Will be between 0 and the value given to @b initTimers()
	*/	
	float remainingTimer(unsigned int index) {checkCT();return ct->remainingTimer(index);}

	/* Timer "fire" handler routine. Needs to be overriden by classes that want to use Timer
	 * @param index The timer number for the timer that has just completed
	 */
	virtual void handleTimer(unsigned int index)=0;

	/** Set a "context pointer" refering to some piece of opaque useful data
	 * @param index Timer number
	 * @param data Opaque pointer. Never free'd or dereferenced
	 */
	void setContextPointer(unsigned int index,void * data) {checkCT();ct->setContextPointer(index,data);}

	/** Retreive a "context pointer" refering to some piece of opaque useful data
	 * @param index Timer number
	 * @return Opaque pointer from @setContextPointer
	 */
	void * contextPointer(unsigned int index) {checkCT();return ct->contextPointer(index);}

	/** Provide a destructor function for a "context pointer" such that we can
	 *  do complete cleanup even if there are still timers remaining at the end of a
	 *  simulation. Called on end of sim for still scheduled timers.
	 *  @param index Timer number
	 */
	void setContextDestructor(unsigned int index, void (*func)(void * data)){checkCT();ct->setContextDestructor(index,func);}

	/* Mark the first @count pointer ids (from 0 to @count-1) as allocated, so they don't get
	 * auto-allocated by setTimer
	 * @param count Number of timers to allocate
	 */
	void allocateTimers(unsigned int count) {checkCT();ct->allocateTimers(count);}

	/* Delete a timer. Useful for auto-allocated timers that you don't need any more to 
	 * reduce memory usage. Does nothing if the timer doesn't exist, and works even if 
	 * a timer is still running.
	 *
	 * This does not need to be called for every timer, as the Timer module will automatically cleanup
	 * all of the remaining timers at the end of the simulation. It is a tool for reclaiming memory 
	 * early for simulations that might need many timers, and would otherwise fill up all the free 
	 * memory with old timers.
	 * @param index Timer to wipe
	 */
	void deleteTimer(unsigned int index) {checkCT();ct->deleteTimer(index);}

private: 
	/* checkCT is an internal check function to test if init() has been called */
	void checkCT() 
	{
		if (ct == NULL)
			opp_error("init() must be called before using Timer functions");
	}

};

#endif
