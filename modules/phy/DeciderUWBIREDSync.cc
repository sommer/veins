#include "DeciderUWBIREDSync.h"


DeciderUWBIREDSync::DeciderUWBIREDSync(DeciderToPhyInterface* iface,
				PhyLayerUWBIR* _uwbiface,
				double _syncThreshold, bool _syncAlwaysSucceeds, bool _stats,
				bool _trace, double _tmin, bool alwaysFailOnDataInterference) :
					DeciderUWBIRED(iface, _uwbiface,
						_syncThreshold, _syncAlwaysSucceeds, _stats, _trace,
						alwaysFailOnDataInterference),
						tmin(_tmin){

};

bool DeciderUWBIREDSync::attemptSync(Signal* s) {
	syncVector.clear();
	// Retrieve all potentially colliding airFrames
	phy->getChannelInfo(s->getSignalStart(), s->getSignalStart()+IEEE802154A::mandatory_preambleLength,
			syncVector);
	assert(syncVector.size() != 0);

	if (syncVector.size() == 1) {
		return evaluateEnergy(s);
	}

	bool synchronized = false;
	unsigned int i = 0;
	bool search = true;
	simtime_t latestSyncStart = s->getSignalStart() + IEEE802154A::mandatory_preambleLength - tmin;
	AirFrame* af = syncVector.front();
	Signal & aSignal = af->getSignal();

	while(search &&
			!(aSignal.getSignalStart() == s->getSignalStart() &&
					aSignal.getSignalLength() == s->getSignalLength())) {
		if(aSignal.getSignalStart()+aSignal.getSignalLength() > latestSyncStart) {
			// CASE: the end of one of the previous signals goes too far
			// and prevents synchronizing on the current frame.
			search = false;
			break;
		}
		i++;
		af = syncVector.at(i);
		aSignal = af->getSignal();
	}

	if(search && i < syncVector.size()) {
		// sync is possible but there is a frame beginning after our sync start
		Signal & nextSignal = syncVector.at(i)->getSignal();
		if(nextSignal.getSignalStart() <
				aSignal.getSignalStart()+aSignal.getSignalLength() + tmin) {
			// CASE: sync is not possible because next frame starts too early
			search = false;
		}
	}

	if(search) {
		// the signal is long enough. Now evaluate its energy
		synchronized = evaluateEnergy(s);
	}

	return synchronized;
};

bool DeciderUWBIREDSync::evaluateEnergy(Signal* s) {
	// Assumption: channel coherence time > signal duration
	// Thus we can simply sample the first pulse of the received signal
	ConstMapping* rxPower = s->getReceivingPower();
	argSync.setTime(s->getSignalStart() + IEEE802154A::tFirstSyncPulseMax);
	// We could retrieve the pathloss through s->getAttenuation() but we must be careful:
	// maybe the pathloss is not the only analogue model (e.g. RSAMAnalogueModel)
	// If we get the pathloss, we can compute Eb/N0: Eb=1E-3*pathloss if we are at peak power
	double signalPower = sqrt(pow(rxPower->getValue(argSync), 2)*0.5*peakPulsePower / 10); // de-normalize, take half, and 10 dB losses
	double noisePower = pow(getNoiseValue(), 2)/ 50;
	if(signalPower / noisePower > syncThreshold) {
		return true;
	}
	return false;
};

DeciderUWBIREDSync::~DeciderUWBIREDSync() {
	syncVector.clear();
};



