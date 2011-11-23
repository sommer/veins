#ifndef TESTGLOBALS_H_
#define TESTGLOBALS_H_

#include <TestModule.h>
#include <AirFrame_m.h>

enum {
	TEST_MACPKT = 12121
};

class AssertAirFrame:public AssertMessage {
protected:	
	AirFrame* pointer;
	simtime_t arrival;
	int state;
public:
	AssertAirFrame(	std::string msg, int state,
					simtime_t arrival,
					AirFrame* frame = 0,
					bool continuesTests = false):
		AssertMessage(msg, false, continuesTests),
		pointer(frame), arrival(arrival), state(state)
	{}
	
	virtual ~AssertAirFrame() {}
		
	/**
	 * Returns true if the passed message is the message
	 * expected by this AssertMessage.
	 * Has to be implemented by every subclass.
	 */
	virtual bool isMessage(cMessage* msg) {
		AirFrame* frame = dynamic_cast<AirFrame*>(msg);
		return frame != 0 && (frame == pointer || pointer == 0) && arrival == msg->getArrivalTime() &&frame->getState() == state;
	}
};

#endif /*TESTGLOBALS_H_*/
