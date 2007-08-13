#include "UnitDisk.h"

Define_Module(UnitDisk);

void UnitDisk::initialize(int stage)
{
	ConnectionManager::initialize(stage);
	if (stage == 0)
	{
		cPar *p = addPar("logName");
		p->setStringValue("UnitDisk");
		radioRange = par("radioRange");
		EV << "UnitDisk initialised with range "<<radioRange<<endl;
	}
}

