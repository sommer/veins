#ifndef __HOPLATENCY_H__
#define __HOPLATENCY_H__

#include "pattern.h"

class HopLatency : public Pattern {
	Module_Class_Members(HopLatency, Pattern, 0);

protected:
	int source;

	void init();
	void activated();
	void deActivated();
	void rx(Packet * msg);
	void timeout();

	void generate();

public:
};

#endif
