#ifndef __LINKTEST_H__
#define __LINKTEST_H__

#include "application.h"

#define LINKTEST_TIMER_INTERVAL	10.0

/** Simple test application. */
class LinkTest: public Application {
	Module_Class_Members(LinkTest, Application, 0);

	public:
		virtual void initialize();
		virtual void finish();
		virtual void handleTimer(unsigned int count);
		virtual void handleEvent(cMessage *msg);
		virtual ~LinkTest();
	private:
		double timer_interval;
		int node_id;
		int node_count;
		unsigned stats_tx, stats_rx;
};

#endif

