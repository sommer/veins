#ifndef TIMER_CORE_H
#define TIMER_CORE_H 1

#include <omnetpp.h>

class Timer;

class TimerCore: public cSimpleModule
{
	protected:
		Timer *timer;

		std::map <unsigned int, cMessage *> *timers;

		/** @brief Handle self messages */
		virtual void handleMessage(cMessage* msg);

	public:
	    Module_Class_Members(TimerCore, cSimpleModule, 0);
		~TimerCore();

		void init (Timer* t);

		/** Set a timer to fire at a point in the future.
			If the timer with that id has already been set then this discards the old information.
			@param index Timer number to set.
			@param when Time in seconds in the future to fire the timer
		 */
		void setTimer(unsigned int index, double when);

		/** Set a timer to fire at a point in the future.
			Auto-generates a timer id that's guaranteed not to have been used by anyone else.
			If the timer with that id has already been set then this discards the old information.
			@param when Time in seconds in the future to fire the timer
			@return Timer id
		 */
		unsigned int setTimer(double when);

		/** Cancel an existing timer set by @b setTimer()
			If the timer has not been set, or has already fires, this does nothing
			Must call @b initTimers() before using.
			@param index Timer to cancel. Must be between 0 and the value given to @b initTimers()
		 */
		void cancelTimer(unsigned int index);

		/** Fires on expiration of a timer.
			Fires after a call to @b setTimer(). Subclasses should override this.
			@param index Timer number that fired. Will be between 0 and the value given to @b initTimers()
		*/	

		float remainingTimer(unsigned int index);
};

#endif
