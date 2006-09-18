#ifndef __TOSINKAPP_H__
#define __TOSINKAPP_H__

#include "application.h"

/** To Sink protocol testing application.
*/
class TosinkApp : public Application {
	Module_Class_Members(TosinkApp, Application, 0);

	private:
		static bool parametersInitialised;

	public:
		virtual void initialize();
		virtual void finish();
		virtual void handleTimer(unsigned int count);
		virtual void handleEvent(cMessage *msg);
		virtual ~TosinkApp();

	protected:
		int stat_tx_done, stat_tx_fail, stat_rx, stat_initiated;
		double stat_sumlatency;
		static int destination, dataSize;
		static double interval, initialWait;
};

#endif
