#ifndef __TMAC_H__
#define __TMAC_H__

#include "mac.h"
#include "message.h"

/** Default frame time in 32KiHz clock ticks. */
#define FRAME_TIME	(20000)
#define WAIT_TX_ACK	0
#define WAIT_TX_DONE	0

/** Number of times to retry an RTS before waiting for other events instead. */
#define RTS_RETRIES	1
#define RTS_IGNORE_TRESHOLD	2

/** Number of times to try a data packet exchange. */
#define PACKET_RETRIES 3

#define RTS_CONTEND_TIME	300
#define SYNC_CONTEND_TIME	300
#define MIN_RTS_CONTEND_TIME	RTS_CONTEND_TIME
#define RTS_WAIT_BONUS		30

#define MIN_CONTEND_TIME 5

#define BACKOFF_TIME		0

/** Default listen timeout in 32KiHz clock ticks. */
#define LISTEN_TIMEOUT	(500)

/** Time to wait before sending a CTS packet in 32KiHz clock ticks. */
#define CTS_CONTEND_TIME	5
/** Time to wait before sending a DATA packet in 32KiHz clock ticks. */
#define DATA_CONTEND_TIME	5
/** Time to wait before sending a ACK packet in 32KiHz clock ticks. */
#define ACK_CONTEND_TIME	5
/** Maximum schedule drift within one schedule in 32KiHz clock ticks. */
#define ALLOWED_DRIFT		10
/** Number of bytes in a SYNC packet (excluding header). */
#define SYNC_SIZE		2
/** Number of bytes in a DS packet (excluding header). */
#define DS_SIZE			0
/** Length of a SYNC packet in 32KiHz clock ticks. */
#define EST_SYNC_TIME		((int)(frameTotalTime(SYNC_SIZE+\
			headerLength())*32768.0))

/** Schedule states. */
enum SchedState {
	SCHED_STATE_ACTIVE,
	SCHED_STATE_STARTUP
};

/** Protocol states. */
enum ProtoState {
	PROTO_STATE_INVALID,
	PROTO_STATE_IDLE,
	PROTO_STATE_CONTEND,
	PROTO_STATE_FORCETIME,
	PROTO_STATE_WFCTS,
	PROTO_STATE_WFDATA,
	PROTO_STATE_SEND_RTS,
	PROTO_STATE_SEND_CTS,
	PROTO_STATE_SEND_DATA,
	PROTO_STATE_SEND_SYNC,
	PROTO_STATE_WFACK,
	PROTO_STATE_SEND_ACK,
	PROTO_STATE_WAIT,
	PROTO_STATE_SEND_DS,
	PROTO_STATE_SEND_FRTS
};

/** Network Allocation Vector states. */
enum NavState {
	NAV_STATE_CLEAR,
	NAV_STATE_BUSY
};

/** Activity states. */
enum ActiveState {
	ACTIVE_STATE_SLEEP,
	ACTIVE_STATE_ACTIVE
};

/** Message kinds. */
enum MessageKind {
	KIND_DATA,
	KIND_RTS,
	KIND_CTS,
	KIND_SYNC,
	KIND_ACK,
	KIND_DS,
	KIND_FRTS
};

/** Timer types. */
enum {
	TIMER_PROTOCOL,
	TIMER_NAV,
	TIMER_SCHED,
	TIMER_ACTIVE,
};

/** Header data. */
struct Header {
	/** Kind of packet (RTS/CTS/DATA/ACK/FRTS/DS). */
	MessageKind kind;
	/** Network Allocation Vector (time the packet exchange will take). */
	int nav,
	/** Field to include synchronisation data. Only used for SYNC packets. */
		sync;
};

/** Macro to access @a kind field in @b Header struct. */
#define PKT_KIND(p) (((Header *)(p)->getData())->kind)
/** Macro to access @a nav field in @b Header struct. */
#define PKT_NAV(p) (((Header *)(p)->getData())->nav)
/** Macro to access @a sync field in @b Header struct. */
#define PKT_SYNC(p) (((Header *)(p)->getData())->sync)

/** Calculate length of packet with length @a x in 32 KiHz clock ticks. */
#define FLENGTH(x)	((int)((frameTotalTime((x)+headerLength()))*32768.0)+1)
/** Length of a CTS packet in 32 KiHz clock ticks. */
#define CTS_FLENGTH	(FLENGTH(0))
/** Length of an ACK packet in 32 KiHz clock ticks. */
#define ACK_FLENGTH	(FLENGTH(0))
/** Length of a DS packet in 32 KiHz clock ticks. */
#define DS_FLENGTH	(FLENGTH(0))
/** Length of a FRTS packet in 32 KiHz clock ticks. */
#define FRTS_FLENGTH	DS_FLENGTH

