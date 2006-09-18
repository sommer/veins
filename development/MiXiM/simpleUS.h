#ifndef __SIMPLEUS_H__
#define __SIMPLEUS_H__

#include "ultrasound.h"

/** Simple ultrasound device. */
class SimpleUS: public Ultrasound {
	Module_Class_Members(SimpleUS, Ultrasound, 0);

	public:
		virtual void initialize();
		virtual void finish();
};

#endif

