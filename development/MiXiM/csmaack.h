#ifndef __CSMAACKH__
#define __CSMAACKH__

#include "mac.h"

// protocol config
#define RETRY_CONTEND_TIME	150
#define DATA_CONTEND_TIME	300
#define MAX_DATA_CONTEND_TIME	10000
#define ACK_CONTEND_TIME	5
#define ALLOWED_DRIFT		30

#define PACKET_RETRIES 3

enum Timers {
	TIMER_PROTOCOL,
	TIMER_NAV
};

enum NavState {
	NAV_STATE_CLEAR,
	NAV_STATE_BUSY
};

enum ProtoState {
	PROTO_STATE_IDLE,
	PROTO_STATE_CONTEND,
	PROTO_STATE_WFACK,
	PROTO_STATE_SEND_DATA,
	PROTO_STATE_SEND_ACK,
	PROTO_STATE_NONE
};

enum MessageKind {
	KIND_DATA,
	KIND_ACK
};

struct Header {
	MessageKind kind;	
};

#define FLENGTH(x)	((int)((frameTotalTime((x)+headerLength()))*32768.0)+1)
#define ACK_FLENGTH	(FLENGTH(0))

#define TIMEOUT_WFACK	(ACK_FLENGTH+ACK_CONTEND_TIME+10)

#define NAV_ACK		(ACK_FLENGTH+ACK_CONTEND_TIME)

class CsmaAck: public Mac {

	// contructor, destructor, module stuff
	Module_Class_Members(CsmaAck, Mac, 0);
	
protected:
	ProtoState proto_state;
	ProtoState proto_next_state;

	NavState nav_state;
	ushort nav_end_time;

	int max_packet_retries, packet_retries;

	Packet * tx_msg;

	int data_contend_time;
	int ack_to;

	void evalState();	// check state, may start transmission
	void startContending(int time);	// contend, ctime = <x>
	void sendAck();
	void sendData();
	void receiveAck(Packet * msg);
	void receiveData(Packet * msg);
	void setIdle();
	
	void protocolTimeout();	
	void navTimeout();	
	void setProtocolTimeout(int t);
	void setNavTimeout(int t);
	void updateNav(ushort nav);

	virtual void incBackoff();
	virtual void decBackoff();

	virtual void initialize();
	virtual void timeout(int which);
	virtual void txPacket(Packet * msg);
	virtual void rxFrame(Packet * msg);
	virtual void transmitDone();
	virtual void rxFailed();
	virtual void rxStarted();
public:
	~CsmaAck();
	virtual int headerLength();
	virtual void endForce(){}
};



#endif
