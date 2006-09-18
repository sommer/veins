#ifndef __AREAMANAGER_H__
#define __AREAMANAGER_H__

#include "mixim.h"

extern int centerNode;

class AreaManager: public MiximBaseModule {
	Module_Class_Members(AreaManager, MiximBaseModule, 0);

protected:
	cMessage * timeout;
	double area_size;
	double area_size_v; // maximum distance to avg
	double area_lifetime;
	double area_ival;
	int nnodes;
	double * nodex, * nodey;

	void initialize();
	void finish();
	void handleMessage(cMessage * msg);

	void newArea();
	virtual ~AreaManager();
};


#endif
