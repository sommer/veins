#ifndef __PROPAGATIONMODEL_H__
#define __PROPAGATIONMODEL_H__

#include "mixim.h"

/** Base class for radio propagation models. */
class PropagationModel: public MiximBaseModule {
	Module_Class_Members(PropagationModel, MiximBaseModule, 0);
};

#endif

