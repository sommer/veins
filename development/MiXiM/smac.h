#ifndef __SMACH__
#define __SMACH__

#include "message.h"
#include "mixim.h"
#include "mac.h"

// protocol config
/** Default frame time in 32KiHz clock ticks. */
#define FRAME_TIME	(32768)
/** Default listen time in 32KiHz clock ticks. */
#define LISTEN_TIME	(1000)
/** Contention window length for SYNC packets in 32KiHz clock ticks. */
#define SYNC_CONTEND_TIME	300
//#define SYNC_CONTEND_TIME	100
/** Contention window length for RTS packets in 32KiHz clock ticks. */
#define RTS_CONTEND_TIME	300
//#define RTS_CONTEND_TIME	100
//#define MAX_RTS_CONTEND_TIME	300
/** Time to wait before sending a CTS packet in 32KiHz clock ticks. */
#define CTS_CONTEND_TIME	5
/** Time to wait before sending a DATA packet in 32KiHz clock ticks. */
#define DATA_CONTEND_TIME	5
/** Time to wait before sending a ACK packet in 32KiHz clock ticks. */
#define ACK_CONTEND_TIME	5
/** Maximum schedule drift within one schedule in 32KiHz clock ticks. */
#define ALLOWED_DRIFT		30
/** Number of bytes in a SYNC packet (excluding header). */
#define SYNC_SIZE		2
/** Length of a SYNC packet in 32KiHz clock ticks. */
#define EST_SYNC_TIME		((int)(frameTotalTime(SYNC_SIZE+\
			headerLength())*32768.0))

/** Network Allocation Vector states. */
enum NavState {
	NAV_STATE_CLEAR,
	NAV_STATE_BUSY
};

/** Message kinds. */
enum MessageKind {
	KIND_DATA,
	KIND_RTS,
	KIND_CTS,
	KIND_SYNC,
	KIND_ACK
};

/** Timer types. */
enum {
	TIMER_PROTOCOL,
	TIMER_NAV,
	TIMER_SCHED,
	TIMER_ACTIVE
};

/** Header data. */
struct Header {
	/** Kind of packet (RTS/CTS/DATA/ACK). */
	MessageKind kind;
	/** Network Allocation Vector (time the packet exchange will take). */
	int nav,
	/** Field to include synchronisation data. Only used for SYNC packets. */
		sync;
};

/** Macro to access @a kind field in @b Header struct. */
#define PKT_KIND(p) (((Header *)(p)->getData(MAC_DATA))->kind)
/** Macro to access @a nav field in @b Header struct. */
#define PKT_NAV(p) (((Header *)(p)->getData(MAC_DATA))->nav)
/** Macro to access @a sync field in @b Header struct. */
#define PKT_SYNC(p) (((Header *)(p)->getData(MAC_DATA))->sync)

/** Calculate length of packet with length @a x in 32 KiHz clock ticks. */
#define FLENGTH(x)	((int)((frameTotalTime((x)+headerLength()))*32768.0)+1)
/** Length of a CTS packet in 32 KiHz clock ticks. */
#define CTS_FLENGTH	(FLENGTH(0))
/** Length of an ACK packet in 32 KiHz clock ticks. */
#define ACK_FLENGTH	(FLENGTH(0))

/** Time to wait for an incoming CTS packet in 32KiHz clock ticks. */
#define TIMEOUT_WFCTS	(CTS_FLENGTH+CTS_CONTEND_TIME+10)
/** Time to wait for an incoming DATA packet in 32KiHz clock ticks. */
#define TIMEOUT_WFDATA	(FLENGTH(0)+DATA_CONTEND_TIME)
/** Time to wait for an incoming ACK packet in 32KiHz clock ticks. */
#define TIMEOUT_WFACK	(ACK_FLENGTH+ACK_CONTEND_TIME+10)

#define NAV_RTS(x)	(CTS_CONTEND_TIME+CTS_FLENGTH+\
				DATA_CONTEND_TIME+FLENGTH(x)+\
				ACK_CONTEND_TIME+ACK_FLENGTH)

#define NAV_ACK		(ACK_CONTEND_TIME+ACK_FLENGTH)

#define RESYNC_LOW	100
#define RESYNC_HIGH	200
#define NEW_RESYNC_COUNTER ((int)intuniform(RESYNC_LOW*20000/frame_time, RESYNC_HIGH*20000/frame_time))

/** Number of times to try a data packet exchange. */
#define PACKET_RETRIES 3

typedef unsigned short ushort;

/** Schedule states. */
enum SchedState {
	SCHED_STATE_SLEEP,
	SCHED_STATE_OWN,
	SCHED_STATE_OTHER,
	SCHED_STATE_STARTUP
};

/** Protocol states. */
enum ProtoState {
	PROTO_STATE_INVALID,
	PROTO_STATE_IDLE,
	PROTO_STATE_CONTEND,
	PROTO_STATE_WFCTS,
	PROTO_STATE_WFDATA,
	PROTO_STATE_SEND_RTS,
	PROTO_STATE_SEND_CTS,
	PROTO_STATE_SEND_DATA,
	PROTO_STATE_SEND_SYNC,
	PROTO_STATE_WFACK,
	PROTO_STATE_SEND_ACK
};

class SMac: public Mac {

	// contructor, destructor, module stuff
	Module_Class_Members(SMac, Mac, 0);

public:
	virtual void initialize();
	virtual void finish();
	
protected:
	unsigned int listen_time;
	unsigned int sleep_time;
	unsigned int frame_time;
	ushort sched_table[10]; // neighbour schedules as delta of own
	int sched_count;

	SchedState sched_state;
	ushort time_last_sched;	// the last time at which my own sched was

	ProtoState proto_state;
	ProtoState proto_next_state;

	NavState nav_state;
	ushort nav_end_time;

	int must_send_sync;
	int resync_counter;
	Packet * tx_msg;

	int packet_retries;

	int rts_contend_time;
	int cts_to;
	ushort cts_nav_end;
	ushort cts_nav_rcv;
	ushort cts_nav_t;

	int ack_to;

	void setMySchedule(ushort time); // set my sched to start <x> ago
	void evalState();	// check state, may start transmission
	void startContending(int time);	// contend, ctime = <x>
	void sendSync();	// send SYNC _now_
	void sendRts();
	void sendCts();
	void sendData();
	void sendAck();
	void receiveSync(Packet * msg);	// received SYNC
	void receiveRts(Packet * msg);
	void receiveCts(Packet * msg);
	void receiveData(Packet * msg);
	void receiveAck(Packet * msg);
	void adoptSchedule(int offset); // extra schedule
	void calcSchedState();	// recalculate: sleep or listen
	void setIdle();
	
	void protocolTimeout();	
	void navTimeout();	
	void schedTimeout();
	void setProtocolTimeout(int t);
	void setNavTimeout(int t);
	void setSchedTimeout(int t);
	void updateNav(ushort nav);
	void txDone(bool success);

	virtual void endForce();

	int isSameSchedule(int time1, int time2);

	virtual int mustUseCA(Packet * msg);
	//virtual void incBackoff();
	virtual void decBackoff();

//	virtual void init();
	virtual void timeout(int which);
	virtual void txPacket(Packet * msg);
	virtual void rxFrame(Packet * msg);
	virtual void transmitDone();
	virtual void rxFailed();
	virtual void rxStarted();
	virtual int headerLength();
};



#endif
