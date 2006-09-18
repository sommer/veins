#ifndef __WORLD_H__
#define __WORLD_H__

#include "mixim.h"

/** Container class for data about the physical world. */
class World: public MiximBaseModule {
	Module_Class_Members(World, MiximBaseModule, 0);

	public:
		virtual void initialize();
		virtual void finish();
};

#endif

