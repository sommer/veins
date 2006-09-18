#ifndef __LOCALGOSSIP_H__
#define __LOCALGOSSIP_H__

#include "pattern.h"

class LocalGossip : public Pattern {
	Module_Class_Members(LocalGossip, Pattern, 0);

protected:
	double reply_prob, reply_delay;
	bool gossiping;

	void init();
	void activated();
	void deActivated();
	void rx(Packet * msg);
	void timeout();

	void generate();

	bool checkGossiping(int centernode, int hops);

public:
};


#endif
