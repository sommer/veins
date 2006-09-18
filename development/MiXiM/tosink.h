#ifndef __TOSINK_H__
#define __TOSINK_H__

#include "pattern.h"

class PropagationModel;

class ToSink : public Pattern {
	Module_Class_Members(ToSink, Pattern, 0);

protected:
	PropagationModel * model;
	int sinknode;
	int ttl;
	cArray localQ;
	int agg_mode;
	double agg_timeout;
	int agg_max;

	void init();
	void activated();
	void deActivated();

	void generate();
	void rx(Packet * msg);
	void timeout();
	void timeout2();

	//~ void route(Packet *msg);
	void aggregate(Packet * msg);
	void sendAggregated();

	struct Header {
		double sendTime;
		int ttl;
		int msgCount;
	};
public:
};


#endif
