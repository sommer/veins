#ifndef TESTDECIDER_H_
#define TESTDECIDER_H_

#include <Decider.h>
#include <TestModule.h>
#include <BasePhyLayer.h>
#include "TestGlobals.h"

class TestDecider:public Decider, public TestModule {
protected:
	int myIndex;
	int run;
	const int RECEIVING_STATE;
	
	bool state;
	double rssi;
	
protected:
	void assertMessage(std::string msg, int state, AirFrame* frame, simtime_t arrival, std::string dest = "") {
		TestModule::assertMessage(new AssertAirFrame(msg, state, arrival, frame), dest);
	}
	
public:
	TestDecider(DeciderToPhyInterface* phy, int index, int run, int RECEIVING_STATE):
		Decider(phy), myIndex(index), run(run), RECEIVING_STATE(RECEIVING_STATE), state(false), rssi(0.0) {
		
		init("decider" + toString(myIndex));
	}
	
	ChannelState getChannelState() {
		state = !state;
		rssi += 1.0;
		return ChannelState(state, rssi);
	}
	
	simtime_t handleChannelSenseRequest(ChannelSenseRequest* request) {
		announceMessage(request);
		
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
			assertMessage("Scheduled AirFrame to Pseudoheader at phy.", RECEIVING_STATE, frame, headerEnd, "phy" + toString(myIndex));
			assertMessage("Scheduled AirFrame to Pseudoheader.", RECEIVING_STATE, frame, headerEnd);
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
