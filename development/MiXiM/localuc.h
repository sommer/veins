#ifndef __LOCALUC_H__
#define __LOCALUC_H__

#include "pattern.h"

class LocalUC : public Pattern {
	Module_Class_Members(LocalUC, Pattern, 0);

protected:
	double reply_prob, reply_delay;
	
	void init();
	void activated();
	void deActivated();
	void rx(Packet * msg);
	void timeout();

	void generate();

public:
};


#endif
