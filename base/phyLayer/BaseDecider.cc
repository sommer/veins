#include "BaseDecider.h"


#define debugEV (ev.isDisabled()||!debug) ? ev : ev << "[Host " << myIndex << "] - PhyLayer(Decider): "


/**
 * @brief Calculates a SNR-Mapping for a Signal.
 *
 * Therefore a Noise-Strength-Mapping is calculated for the time-interval
 * of the Signal and the Signal-Strength-Mapping is divided by the
 * Noise-Strength-Mapping.
 *
 * Note: 'divided' means here the special element-wise operation on mappings.
 *
 */
Mapping* BaseDecider::calculateSnrMapping(AirFrame* frame)
{
	/* calculate Noise-Strength-Mapping */
	Signal& signal = frame->getSignal();

	simtime_t start = signal.getSignalStart();
	simtime_t end = start + signal.getSignalLength();

	Mapping* noiseMap = calculateRSSIMapping(start, end, frame);
	assert(noiseMap);
	ConstMapping* recvPowerMap = signal.getReceivingPower();
	assert(recvPowerMap);

	Mapping* snrMap = MappingUtils::divide( *recvPowerMap, *noiseMap, 0.0 );

	delete noiseMap;
	noiseMap = 0;

	// TODO: so far recvPowerMap is a new instance of Mapping, that's why normally it should be
	// deleted here, but in the future signal.getReceivingPower() will return a pointer to a
	// shared instance, that is why we omit delete already to be future-proof

	// delete recvPowerMap;
	// recvPowerMap = 0;

	return snrMap;
}

/**
 * @brief Calculates a RSSI-Mapping (or Noise-Strength-Mapping) for a Signal.
 *
 * This method can be used to calculate a RSSI-Mapping in case the parameter
 * exclude is omitted OR to calculate a Noise-Strength-Mapping in case the
 * AirFrame of the received Signal is passed as parameter exclude.
 */
Mapping* BaseDecider::calculateRSSIMapping(	simtime_t start,
											simtime_t end,
											AirFrame* exclude = 0)
{
	AirFrameVector airFrames;

	// collect all AirFrames that intersect with [start, end]
	phy->getChannelInfo(start, end, airFrames);

	// if there is no AirFrame, return an empty mapping
	if (airFrames.empty())
		return MappingUtils::createMapping(0.0, DimensionSet(Dimension::time));

	// create an empty mapping
	Mapping* resultMap = MappingUtils::createMapping(0.0, DimensionSet(Dimension::time));

	// otherwise, iterate over all AirFrames (except exclude)
	// and sum up their receiving-power-mappings
	AirFrameVector::iterator it;
	for (it = airFrames.begin(); it != airFrames.end(); it++)
	{
		// the vector should not contain pointers to 0
		assert (*it != 0);

		// if iterator points to exclude (that includes the default-case 'exclude == 0')
		// then skip this AirFrame
		if ( *it == exclude ) continue;

		// otherwise get the Signal and its receiving-power-mapping
		Signal& signal = (*it)->getSignal();

		// backup pointer to result map
		// Mapping* resultMapOld = resultMap;

		// TODO1.1: for testing purposes, for now we don't specify an interval
		// and add the Signal's receiving-power-mapping to resultMap in [start, end],
		// the operation Mapping::add returns a pointer to a new Mapping

		ConstMapping* recvPowerMap = signal.getReceivingPower();
		assert(recvPowerMap);

		// Mapping* resultMapNew = Mapping::add( *(signal.getReceivingPower()), *resultMap, start, end );

		Mapping* resultMapNew = MappingUtils::add( *recvPowerMap, *resultMap, 0.0 );

		// discard old mapping
		delete resultMap;
		resultMap = resultMapNew;

		// TODO: so far recvPowerMap is a new instance of Mapping, that's why normally it should be
		// deleted here, but in the future signal.getReceivingPower() will return a pointer to a
		// shared instance, that is why we omit delete already to be future-proof

		// delete recvPowerMap;
		// recvPowerMap = 0;
	}

	return resultMap;
}

