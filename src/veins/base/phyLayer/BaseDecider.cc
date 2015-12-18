/*
 * BaseDecider.cc
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#include "veins/base/phyLayer/BaseDecider.h"

#include <cassert>

#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;

simtime_t BaseDecider::processSignal(AirFrame* frame) {

	assert(frame);
	deciderEV << "Processing AirFrame..." << endl;

	switch(getSignalState(frame)) {
	case NEW:
		return processNewSignal(frame);
	case EXPECT_HEADER:
		return processSignalHeader(frame);
	case EXPECT_END:
		return processSignalEnd(frame);
	default:
		return processUnknownSignal(frame);
	}
}

simtime_t BaseDecider::processNewSignal(AirFrame* frame) {
	if(currentSignal.first != 0) {
		deciderEV << "Already receiving another AirFrame!" << endl;
		return notAgain;
	}

	// get the receiving power of the Signal at start-time
	Signal& signal = frame->getSignal();
	double recvPower = signal.getReceivingPower()->getValue(Argument(signal.getReceptionStart()));

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

	//channel turned busy
	setChannelIdleStatus(false);

	return signal.getReceptionEnd();
}

simtime_t BaseDecider::processSignalEnd(AirFrame* frame) {
	deciderEV << "packet was received correctly, it is now handed to upper layer...\n";
	phy->sendUp(frame, new DeciderResult(true));

	// we have processed this AirFrame and we prepare to receive the next one
	currentSignal.first = 0;

	//channel is idle now
	setChannelIdleStatus(true);

	return notAgain;
}

simtime_t BaseDecider::processUnknownSignal(AirFrame* frame) {
	throw cRuntimeError("Unknown state for the AirFrame with ID %d", frame->getId());
	return notAgain;
}


ChannelState BaseDecider::getChannelState() {

	simtime_t now = phy->getSimTime();
	double rssiValue = calcChannelSenseRSSI(now, now);

	return ChannelState(isChannelIdle, rssiValue);
}


simtime_t BaseDecider::handleChannelSenseRequest(ChannelSenseRequest* request) {

	assert(request);

	if (currentChannelSenseRequest.first == 0)
	{
		return handleNewSenseRequest(request);
	}

	if (currentChannelSenseRequest.first != request) {
	    throw cRuntimeError("Got a new ChannelSenseRequest while already handling another one!");
		return notAgain;
	}

	handleSenseRequestEnd(currentChannelSenseRequest);

	// say that we don't want to have it again
	return notAgain;
}

simtime_t BaseDecider::handleNewSenseRequest(ChannelSenseRequest* request)
{
	// no request handled at the moment, handling the new one
	simtime_t now = phy->getSimTime();

	// saving the pointer to the request and its start-time (now)
	currentChannelSenseRequest.setRequest(request);
	currentChannelSenseRequest.setSenseStart(now);

	//get point in time when we can answer the request (as far as we
	//know at this point in time)
	currentChannelSenseRequest.canAnswerAt = canAnswerCSR(currentChannelSenseRequest);

	//check if we can already answer the request
	if(now == currentChannelSenseRequest.canAnswerAt)
	{
		answerCSR(currentChannelSenseRequest);
		return notAgain;
	}

	return currentChannelSenseRequest.canAnswerAt;
}

void BaseDecider::handleSenseRequestEnd(CSRInfo& requestInfo) {
	assert(canAnswerCSR(requestInfo) == phy->getSimTime());
	answerCSR(requestInfo);
}

int BaseDecider::getSignalState(AirFrame* frame) {
	if(frame == currentSignal.first)
		return currentSignal.second;

	return NEW;
}

void BaseDecider::channelStateChanged()
{
	if(!currentChannelSenseRequest.getRequest())
		return;

	//check if the point in time when we can answer the request has changed
	simtime_t canAnswerAt = canAnswerCSR(currentChannelSenseRequest);

	//check if answer time has changed
	if(canAnswerAt != currentChannelSenseRequest.canAnswerAt) {
		//can we answer it now?
		if(canAnswerAt == phy->getSimTime()) {
			phy->cancelScheduledMessage(currentChannelSenseRequest.getRequest());
			answerCSR(currentChannelSenseRequest);
		} else {
			phy->rescheduleMessage(currentChannelSenseRequest.getRequest(),
								   canAnswerAt);
			currentChannelSenseRequest.canAnswerAt = canAnswerAt;
		}
	}
}

void BaseDecider::setChannelIdleStatus(bool isIdle) {
	isChannelIdle = isIdle;

	channelStateChanged();
}

simtime_t BaseDecider::canAnswerCSR(const CSRInfo& requestInfo)
{
	assert(requestInfo.first);

	bool modeFulfilled = false;

	switch(requestInfo.first->getSenseMode())
	{
	case UNTIL_IDLE:
		modeFulfilled = isChannelIdle;
		break;
	case UNTIL_BUSY:
		modeFulfilled = !isChannelIdle;
		break;
	}

	if(modeFulfilled) {
		return phy->getSimTime();
	}

	//return point in time when time out is reached
	return requestInfo.second + requestInfo.first->getSenseTimeout();
}

double BaseDecider::calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end) {
	Mapping* rssiMap = calculateRSSIMapping(start, end);

	// the sensed RSSI-value is the maximum value between (and including) the interval-borders
	Mapping::argument_value_t rssi = MappingUtils::findMax(*rssiMap, Argument(start), Argument(end), Argument::MappedZero() /* the value if no maximum will be found */);

	delete rssiMap;
	return rssi;
}

