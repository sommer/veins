#include "UnitDisk.h"

Define_Module(UnitDisk);

#ifndef udEV
#define udEV (ev.isDisabled()||!debug) ? ev : ev << "UnitDisk: "
#endif

void UnitDisk::initialize(int stage)
{
	ConnectionManager::initialize(stage);
	if (stage == 0)
	{
		debug = par("debug").boolValue();
		//cPar *p = addPar("logName"); //TODO: addPar removed in omnet 4, replace it
		//p->setStringValue("UnitDisk");
		radioRange = par("radioRange").doubleValue();
		udEV << "UnitDisk initialised with range "<<radioRange<<endl;
	}
}

