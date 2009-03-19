#include "Decider.h"


bool DeciderResult::isSignalCorrect() const {
	return isCorrect;
}

Decider::Decider(DeciderToPhyInterface* phy):
	phy(phy), notAgain(-1) {}

simtime_t Decider::processSignal(AirFrame* s) {

	//TODO: implement
	return -1;
}

ChannelState Decider::getChannelState() {

	//TODO: implement
	return ChannelState();
}

simtime_t Decider::handleChannelSenseRequest(ChannelSenseRequest* request) {

	//TODO: implement
	return -1;
}
