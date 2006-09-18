#ifndef __MULTIHOPAPP_H__
#define __MULTIHOPAPP_H__

#include "application.h"

/** Multihop routing test application.

    This class implements an application that sends data from node 0 to node 1.
    For this to work, it needs either for node 0 and 1 to be within radio
    range, or a working multihop routing protocol.
*/
class MultihopApp : public Application {
	Module_Class_Members(MultihopApp, Application, 0);

	public:
		virtual void initialize();
		virtual void finish();
		virtual void handleTimer(unsigned int count);
		virtual void handleEvent(Packet *msg);

	protected:
		int stat_tx_done, stat_tx_fail, stat_rx;
		int source, destination;
};

#endif
