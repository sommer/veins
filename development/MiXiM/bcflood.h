#ifndef __BCFLOOD_H__
#define __BCFLOOD_H__

#include "pattern.h"

class BCFlood : public Pattern {
	Module_Class_Members(BCFlood, Pattern, 0);

protected:
	int source, mySerial, lastSerial, bitmap;
	
	void init();
	void activated();
	void deActivated();

	void generate();
	void rx(Packet * msg);
	void timeout();

	bool registerSerial(int serial);

	struct Header {
		double sendTime;
		int serial;
	};

public:
};


#endif
