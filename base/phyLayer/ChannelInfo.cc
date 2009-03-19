#include "ChannelInfo.h"

#include <iostream>
#include <assert.h>

void ChannelInfo::addAirFrame(AirFrame* a, simtime_t startTime) {

	//calculate endTime of AirFrame
	simtime_t endTime = startTime + a->getDuration();

	//add AirFrame to active AirFrames
	activeAirFrames[endTime].push_back(AirFrameTimePair(startTime, a));

	//add to start time map
	airFrameStarts[a] = startTime;
}

simtime_t ChannelInfo::removeAirFrame(AirFrame* a) {

	//get start of AirFrame
	simtime_t startTime = airFrameStarts[a];

	//calculate end time
	simtime_t endTime = startTime + a->getDuration();

	//remove this AirFrame from active AirFrames
	deleteAirFrame(activeAirFrames, a, startTime, endTime);

	//add to inactive AirFrames
	addToInactives(a, startTime, endTime);

	/*
	 * Now check, whether the earliest time-point we need to store information for
	 * might have moved on in time, since an AirFrame has been deleted.
	 *
	 * TODO: check for a more efficient way to find out that earliest time-point
	 */

	// make a variable for the earliest-start-time of all remaining AirFrames
	simtime_t earliestStart = 0;
	AirFrameStartMap::const_iterator it = airFrameStarts.begin();

	// if there is an entry for an AirFrame
	if (it != airFrameStarts.end())
	{
		// store the start-time of the first entry as earliestStart so far
		earliestStart = (*it).second;

		// go through all other start-time-points
		for (; it != airFrameStarts.end(); ++it)
		{
			// and check if an earlier start-time was found,
			// if so, replace earliestStart with it
			if( (*it).second < earliestStart )
				earliestStart = (*it).second;
		}

		// check if the current earliest-start-time is later than the stored
		// value so far, if so, replace the earliestInfoPoint
		if ( earliestStart > earliestInfoPoint )
			earliestInfoPoint = earliestStart;
	}


	//std::cerr << "remove";

	return earliestInfoPoint;
}

void ChannelInfo::deleteAirFrame(AirFrameMatrix& airFrames,
								 AirFrame* a,
								 simtime_t startTime, simtime_t endTime) {
	AirFrameMatrix::iterator listIt = airFrames.find(endTime);
	AirFrameTimeList* list = &listIt->second;

	for(AirFrameTimeList::iterator it = list->begin();
		   it != list->end(); it++) {

		if(it->second == a) {
			it = list->erase(it);
			//std::cerr << "Erased AirFrame with start " << startTime << std::endl;
			if(list->empty()) {
				airFrames.erase(listIt);
				//std::cerr << "Was last one in list." << std::endl;
			}
			return;
		}
	}
	assert(false);
}

void ChannelInfo::addToInactives(AirFrame* a, simtime_t startTime, simtime_t endTime) {

	//At first, check if some inactives can be removed because the
	//AirFrame to deactivate was the last one they intersected
	//with.

	//get through inactive AirFrame which intersected with the
	//AirFrame to deactivate (only these AirFrames could be
	//possibly deleted

	IntersectionIterator inactiveIntersectIt(&inactiveAirFrames, startTime, endTime);

	AirFrame* inactiveIntersect = inactiveIntersectIt.next();
	while(inactiveIntersect != 0) {

		simtime_t currentStart = airFrameStarts[inactiveIntersect];
		simtime_t currentEnd = currentStart + inactiveIntersect->getDuration();

		//std::cerr << "Found intersecting with to inactive " << currentStart << std::endl;

		//check if this AirFrame still intersects with at least one active AirFrame
		if(!isIntersecting(activeAirFrames, currentStart, currentEnd)) {
			inactiveIntersectIt.eraseAirFrame();

			//std::cerr << "Doesn't intersect with any active frame anymore -> delete" << std::endl;

			airFrameStarts.erase(inactiveIntersect);

			delete inactiveIntersect;
			inactiveIntersect = 0;
		}
		inactiveIntersect = inactiveIntersectIt.next();
	}

	//at last check if the AirFrame to deactivate still intersects with
	//at least one active AirFrame. If so, add it to the inactive list.
	if(isIntersecting(activeAirFrames, startTime, endTime)) {
		//std::cerr << "Removed AirFrame still intersects with at least one active." << std::endl;
		inactiveAirFrames[endTime].push_back(AirFrameTimePair(startTime, a));
	} else {
		//std::cerr << "Removed AirFrame can be removed." << std::endl;
		airFrameStarts.erase(a);

		delete a;
	}
}

bool ChannelInfo::isIntersecting(const AirFrameMatrix& airFrames,
								 simtime_t from, simtime_t to) const{

	ConstIntersectionIterator it(&airFrames, from, to);
	return (it.next() != 0);
}

void ChannelInfo::getIntersections( const AirFrameMatrix& airFrames,
									simtime_t from, simtime_t to,
									AirFrameVector& outVector) const {

	ConstIntersectionIterator it(&airFrames, from, to);

	AirFrame* intersect = it.next();
	while(intersect != 0) {
		outVector.push_back(intersect);
		intersect = it.next();
	}
}

void ChannelInfo::getAirFrames(simtime_t from, simtime_t to, AirFrameVector& out) const {
	//check for intersecting inactive AirFrames
	getIntersections(inactiveAirFrames, from, to, out);

	//check for intersecting active AirFrames
	getIntersections(activeAirFrames, from, to, out);
}


