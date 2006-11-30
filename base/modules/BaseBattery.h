#ifndef BASE_BATTERY_H
#define BASE_BATTERY_H

#include <BaseModule.h>

class BaseBattery : public BaseModule {
	Module_Class_Members(BaseBattery, BaseModule, 0)
		virtual void activity();
	// Add you own member functions here!
};

#endif

