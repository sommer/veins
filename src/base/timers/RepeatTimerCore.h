#ifndef REPEATTIMER_CORE_H
#define REPEATTIMER_CORE_H 1

#include <omnetpp.h>

#include "BaseModule.h"
#include "MiXiMDefs.h"

class RepeatTimer;

class MIXIM_API RepeatTimerCore: public BaseModule
{
	public:
		typedef void (cleanup)(void * data);
		typedef struct {
			cMessage * timer;
			cleanup * destructor;
			int repeats, count;
			double when;
		} TInfo;
	protected:
		void checkExists(unsigned int index);
		RepeatTimer *timer;

		std::map <unsigned int, TInfo> *timer_map;

		/** @brief Handle self messages */
		virtual void handleMessage(cMessage* msg);

	public:
		//Module_Class_Members(RepeatTimerCore, BaseModule, 0);
		~RepeatTimerCore();

		void init (RepeatTimer* t);

		/** Set a timer to fire at a point in the future.
		 * If the timer with that id has already been set then this discards the old information.
		 * @param index RepeatTimer number to set.
		 * @param when Time in seconds in the future to fire the timer
		 */
		void setRepeatTimer(unsigned int index, double when, int repeats = 1);

		/** Set a timer to fire at a point in the future.
		 * Auto-generates a timer id that's guaranteed not to have been used by anyone else.
		 * If the timer with that id has already been set then this discards the old information.
		 * @param when Time in seconds in the future to fire the timer
		 * @return RepeatTimer id
		 */
		unsigned int setRepeatTimer(double when, int repeats = 1);

		/** Cancel an existing timer set by @b setRepeatTimer()
		 * If the timer has not been set, or has already fires, this does nothing
		 * Must call @b init() before using.
		 * @param index RepeatTimer to cancel. Must be between 0 and the value given to @b init()
		 */
		void cancelRepeatTimer(unsigned int index);

		void resetRepeatTimer(unsigned int index);

		void resetAllRepeatTimers(void);

		bool timerExists(unsigned int index);

		/** Fires on expiration of a timer.
		 * Fires after a call to @b setRepeatTimer(). Subclasses should override this.
		 * @param index RepeatTimer number that fired. Will be between 0 and the value given to @b init()
		 */	
		float remainingRepeatTimer(unsigned int index);

		/** Set a "context pointer" refering to some piece of opaque useful data
		 * @param index RepeatTimer number
		 * @param data Opaque pointer. Never free'd or dereferenced
		 */
		void setContextPointer(unsigned int index,void * data);

		/** Retreive a "context pointer" refering to some piece of opaque useful data
		 * @param index RepeatTimer number
		 * @return Opaque pointer from @setContextPointer
		 */
		void * contextPointer(unsigned int index);

		/** Provide a destructor function for a "context pointer" such that we can
		 *  do complete cleanup even if there are still timers remaining at the end of a
		 *  simulation. Called on end of sim for still scheduled timers.
		 *  @param index RepeatTimer number
		 */
		void setContextDestructor(unsigned int index, cleanup *);

		/* Mark the first @count pointer ids (from 0 to @count-1) as allocated, so they don't get
		 * auto-allocated by setRepeatTimer
		 * @param count Number of timers to allocate
		 */
		void allocateRepeatTimers(unsigned int count);

		/* Delete a timer. Useful for auto-allocated timers that you don't need any more to 
		 * reduce memory usage. Does nothing if the timer doesn't exist
		 * @param index RepeatTimer to wipe
		 */
		void deleteRepeatTimer(unsigned int index);
};

#endif
