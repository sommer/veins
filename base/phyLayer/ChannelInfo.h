#ifndef CHANNELINFO_H_
#define CHANNELINFO_H_

#include <vector>
#include <omnetpp.h>
#include "AirFrame_m.h"

/**
 * @brief This class is used by the BasePhyLayer to keep track
 * of the AirFrames on the channel.
 *
 * ChannelInfo is able to return every AirFrame which
 * intersects with a specified interval. This
 * is mainly used to get the noise for a received signal.
 *
 * ChannelInfo is a passive class meaning the user has to
 * tell it when a new AirFrame starts and an existing ends.
 *
 * Once an AirFrame has been added to the ChannelInfo the
 * ChannelInfo holds the ownership of this AirFrame even if
 * the AirFrame is removed again from the ChannelInfo.
 * This is necessary because the ChannelInfo has to be
 * able to store also the AirFrames which are over but still
 * intersect with an currently running AirFrame.
 *
 * Note: ChannelInfo assumes that the AirFrames are added
 * 		 and removed chronologically. This means every time
 * 		 you add an AirFrame with a specific start time
 * 		 ChannelInfo assumes that start time as the current
 * 		 time and assumes that every following action happens
 * 		 after that moment. The same goes for "removeAirFrame".
 * 		 When removing an AirFrames, ChannelInfo assumes
 * 		 the start time plus the duration of the AirFrame as
 * 		 the current time.
 * 		 This also affects "getAirFrames" in the way that you
 * 		 may only ask for intervals which lie before the
 * 		 "current time" of ChannelInfo.
 *
 * @ingroup phyLayer
 */
class ChannelInfo {

protected:


	typedef std::pair<simtime_t, AirFrame*> AirFrameTimePair;
	typedef std::list<AirFrameTimePair> AirFrameTimeList;
	/**
	 * The AirFrames are stored in a Matrix with start- and end time
	 * as dimensions.
	 */
	typedef std::map<simtime_t, AirFrameTimeList > AirFrameMatrix;

	/**
	 * @brief Iterator for every intersection of
	 * a specific interval in a AirFrameMatrix.
	 */
	template<class C, class ItMatrix, class ItList>
	class BaseIntersectionIterator {
	public:
		C* intervals;
		simtime_t from;
		simtime_t to;
		ItMatrix endIt;
		ItList startIt;
		bool alreadyNext;

	public:

		/**
		 * @brief Creates an iterator for the specified interval at the
		 * specified AirFrameMatrix.
		 */
		BaseIntersectionIterator(C* airFrames, simtime_t from, simtime_t to) :
			intervals(airFrames), from(from), to(to) {
			endIt = intervals->lower_bound(from);

			if(endIt != intervals->end()) {
				startIt = endIt->second.begin();
			}
			alreadyNext = true;
		}

		/**
		 * @brief Increases the iterator to the next intersecting AirFrame
		 * and returns a pointer to this AirFrame.
		 */
		AirFrame* next() {
			if(endIt == intervals->end())
				return 0;

			//"alreadyNext" indicates that some previous iterator
			//function has already increased the intern iterators
			//to a yet unchecked values. This happen after
			//initialisation of the iterator and when an element
			//is erased.
			if(alreadyNext)
				alreadyNext = false;
			else
				startIt++;

			while(endIt != intervals->end()) {
				while(startIt != endIt->second.end()) {
					if(startIt->first <= to) {
						return startIt->second;
					}
					startIt++;
				}
				endIt++;
				if(endIt == intervals->end())
					return 0;
				startIt = endIt->second.begin();
			}
			return 0;
		}
	};

	typedef BaseIntersectionIterator<const AirFrameMatrix,
									 AirFrameMatrix::const_iterator,
									 AirFrameTimeList::const_iterator> ConstIntersectionIterator;