// TODO: for now we check a larger mapping within an interval
/**
 * @brief Checks a mapping against a specific threshold (element-wise).
 *
 * @return	false	, if there exists an entry smaller than threshold
 * 			true	, otherwise
 *
 *
 */
bool BaseDecider::checkIfAboveThreshold(Mapping* map, simtime_t start, simtime_t end)
{
	assert(map);

	if(debug){
		debugEV << "Checking if SNR is above Threshold of " << snrThreshold << endl;
		debugEV << "SNR at time " << start << " is " << map->getValue(Argument(start)) << endl;
	}
	// check if values at start- and end-time (interval-borders) fulfill snrThreshold-criterion
	// TODO: remove later when these interval-borders are always key-entries in the mapping
	if ( map->getValue(Argument(start)) <= snrThreshold )
		return false;

	// check every entry in the mapping against threshold value
	ConstMappingIterator* it = map->createConstIterator();

	// the mapping should not be empty
	assert(it->inRange());

	while ( it->inRange() )
	{
		// TODO: remove later when not needed anymore
		simtime_t timePos = it->getPosition().getTime();

		if(debug){
			debugEV << "SNR at time " << timePos << " is " << it->getValue() << endl;
		}

		// TODO: remove the interval-criterion later
		// perform the check for smaller entry
		if ( it->getValue() <= snrThreshold
				&& start <= timePos
				&& timePos <= end) return false;

		// if there is no next entry, end of mapping reached
		if ( !(it->hasNext()) ) break;
		it->next();
	}

	if(debug){
		debugEV << "SNR at time " << end << " is " << map->getValue(Argument(end)) << endl;
	}

	if ( map->getValue(Argument(end)) <= snrThreshold )
		return false;

	return true;
}

// TODO: check implementation
/**
 * @brief This function processes a AirFrame given by the PhyLayer and
 * returns the time point when BaseDecider wants to be given the AirFrame again.
 */
simtime_t BaseDecider::processSignal(AirFrame* frame)
{
	assert(frame);

	debugEV << "Processing AirFrame..." << endl;

	// check whether we are already receiving
	if (currentAirFrame == 0)
	{
		// we are ready for receiving a new Signal, this is the first time
		// we are handed over this AirFrame

		// call handling-method
		return handleNewSignal(frame);
	}
	else if (currentAirFrame == frame)
	{
		// we are currently receiving exactly this AirFrame, so this is the second
		// time we are handed this AirFrame and it is the end-time of the Signal now

		// call handling-method
		return handleSignalOver(frame);
	}

	// we could not start to receive this AirFrame and we have not been receiving it
	// so we just do nothing
	debugEV << "Already receiving another AirFrame!" << endl;

	return notAgain;
}

/**
 * TODO: check implementation
 *
 */
simtime_t BaseDecider::handleNewSignal(AirFrame* frame)
{
	// extract Signal from AirFrame
	Signal& signal = frame->getSignal();

	// get the receiving power of the Signal at start-time
	double recvPower = signal.getReceivingPower()->getValue(Argument(signal.getSignalStart()));

	// check whether signal is strong enough to receive
	if ( recvPower < sensitivity )
	{
		debugEV << "Signal is to weak (" << recvPower << " < " << sensitivity << ") -> do not receive." << endl;
		// Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
		return notAgain;
	}

	// Signal is strong enough, receive this Signal and schedule it
	currentAirFrame = frame;
	debugEV << "Trying to receive AirFrame." << endl;
	return ( signal.getSignalStart() + signal.getSignalLength() );
}

/**
 * TODO: check implementation
 *
 */
