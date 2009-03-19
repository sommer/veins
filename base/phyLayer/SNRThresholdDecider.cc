#include "SNRThresholdDecider.h"

// TODO: for now we check a larger mapping within an interval
bool SNRThresholdDecider::checkIfAboveThreshold(Mapping* map, simtime_t start, simtime_t end)
{
	assert(map);

	if(debug){
		debugEV << "Checking if SNR is above Threshold of " << snrThreshold << endl;
	}

	// check every entry in the mapping against threshold value
	ConstMappingIterator* it = map->createConstIterator(Argument(start));
	// check if values at start-time fulfill snrThreshold-criterion
	if(debug){
		debugEV << "SNR at time " << start << " is " << it->getValue() << endl;
	}
	if ( it->getValue() <= snrThreshold ){
		delete it;
		return false;
	}

	while ( it->hasNext() && it->getNextPosition().getTime() < end)
	{
		it->next();

		if(debug){
			debugEV << "SNR at time " << it->getPosition().getTime() << " is " << it->getValue() << endl;
		}

		// perform the check for smaller entry
		if ( it->getValue() <= snrThreshold) {
			delete it;
			return false;
		}
	}

	it->iterateTo(Argument(end));
	if(debug){
		debugEV << "SNR at time " << end << " is " << it->getValue() << endl;
	}

	if ( it->getValue() <= snrThreshold ){
		delete it;
		return false;
	}

	delete it;
	return true;
}

simtime_t SNRThresholdDecider::processSignalEnd(AirFrame* frame)
{
	assert(frame == currentSignal.first);
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
		phy->sendUp(frame, new DeciderResult(true));
	} else
	{
		debugEV << "SNR is below threshold("<<snrThreshold<<") -> dropped." << endl;
	}

	delete snrMap;
	snrMap = 0;


	// we have processed this AirFrame and we prepare to receive the next one
	currentSignal.first = 0;

	//channel turned idle
	setChannelIdleStatus(true);

	return notAgain;
}

