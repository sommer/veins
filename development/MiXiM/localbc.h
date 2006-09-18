#ifndef __LOCALBC_H__
#define __LOCALBC_H__

#include "pattern.h"

class LocalBC : public Pattern {
	Module_Class_Members(LocalBC, Pattern, 0);

protected:
	double reply_prob;
	double reply_delay;
	
	void init();
	void activated();
	void deActivated();
	virtual void rxDelay(cMessage * msg);

	void generate();
	void rx(Packet * msg);
	void timeout();

public:
};


#endif