void BaseDecider::answerCSR(CSRInfo& requestInfo) {

	double rssiValue = calcChannelSenseRSSI(requestInfo.second, phy->getSimTime());

	// put the sensing-result to the request and
	// send it to the Mac-Layer as Control-message (via Interface)
	requestInfo.first->setResult( ChannelState(isChannelIdle, rssiValue) );
	phy->sendControlMsgToMac(requestInfo.first);

	requestInfo.first = 0;
	requestInfo.second = -1;
	requestInfo.canAnswerAt = -1;
}

Mapping* BaseDecider::calculateSnrMapping(AirFrame* frame)
{
	/* calculate Noise-Strength-Mapping */
	Signal& signal = frame->getSignal();

	simtime_t start = signal.getReceptionStart();
	simtime_t end   = signal.getReceptionEnd();

	Mapping* noiseMap = calculateRSSIMapping(start, end, frame);
	assert(noiseMap);
	ConstMapping* recvPowerMap = signal.getReceivingPower();
	assert(recvPowerMap);

	//TODO: handle noise of zero (must not devide with zero!)
	Mapping* snrMap = MappingUtils::divide( *recvPowerMap, *noiseMap, Argument::MappedZero() );

	delete noiseMap;
	noiseMap = 0;

	return snrMap;
}

void BaseDecider::getChannelInfo(simtime_t_cref start, simtime_t_cref end,
                                 AirFrameVector& out)
{
	phy->getChannelInfo(start, end, out);
}

Mapping* BaseDecider::calculateRSSIMapping( simtime_t_cref start,
                                            simtime_t_cref end,
                                            AirFrame*      exclude)
{
	if(exclude)
		deciderEV << "Creating RSSI map excluding AirFrame with id " << exclude->getId() << endl;
	else
		deciderEV << "Creating RSSI map." << endl;

	AirFrameVector airFrames;

	// collect all AirFrames that intersect with [start, end]
	getChannelInfo(start, end, airFrames);

	//TODO: create a "MappingUtils:createMappingFrom()"-method and use it here instead
	//of abusing the add method
	// create an empty mapping
	Mapping* resultMap = MappingUtils::createMapping(Argument::MappedZero(), DimensionSet::timeDomain());

	// iterate over all AirFrames (except exclude)
	// and sum up their receiving-power-mappings
	for (AirFrameVector::const_iterator it = airFrames.begin(); it != airFrames.end(); ++it) {
		// the vector should not contain pointers to 0
		assert (*it != 0);

		// if iterator points to exclude (that includes the default-case 'exclude == 0')
		// then skip this AirFrame
		if ( *it == exclude ) {
			continue;
		}

		// otherwise get the Signal and its receiving-power-mapping
		Signal& signal = (*it)->getSignal();

		// backup pointer to result map
		// Mapping* resultMapOld = resultMap;

		// TODO1.1: for testing purposes, for now we don't specify an interval
		// and add the Signal's receiving-power-mapping to resultMap in [start, end],
		// the operation Mapping::add returns a pointer to a new Mapping

		const ConstMapping *const recvPowerMap = signal.getReceivingPower();
		assert(recvPowerMap);

		// Mapping* resultMapNew = Mapping::add( *(signal.getReceivingPower()), *resultMap, start, end );

		deciderEV << "Adding mapping of Airframe with ID " << (*it)->getId()
				<< ". Starts at " << signal.getReceptionStart()
				<< " and ends at " << signal.getReceptionEnd() << endl;

		Mapping* resultMapNew = MappingUtils::add( *recvPowerMap, *resultMap, Argument::MappedZero() );

		// discard old mapping
		delete resultMap;
		resultMap = resultMapNew;
		resultMapNew = 0;
	}

	// add thermal noise
	ConstMapping* thermalNoise = phy->getThermalNoise(start, end);
	if (thermalNoise) {
		// FIXME: workaround needed to make *really* sure that the resultMap is defined for the range of the exclude-frame
		const ConstMapping* excludePwr = exclude ? exclude->getSignal().getReceivingPower() : 0;
		if (excludePwr) {
			Mapping* p1 = resultMap;
			// p2 = exclude + thermal
			Mapping* p2 = MappingUtils::add(*excludePwr, *thermalNoise);
			// p3 = p2 - exclude
			Mapping* p3 = MappingUtils::subtract(*p2, *excludePwr);
			// result = result + p3
			resultMap = MappingUtils::add(*resultMap, *p3);
			delete p3;
			delete p2;
			delete p1;
		}
		else {
			Mapping* p1 = resultMap;
			resultMap = MappingUtils::add(*resultMap, *thermalNoise);
			delete p1;
		}
	}
	return resultMap;
}

