#include "AggrPkt.h"

Register_Class(AggrPkt);

void AggrPkt::setStoredPacketsArraySize(unsigned int size) { }

unsigned int AggrPkt::getStoredPacketsArraySize() const {
	return storedPackets.size();
}

// should not be used -- implemented only because omnet++ expects it
pApplPkt& AggrPkt::getStoredPackets(unsigned int k) {
	pApplPkt& pkt = storedPackets.front();
	return pkt ;
}

// should not be used -- implemented only because omnet++ expects it
void AggrPkt::setStoredPackets(unsigned int k, const pApplPkt& storedPackets_var) {

}

// use these functions instead

void AggrPkt::storePacket(pApplPkt& msg) {
	take(msg); // update ownership
	storedPackets.push_back(msg);
}

pApplPkt& AggrPkt::popFrontPacket() {
	pApplPkt& pkt = storedPackets.front();
	storedPackets.pop_front();
	drop(pkt); // update ownership
	return pkt;
}

bool AggrPkt::isEmpty() {
	return storedPackets.size() == 0;
}