	class IntersectionIterator: public BaseIntersectionIterator<AirFrameMatrix,
																AirFrameMatrix::iterator,
																AirFrameTimeList::iterator> {
	public:
		IntersectionIterator(AirFrameMatrix* airFrames, simtime_t from, simtime_t to) :
			BaseIntersectionIterator<AirFrameMatrix,
									 AirFrameMatrix::iterator,
									 AirFrameTimeList::iterator>(airFrames, from, to) {}

		/**
		 * @brief Erases the AirFrame the iterator currently points to
		 * from the AirFrameMatrix.
		 *
		 * After the erase the iterator points to an invalid value
		 * and "next()" should be called.
		 */
		void eraseAirFrame() {
			//check if our iterator points to a valid entry
			if(endIt == intervals->end())
				return;

			//erase AirFrame from list
			startIt = endIt->second.erase(startIt);

			//check if we've deleted the last entry in the list
			if(startIt == endIt->second.end()) {
				//check if we deleted the only entry in the list
				if(endIt->second.empty())
					intervals->erase(endIt++); //delete list from map
				else
					endIt++;

				//increase to a valid value if we are not done
				if(endIt != intervals->end())
					startIt = endIt->second.begin();
			}
			alreadyNext = true;
		}
	};

	/**
	 * @brief Stores the currently active AirFrames.
	 *
	 * This means every AirFrame which was added but not yet removed.
	 */
	AirFrameMatrix activeAirFrames;

	/**
	 * @brief Stores inactive AirFrames.
	 *
	 * This means every AirFrame which has been already removed but still
	 * is needed because it intersect with one or more active AirFrames.
	 */
	AirFrameMatrix inactiveAirFrames;

	typedef std::map<AirFrame*, simtime_t> AirFrameStartMap;

	/**
	 * @brief Stores the start time of every AirFrame.
	 */
	AirFrameStartMap airFrameStarts;

	/**
	 * @brief Stores the earliest time-point we need to keep information for.
	 */
	simtime_t earliestInfoPoint;


public:
	typedef std::vector<AirFrame*> AirFrameVector;

protected:


	/**
	 * @brief Returns every AirFrame of an AirFrameMatrix which
	 * intersect with a given interval.
	 *
	 * The intersecting AirFrames are stored in the
	 * AirFrameVector reference passed as parameter.
	 */
	void getIntersections( const AirFrameMatrix& airFrames,
						   simtime_t from, simtime_t to,
						   AirFrameVector& outVector) const;

	/**
	 * @brief Returns true if there is at least one AirFrame in the
	 * passed AirFrameMatrix which intersect with the given
	 * interval.
	 */
	bool isIntersecting( const AirFrameMatrix& airFrames,
						 simtime_t from, simtime_t to) const;

	/**
	 * @brief Moves an previously active AirFrame to the inactives.
	 *
	 * This methods checks if there are some inactive AirFrames
	 * which can be deleted because the AirFrame to inactivate
	 * was the last one they intersected with.
	 * It also checks if the AirFrame do inactivate still
	 * intersect with at least one active AirFrame before
	 * it is moved to inactived AirFrames.
	 */
	void addToInactives(AirFrame* a, simtime_t startTime, simtime_t endTime);

	/*
	 * @brief Deletes an AirFrame from an AirFrameMatrix.
	 */
	void deleteAirFrame(AirFrameMatrix& airFrames,
			 			AirFrame* a,
			 			simtime_t startTime, simtime_t endTime);

public:

	/**
	 * @brief Tells the ChannelInfo that an AirFrame has started.
	 *
	 * From this point ChannelInfo gets the ownership of the
	 * AirFrame.
	 *
	 * parameter startTime holds the time the receiving of
	 * the AirFrame has started in seconds.
	 */
	void addAirFrame(AirFrame* a, simtime_t startTime);

	/**
	 * @brief Tells the ChannelInfo that an AirFrame is over.
	 *
	 * This does not mean that in looses ownership of the AirFrame.
	 *
	 * @return  The current time-point from that information concerning AirFrames
	 * is needed to be stored.
	 */
	simtime_t removeAirFrame(AirFrame* a);

	/**
	 * @brief Fills the passed AirFrameVector reference with the AirFrames
	 * which intersect with the given time interval.
	 *
	 * Note: Completeness of the list of AirFrames for specific
	 * interval can only be assured if start and end point of
	 * the interval lies inside the duration of at least one
	 * currently active AirFrame.
	 * An AirFrame is called active if it has been added but
	 * not yet removed from ChannelInfo.
	 */
	void getAirFrames(simtime_t from, simtime_t to, AirFrameVector& out) const;

	/**
	 * @brief Returns the current time-point from that information concerning AirFrames
	 * is needed to be stored.
	 */
	simtime_t getEarliestInfoPoint()
	{
		return earliestInfoPoint;
	}

};

#endif /*CHANNELINFO_H_*/
