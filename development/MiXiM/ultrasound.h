#ifndef __ULTRASOUND_H__
#define __ULTRASOUND_H__

#include "mixim.h"

/** Base class for ultrasound devices. */
class Ultrasound : public MiximBaseModule {
	Module_Class_Members(Ultrasound, MiximBaseModule, 0);

	public:
		virtual void initialize(void);
		virtual void finish(void);
		virtual void handleMessage(cMessage*);
};

#endif

