#include "DeciderUWBIREDSyncOnAddress.h"

#include "DeciderUWBIRED.h"
#include "MacPkt_m.h"
#include "AirFrame_m.h"

DeciderUWBIREDSyncOnAddress::DeciderUWBIREDSyncOnAddress(DeciderToPhyInterface* iface,
				PhyLayerUWBIR* _uwbiface,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, const LAddress::L2Type& _addr, bool alwaysFailOnDataInterference) :
					DeciderUWBIRED(iface, _uwbiface,
						_syncThreshold, _syncAlwaysSucceeds, _stats, _trace, alwaysFailOnDataInterference),
						syncAddress(_addr) {

};


bool DeciderUWBIREDSyncOnAddress::attemptSync(Signal* s) {
	cMessage* encaps = currFrame->getEncapsulatedPacket();
	assert(static_cast<MacPkt*>(encaps));
	MacPkt* macPkt = static_cast<MacPkt*>(encaps);

	return (macPkt->getSrcAddr()==syncAddress);
};

simtime_t DeciderUWBIREDSyncOnAddress::processSignal(AirFrame* frame) {
	currFrame = frame;
	return DeciderUWBIRED::processSignal(frame);
};