/** Time to wait for an incoming CTS packet in 32KiHz clock ticks. */
#define TIMEOUT_WFCTS	(CTS_FLENGTH+CTS_CONTEND_TIME+10)
/** Time to wait for an incoming DATA packet in 32KiHz clock ticks. */
#define TIMEOUT_WFDATA	(FLENGTH(0)+DATA_CONTEND_TIME)
/** Time to wait for an incoming ACK packet in 32KiHz clock ticks. */
#define TIMEOUT_ACK	(ACK_FLENGTH+ACK_CONTEND_TIME+10)

/*~ #ifdef USE_RFTS
 ~ #define NAV_RTS(x)	(CTS_CONTEND_TIME+CTS_FLENGTH+\
				~ DATA_CONTEND_TIME+DS_FLENGTH+FLENGTH(x)+\
				~ ACK_CONTEND_TIME+ACK_FLENGTH)
~ #else
~ #define NAV_RTS(x)	(CTS_CONTEND_TIME+CTS_FLENGTH+\
				~ DATA_CONTEND_TIME+FLENGTH(x)+\
				~ ACK_CONTEND_TIME+ACK_FLENGTH)
~ #endif*/

//#define NAV_CTS		(CTS_CONTEND_TIME+CTS_FLENGTH+DATA_CONTEND_TIME)
#define NAV_CTS		(CTS_CONTEND_TIME+CTS_FLENGTH)

#define NAV_ACK		(ACK_CONTEND_TIME+ACK_FLENGTH)

#define NAV_FRTS(x)	(FLENGTH(x)+ACK_CONTEND_TIME+ACK_FLENGTH)

#define NAV_DS(x)	(FLENGTH(x)+ACK_CONTEND_TIME+ACK_FLENGTH)

/** @defgroup TMacFlags Flags for the T-MAC protocol. *//** @{ */
#define FLAG_USE_OVERHEARING_AVOIDANCE	0x0001
#define FLAG_USE_FRTS			0x0002
#define FLAG_USE_IGNORE_RTS		0x0004
/** @} */

#define RESYNC_LOW	100
#define RESYNC_HIGH 	200

typedef unsigned short ushort;

class TMac: public EyesMacLayer {

	//Module_Class_Members(TMac, EyesMacLayer, 0);

	bool wantsForce;

public:
	virtual int headerLength();
	virtual void initialize();
	virtual void finish();
	virtual void txPacket(MacPacket* msg);
	virtual ~TMac();

protected:
	static unsigned int frame_time;
	static unsigned int listen_timeout;
	static int max_packet_retries;
	static int flags;

	SchedState sched_state;
	ushort time_last_sched;
	ushort time_next_sched;
	ushort extra_sched[5];
	int extra_sched_count;
	int resync_counter;
	bool next_is_own;
	bool in_my_frame;

	ProtoState proto_state;
	ProtoState proto_next_state;

	NavState nav_state;
	ushort nav_end_time;

	ActiveState active_state;

	bool must_send_sync;
	MacPacket * tx_msg;

	int rts_contend_time;
	int rts_sendfailed;
	int rts_retries;
	
	int packet_retries;
	
	int cts_to;
	ushort cts_nav_end;
	ushort cts_nav_rcv;
	ushort cts_nav_t;

	int ack_to;
	bool allowRetries;

	void evalState();
	void startContending(int time);
	void sendSync();
	void sendRts();
	void sendCts();
	void sendData();
	void sendAck();
	void sendFRts();
	void sendDs();
	void receiveSync(MacPacket * msg);
	void receiveRts(MacPacket * msg);
	void receiveCts(MacPacket * msg);
	void receiveData(MacPacket * msg);
	void receiveAck(MacPacket * msg);
	void receiveDs(MacPacket * msg);
	void receiveFRts(MacPacket * msg);
	void setIdle();
	void setWait(int time);
	void gotoBackoff();
	void kickFrameActive();
	void kickFrameActive2();

	void protocolTimeout();	
	void navTimeout();	
	void schedTimeout();
	void activeTimeout();
	void setProtocolTimeout(int t);
	void setNavTimeout(int t);
	void setSchedTimeout(int t);
	void setActiveTimeout(int t);
	void updateNav(ushort nav);
	void setMySchedule(ushort t);
	void adoptSchedule(ushort s);
	bool isSameSchedule(ushort s1, ushort s2);
	void txDone(bool success);

	void forceRequest();

	void endForce();

//	virtual void incBackoff();
//	virtual void decBackoff();
	virtual int mustUseCA(MacPacket * pkt);

	virtual void timeout(int which);
	virtual void rxFrame(MacPacket * msg);
	virtual void transmitDone();
	virtual void rxFailed();
	virtual void rxStarted();
	inline int navRts(int length);

private:
	static bool parametersInitialised;
};

#endif

