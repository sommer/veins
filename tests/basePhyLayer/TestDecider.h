#ifndef TESTDECIDER_H_
#define TESTDECIDER_H_

#include <Decider.h>
#include <TestModule.h>
#include <BasePhyLayer.h>
#include "TestGlobals.h"

class AssertAirFrame:public AssertMessage {
protected:
	AirFrame* pointer;
	simtime_t arrival;
public:
	AssertAirFrame(std::string msg, AirFrame* frame, simtime_t arrival, TestModule* cModule = 0, int cState = 0):
		AssertMessage(msg, cModule, cState), pointer(frame), arrival(arrival) {}
		
	/**
	 * Returns true if the passed message is the message
	 * expected by this AssertMessage.
	 * Has to be implemented by every subclass.
	 */
	virtual bool isMessage(cMessage* msg) {
		AirFrame* frame = dynamic_cast<AirFrame*>(msg);
		return frame == pointer && arrival == msg->arrivalTime();
	}
};

class TestDecider:public Decider, public TestModule {
protected:
	int myIndex;
	int run;
	
protected:
	void assertMessage(std::string msg, AirFrame* frame, simtime_t arrival, std::string dest = "") {
		TestModule::assertMessage(new AssertAirFrame(msg, frame, arrival), dest);
	}
	
public:
	TestDecider(DeciderToPhyInterface* phy, int index, int run):
		Decider(phy), myIndex(index), run(run) {
		
		init("decider" + toString(myIndex));
	}
	
	simtime_t handleChannelSenseRequest(ChannelSenseRequest* request) {
		simtime_t time = phy->getSimTime();
		if(request->getSenseDuration() > 0.0) {
			simtime_t next = time + request->getSenseDuration();
			TestModule::assertMessage(	"Scheduled sense request at phy.", 
										BasePhyLayer::CHANNEL_SENSE_REQUEST, 
										next,
										"phy" + toString(myIndex));
			TestModule::assertMessage(	"Scheduled sense request.", 
										BasePhyLayer::CHANNEL_SENSE_REQUEST, 
										next);
			request->setSenseDuration(0.0);
			return next;
		} else {
			TestModule::assertMessage(	"Sense request answer at mac.", 
										BasePhyLayer::CHANNEL_SENSE_REQUEST, 
										time,
										"mac" + toString(myIndex));
			request->setResult(ChannelState());
			phy->sendControlMsg(request);
			return -1.0;
		}
	}
	
	simtime_t processSignal(AirFrame* frame) {
		announceMessage(frame);
		
		const Signal& s = frame->getSignal();
		
		simtime_t time = phy->getSimTime();
		simtime_t headerEnd = s.getSignalStart() + s.getSignalLength() * 0.1;
		if(time == s.getSignalStart()) {			
			assertMessage("Scheduled AirFrame to Pseudoheader at phy.", frame, headerEnd, "phy" + toString(myIndex));
			assertMessage("Scheduled AirFrame to Pseudoheader.", frame, headerEnd);
			return headerEnd;
			
		} else if(time == headerEnd) {
			phy->sendUp(frame, DeciderResult(true));
			TestModule::assertMessage("MacPkt at mac layer.", TEST_MACPKT, 
						  time, "mac" + toString(myIndex));
			return -1;
		}
		
		fail("Simtime(" + toString(time) + ") was neither signal start (" + toString(s.getSignalStart()) + 
			 ") nor header end(" + toString(headerEnd) + ").");
		return -1;
	}
	
	~TestDecider() {
		finalize();
	}
};
#endif /*TESTDECIDER_H_*/
