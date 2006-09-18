#ifndef __APPSELECTORH__
#define __APPSELECTORH__

#include "application.h"

/** Traffic generator multiplexer. */

class AppSelector: public Application {

	Module_Class_Members(AppSelector, Application, 0);

private:
	static bool parametersInitialised;

public:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();

	struct Header {
		double sendTime;
		int pattern;
	};
protected:
	int active_refcount;
	int stat_tx, stat_rx;

	virtual void tx(Packet * msg);
	virtual void rx(Packet * msg);
	void txDone(Packet *packet);

	void becomeActive();
	void becomeIdle();

	virtual ~AppSelector();
};


#endif //APPSELECTORH
