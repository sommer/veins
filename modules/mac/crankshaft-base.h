#ifndef __CRANKSHAFT_H__
#define __CRANKSHAFT_H__

#include "mac.h"
#include <vector>
#include <climits>

#define SINK_ALWAYS_ON

#define CLOCK_BYTES 3
#define ADDRESS_BYTES 2
#define TYPE_BYTES 1

class CrankshaftBase: public EyesMacLayer {

	// contructor, destructor, module stuff
	//Module_Class_Members(CrankshaftBase, EyesMacLayer, 0);

private:
	static bool parametersInitialised;

protected:
	enum {
		SLOT_TIMER,
		CONTENTION_TIMER,
		MSG_TIMEOUT,
		ACK_TIMEOUT,
		POLL_TIMER
	};

	enum MsgType {
		MSG_DATA,
		MSG_ACK,
		MSG_BCAST,
		MSG_NOTIFY
	};

	enum {
		SEND_NONE,
		SEND_ACK,
		SEND_DATA,
		SEND_NOTIFY,
		SEND_WFACK
	} send_state;
	
	enum {
		INIT_NONE,
		INIT_DO_NOTIFIY,
		INIT_DONE
	} initialized ;

	enum SlotState {
		SSTATE_NONE,
		SSTATE_SEND,
		SSTATE_SEND_RECEIVE,
		SSTATE_RECEIVE,
		SSTATE_SLEEP
	} current_slot_state;

	enum {
		CHECK_ACTIVITY,
		CHECK_STARTSYM
	} msgTimeoutState;

	int current_slot, backoff;
	int max_header_length, min_header_length;
	int ack_to, retries;
	bool precontend;
	
	static int data_length, backoff_max, max_retries;
	static bool useScp, useSift, rerouteOnFail, quickAbort;
	static double retryChance;

	/* MacPacket that will be sent to the radio. */
	MacPacket *packet;
	/* MacPacket that is received (and will be sent back to) the routing layer. */
	MacPacket *tx_msg;

	virtual void initialize();
	virtual void finish();
	virtual void timeout(int which);
	virtual void txPacket(MacPacket * msg);
	virtual void rxFrame(MacPacket * msg);
	virtual void rxStarted();
	virtual void transmitDone();
	virtual void rxFailed();
	virtual void rxHeader(MacPacket * msg);

	virtual void wrapSlotCounter();
	virtual SlotState getCurrentSlotState() = 0;
	virtual int slotsUntilWake(int destination) = 0;

	virtual void contend(void);

	class Header {
	public:
		virtual void *data() = 0;
		virtual int dataSize() = 0;
		virtual int extraLength() = 0;
		virtual MsgType getType() = 0;
		virtual ~Header() {}
	};
	
	class MessageInfo {
		public:
			int id;
			bool last_failed;
			MessageInfo(int _id) : id(_id), last_failed(false) {}
	};

	virtual Header *newHeader(MsgType type) = 0;
	virtual Header *newHeader(void *data) = 0;

	virtual ~CrankshaftBase();
public:
	virtual int headerLength();
	virtual int firstToWake(std::vector<int> *nodes);
};


#endif // __CRANKSHAFT_H__
