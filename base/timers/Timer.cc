#include "Timer.h"
#include "TimerCore.h"

void Timer::init(cModule *parent)
{
	owner = parent;
	if (ct==NULL)
	{
		ct = dynamic_cast<TimerCore*>(cModuleType::get("TimerCore")->createScheduleInit("TimerCore",owner));
		ct->init(this);
	}
}
