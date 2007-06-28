#include "FrameTimer.h"

void FrameTimer::init(BaseModule *parent)
{
	owner = parent;
	tg = dynamic_cast<FrameTimerGenerator*>(findModuleType("FrameTimerGenerator")->createScheduleInit("generator",owner));
	tg->init(this);
}
