#include <omnetpp.h>
#include "channelStateMsg_m.h"

// module class declaration:
class errorCalc:public cSimpleModule {
      public:
	Module_Class_Members(errorCalc, cSimpleModule, 0)
	virtual void initialize();
	virtual void handleMessage(cMessage * msg);
	virtual void finish();

      private:
};

// module type registration:
Define_Module(errorCalc);

void errorCalc::initialize()
{
}

// implementation of the module class:
void errorCalc::handleMessage(cMessage * msg)
{
	send(msg, "out");
}

void errorCalc::finish()
{
}
