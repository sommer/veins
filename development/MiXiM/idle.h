#ifndef __IDLE_H__
#define __IDLE_H__

#include "pattern.h"

class Idle : public Pattern {
	Module_Class_Members(Idle, Pattern, 0);

protected:
	void init();
	void activated();
	void deActivated();
	void rx(Packet * msg);
	void timeout();

	void generate();

public:
};


#endif
