#ifndef __SIMPLEULTRASOUNDPROPAGATIONMODEL_H__
#define __SIMPLEULTRASOUNDPROPAGATIONMODEL_H__

#include "propagationModel.h"

/** Simple ultrasound propagation model. */
class SimpleUSPropagationModel: public PropagationModel {
	Module_Class_Members(SimpleUSPropagationModel, PropagationModel, 0);

    public:
	virtual void initialize();
	virtual void finish();
};

#endif