simtime_t BaseDecider::handleSignalOver(AirFrame* frame)
{
	// here the Signal is finally processed

	// first collect all necessary information
	Mapping* snrMap = calculateSnrMapping(frame);
	assert(snrMap);

	//TODO: this extraction is just temporary, since we need to pass Signal's start-
	// and end-time to the following method
	const Signal& signal = frame->getSignal();
	simtime_t start = signal.getSignalStart();
	simtime_t end = start + signal.getSignalLength();

	bool aboveThreshold = checkIfAboveThreshold(snrMap, start, end);

	// check if the snrMapping is above the Decider's specific threshold,
	// i.e. the Decider has received it correctly
	if (aboveThreshold)
	{
		debugEV << "SNR is above threshold("<<snrThreshold<<") -> sending up." << endl;
		// go on with processing this AirFrame, send it to the Mac-Layer
		phy->sendUp(frame, DeciderResult(true));
	} else
	{
		debugEV << "SNR is below threshold("<<snrThreshold<<") -> dropped." << endl;
	}

	delete snrMap;
	snrMap = 0;


	// we're through with this AirFrame and we prepare to receive the next one
	currentAirFrame = 0;
	return notAgain;
}

// TODO: check implementation
/**
 * @brief A function that returns information about the channel state
 *
 * It is an alternative for the MACLayer in order to obtain information
 * immediately (in contrast to sending a ChannelSenseRequest,
 * i.e. sending a cMessage over the OMNeT-control-channel)
 */
ChannelState BaseDecider::getChannelState()
{
	simtime_t now = phy->getSimTime();
	Mapping* rssiMap = calculateRSSIMapping(now, now);

	double rssiValue = rssiMap->getValue(Argument(now));

	delete rssiMap;
	rssiMap = 0;

	return ChannelState(currentlyIdle(), rssiValue);
}

// TODO: implement
/**
 * @brief This function is called by the PhyLayer to hand over a
 * ChannelSenseRequest.
 *
 * The MACLayer is able to send a ChannelSenseRequest to the PhyLayer
 * that calls this function with it and is returned a time point when to
 * re-call this function with the specific ChannelSenseRequest.
 *
 * The BaseDecider puts the result (ChannelState) to the ChannelSenseRequest
 * and "answers" by calling the "sendControlMsg"-function on the
 * DeciderToPhyInterface, i.e. telling the PhyLayer to send it back.
 */
simtime_t BaseDecider::handleChannelSenseRequest(ChannelSenseRequest* request)
{
	assert(request);

	if (currentChannelSenseRequest.first == 0)
	{
		// no request handled at the moment, handling the new one
		simtime_t now = phy->getSimTime();

		// saving the pointer to the request and its start-time (now)
		currentChannelSenseRequest.first = request;
		currentChannelSenseRequest.second = now;

		return ( now + request->getSenseDuration() );

	}
	else if (currentChannelSenseRequest.first == request)
	{
		// we are handling this exactly this request at the moment,
		// so sensing-interval should be over now
		simtime_t start = currentChannelSenseRequest.second;
		simtime_t end = start + request->getSenseDuration();

		Mapping* rssiMap = calculateRSSIMapping(start, end);

		// TODO: better: iterate over whole interval [start, end] in rssiMap
		// and find the maximum value

		// the sensed RSSI-value is the maximum value of the interval-borders
		double rssiValue = std::max(
				rssiMap->getValue(Argument(start)),
				rssiMap->getValue(Argument(end))
				);

		delete rssiMap;
		rssiMap = 0;

		// put the sensing-result to the request and
		// send it to the Mac-Layer as Control-message (via Interface)
		request->setResult( ChannelState(currentlyIdle(), rssiValue) );
		phy->sendControlMsg(request);

		// reset currently handled request
		resetChannelSenseRequest();

		// say that we don't want to have it again
		return notAgain;
	}

	// we have been sensing the channel due to a request,
	// but we are now handling the new request

	// throw warning
	log("WARNING: ChannelSenseRequest arrived while already handling another one!");

	// then handle the new request
	simtime_t now = phy->getSimTime();

	// saving the pointer to the request and its start-time (now)
	currentChannelSenseRequest.first = request;
	currentChannelSenseRequest.second = now;

	return ( now + request->getSenseDuration() );
}

