#ifndef REPEAT_TIMER_H
#define REPEAT_TIMER_H 1

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "RepeatTimerCore.h"
#include "BaseModule.h"

/* repeat timers are repeating timers like FrameTimer,
 * but with a fixed number of repeats.
 * For more info see: FrameTimer.(h|cc)
 */
class MIXIM_API RepeatTimer {
	friend class RepeatTimerCore;
protected:
	RepeatTimerCore * core;
	BaseModule *owner;
public:
	virtual ~RepeatTimer() {
		delete core;
	}
	void setRepeatTimer(unsigned int index, double period, int repeats = 1) {
		core->setRepeatTimer(index, period, repeats);
	}
	unsigned int setRepeatTimer(double period, int repeats = 1) {
		return core->setRepeatTimer(period, repeats);
	}
	void cancelRepeatTimer(unsigned int index) {
		core->cancelRepeatTimer(index);
	}
	void resetRepeatTimer(unsigned int index) {
		core->resetRepeatTimer(index);
	}
	void resetAllRepeatTimers(void) {
		core->resetAllRepeatTimers();
	}
	bool timerExists(unsigned int index) {
		return core->timerExists(index);
	}

	float remainingRepeatTimer(unsigned int index) {
		return core->remainingRepeatTimer(index);
	}
	void setContextPointer(unsigned int index,void * data) {
		core->setContextPointer(index,data);
	}
	void * contextPointer(unsigned int index) {
		return core->contextPointer(index);
	}
	void setContextDestructor(unsigned int index, void (*func)(void * data)) {
		core->setContextDestructor(index,func);
	}
	void allocateRepeatTimers(unsigned int count) {
		core->allocateRepeatTimers(count);
	}
	void deleteRepeatTimer(unsigned int index) {
		core->deleteRepeatTimer(index);
	}

	virtual void init(BaseModule * parent);
	virtual void handleRepeatTimer(unsigned int index){}
};

#endif
