#ifndef __TESTAPP_H__
#define __TESTAPP_H__

#include "application.h"

#define TESTAPP_TIMER_INTERVAL	50.0

/** Simple test application. */
class TestApp: public Application {
	Module_Class_Members(TestApp, Application, 0);

	public:
		virtual void initialize();
		virtual void finish();
		virtual void handleTimer(unsigned int count);
		virtual void handleEvent(cMessage *msg);
		virtual ~TestApp();
};

#endif

