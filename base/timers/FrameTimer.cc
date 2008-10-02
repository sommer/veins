#include "FrameTimer.h"

void FrameTimer::init(BaseModule *parent)
{
	owner = parent;
	tg = dynamic_cast<FrameTimerGenerator*>(cModuleType::get("FrameTimerGenerator")->createScheduleInit("generator",owner));
	tg->init(this);
}
