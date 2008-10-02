#include "RepeatTimer.h"

void RepeatTimer::init(BaseModule * parent)
{
	owner = parent;
	core = dynamic_cast < RepeatTimerCore * >
	    (cModuleType::get("RepeatTimerCore")->
	     createScheduleInit("generator", owner));
	core->init(this);
}
