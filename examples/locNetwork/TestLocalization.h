#ifndef TEST_LOCALIZATION_H
#define TEST_LOCALIZATION_H

#include "BaseLocalization.h"
#include "LocPkt_m.h"

#define MAX_NEIGHBOURS 10

struct Position {
	int id;
	double x;
	double y;
};

class TestLocalization:public BaseLocalization {
      public:
	Module_Class_Members(TestLocalization, BaseLocalization, 0);

	virtual void initialize(int);
	virtual void finish();

	enum APPL_MSG_TYPES {
		SEND_BROADCAST_TIMER,
		BROADCAST_MESSAGE,
		BROADCAST_REPLY_MESSAGE
	};

      private:
	bool neighbours[MAX_NEIGHBOURS];

      protected:
	 cMessage * delayTimer;

      protected:
	 virtual void handleSelfMsg(cMessage *);

	virtual void handleLowerMsg(cMessage *);

	void sendBroadcast();

	void sendReply(LocPkt * msg);
};

#endif				/* TEST_LOCALIZATION_H */
