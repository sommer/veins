#include "veins/modules/phy/SNRThresholdDecider.h"

#include <cassert>

#include "veins/base/messages/AirFrame_m.h"
#include "veins/base/phyLayer/Mapping.h"

using Veins::AirFrame;

simtime_t SNRThresholdDecider::processNewSignal(AirFrame* frame)
{
	//the rssi level changes therefore we need to check if we can
	//answer an ongoing ChannelSenseRequest now
	channelStateChanged();

	if(currentSignal.first != 0) {
		deciderEV << "Already receiving another AirFrame!" << endl;
		return notAgain;
	}

	//get the receiving power of the Signal
	//Note: We assume the transmission power is represented by a rectangular function
	//which discontinuities (at start and end of the signal) are represented
	//by two key entries with different values very close to each other (see
	//MappingUtils "addDiscontinuity" method for details). This means
	//the transmission- and therefore also the receiving-power-mapping is still zero
	//at the exact start of the signal and not till one time step after the start its
	//at its actual transmission(/receiving) power.
	//Therefore we use MappingUtils "post"-method to ask for the receiving power
	//at the correct position.
	Signal& signal = frame->getSignal();
	simtime_t receivingStart = MappingUtils::post(signal.getReceptionStart());
	double recvPower = signal.getReceivingPower()->getValue(Argument(receivingStart));

	// check whether signal is strong enough to receive
	if ( recvPower < sensitivity )
	{
		deciderEV << "Signal is to weak (" << recvPower << " < " << sensitivity
				<< ") -> do not receive." << endl;
		// Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
		return notAgain;
	}

	// Signal is strong enough, receive this Signal and schedule it
	deciderEV << "Signal is strong enough (" << recvPower << " > " << sensitivity
			<< ") -> Trying to receive AirFrame." << endl;

	currentSignal.first = frame;
	currentSignal.second = EXPECT_END;

	return signal.getReceptionEnd();
}

// TODO: for now we check a larger mapping within an interval
bool SNRThresholdDecider::checkIfAboveThreshold(Mapping* map, simtime_t_cref start, simtime_t_cref end)
{
	assert(map);

	if(debug){
		deciderEV << "Checking if SNR is above Threshold of " << snrThreshold << endl;
	}

	// check every entry in the mapping against threshold value
	ConstMappingIterator* it = map->createConstIterator(Argument(start));
	// check if values at start-time fulfill snrThreshold-criterion
	if(debug){
		deciderEV << "SNR at time " << start << " is " << it->getValue() << endl;
	}
	if ( it->getValue() <= snrThreshold ){
		delete it;
		return false;
	}

	while ( it->hasNext() && it->getNextPosition().getTime() < end)
	{
		it->next();

		if(debug){
			deciderEV << "SNR at time " << it->getPosition().getTime() << " is " << it->getValue() << endl;
		}

		// perform the check for smaller entry
		if ( it->getValue() <= snrThreshold) {
			delete it;
			return false;
		}
	}

	it->iterateTo(Argument(end));
	if(debug){
		deciderEV << "SNR at time " << end << " is " << it->getValue() << endl;
	}

	if ( it->getValue() <= snrThreshold ){
		delete it;
		return false;
	}

	delete it;
	return true;
}

ChannelState SNRThresholdDecider::getChannelState() {

	simtime_t now = phy->getSimTime();
	double rssiValue = calcChannelSenseRSSI(now, now);

	return ChannelState(isIdleRSSI(rssiValue), rssiValue);
}

void SNRThresholdDecider::answerCSR(CSRInfo& requestInfo)
{
	// put the sensing-result to the request and
	// send it to the Mac-Layer as Control-message (via Interface)
	requestInfo.first->setResult( getChannelState() );
	phy->sendControlMsgToMac(requestInfo.first);

	requestInfo.first = 0;
	requestInfo.second = -1;
	requestInfo.canAnswerAt = -1;
}

simtime_t SNRThresholdDecider::canAnswerCSR(const CSRInfo& requestInfo) {
	const ChannelSenseRequest* request = requestInfo.getRequest();
	assert(request);

	if(request->getSenseMode() == UNTIL_TIMEOUT) {
		throw cRuntimeError("SNRThresholdDecider received an UNTIL_TIMEOUT ChannelSenseRequest.\n"
				  "SNRThresholdDecider can only handle UNTIL_IDLE or UNTIL_BUSY requests because it "
				  "implements only instantaneous sensing where UNTIL_TIMEOUT requests "
				  "don't make sense. Please refer to ChannelSenseRequests documentation "
				  "for details.");
	}

	simtime_t requestTimeout = requestInfo.getSenseStart() + request->getSenseTimeout();

	simtime_t now = phy->getSimTime();

	ConstMapping* rssiMapping = calculateRSSIMapping(now, requestTimeout);

	//this Decider only works for time-only signals
	assert(rssiMapping->getDimensionSet() == DimensionSet::timeDomain());

	ConstMappingIterator* it = rssiMapping->createConstIterator(Argument(now));

	assert(request->getSenseMode() == UNTIL_IDLE
		   || request->getSenseMode() == UNTIL_BUSY);
	bool untilIdle = request->getSenseMode() == UNTIL_IDLE;

	simtime_t answerTime = requestTimeout;
	//check if the current rssi value enables us to answer the request
	if(isIdleRSSI(it->getValue()) == untilIdle) {
		answerTime = now;
	}
	else {
		//iterate through request interval to check when the rssi level
		//changes such that the request can be answered
		while(it->hasNext() && it->getNextPosition().getTime() < requestTimeout) {
			it->next();

			if(isIdleRSSI(it->getValue()) == untilIdle) {
				answerTime = it->getPosition().getTime();
				break;
			}
		}
	}

	delete it;
	delete rssiMapping;

	return answerTime;
}

simtime_t SNRThresholdDecider::processSignalEnd(AirFrame* frame)
{
	assert(frame == currentSignal.first);
	// here the Signal is finally processed

	// first collect all necessary information
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	const Signal& signal = frame->getSignal();
	simtime_t start = signal.getReceptionStart();
	simtime_t end = signal.getReceptionEnd();

	// NOTE: Since this decider does not consider the amount of time when the signal's SNR is
	// below the threshold even the smallest (normally insignificant) drop causes this decider
	// to reject reception of the signal.
	// Since the default MiXiM-signal is still zero at its exact start and end, these points
	// are ignored in the interval passed to the following method.
	bool aboveThreshold = checkIfAboveThreshold(snrMap,
												MappingUtils::post(start),
												MappingUtils::pre(end));

	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly
	if (aboveThreshold)
	{
		deciderEV << "SNR is above threshold("<<snrThreshold<<") -> sending up." << endl;
		// go on with processing this AirFrame, send it to the Mac-Layer
		phy->sendUp(frame, new DeciderResult(true));
	} else
	{
		deciderEV << "SNR is below threshold("<<snrThreshold<<") -> dropped." << endl;
	}

	delete snrMap;
	snrMap = 0;


	// we have processed this AirFrame and we prepare to receive the next one
	currentSignal.first = 0;

	return notAgain;
}

