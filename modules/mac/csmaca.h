#ifndef __CSMACAH__
#define __CSMACAH__

#include "mac.h"

// protocol config
#define RTS_CONTEND_TIME	100
#define MAX_RTS_CONTEND_TIME	300
#define CTS_CONTEND_TIME	5
#define DATA_CONTEND_TIME	5
#define ACK_CONTEND_TIME	5
#define ALLOWED_DRIFT		30

enum ProtoState {
	PROTO_STATE_IDLE,
	PROTO_STATE_CONTEND,
	PROTO_STATE_WFCTS,
	PROTO_STATE_WFDATA,
	PROTO_STATE_SEND_RTS,
	PROTO_STATE_SEND_CTS,
	PROTO_STATE_SEND_DATA,
	PROTO_STATE_WFACK,
	PROTO_STATE_SEND_ACK	
};
enum NavState {
	NAV_STATE_CLEAR,
	NAV_STATE_BUSY
};

enum Timers {
	TIMER_PROTOCOL,
	TIMER_NAV
};

enum MessageKind {
	KIND_DATA,
	KIND_RTS,
	KIND_CTS,
	KIND_ACK
};

struct Header {
	MessageKind kind;
	unsigned short nav;
};
	
#define FLENGTH(x)	((int)((frameTotalTime((x)+headerLength()))*32768.0)+1)
#define CTS_FLENGTH	(FLENGTH(0))
#define ACK_FLENGTH	(FLENGTH(0))

#define TIMEOUT_WFCTS	(CTS_FLENGTH+CTS_CONTEND_TIME+10)
#define TIMEOUT_WFDATA	(FLENGTH(0)+DATA_CONTEND_TIME)
#define TIMEOUT_WFACK	(ACK_FLENGTH+ACK_CONTEND_TIME+10)

#define NAV_RTS(x)	(CTS_CONTEND_TIME+CTS_FLENGTH+\
				DATA_CONTEND_TIME+FLENGTH(x)+\
				ACK_CONTEND_TIME+ACK_FLENGTH)

#define NAV_ACK		(ACK_CONTEND_TIME+ACK_FLENGTH)


//typedef unsigned short unsigned short;

class CsmaCA: public EyesMacLayer {

	// contructor, destructor, module stuff
	Module_Class_Members(CsmaCA, EyesMacLayer, 0);
	~CsmaCA();

protected:
	int flags;

	int proto_state;
	int proto_next_state;

	int nav_state;
	unsigned short nav_end_time;

	MacPacket * tx_msg;

	int rts_contend_time;
	int cts_to;
	unsigned short cts_nav_end;
	unsigned short cts_nav_rcv;
	unsigned short cts_nav_t;

	int ack_to;

	void evalState();	// check state, may start transmission
	void startContending(int time);	// contend, ctime = <x>
	void sendRts();
	void sendCts();
	void sendData();
	void sendAck();
	void receiveRts(MacPacket * msg);
	void receiveCts(MacPacket * msg);
	void receiveData(MacPacket * msg);
	void receiveAck(MacPacket * msg);
	void setIdle();
	
	void protocolTimeout();	
	void navTimeout();	
	void setProtocolTimeout(int t);
	void setNavTimeout(int t);
	void updateNav(unsigned short nav);
	void txDone();

	virtual int mustUseCA(MacPacket * msg);
	virtual void incBackoff();
	virtual void decBackoff();

	virtual void initialize();
	virtual void timeout(int which);
	virtual void txPacket(MacPacket * msg);
	virtual void rxFrame(MacPacket * msg);
	virtual void transmitDone();
	virtual void rxFailed();
	virtual void rxStarted();
public:
	virtual int headerLength();
	virtual void endForce(){}
};



#endif
