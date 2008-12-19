#ifndef PHYUTILS_H_
#define PHYUTILS_H_

#include "AnalogueModel.h"
#include "Mapping.h"

#include <omnetpp.h>
#include <iostream>
#include <assert.h>
#include <list>



class RSAMMapping;

/**
 * @brief This special AnalogueModel provides filtering of a Signal
 * according to the actual RadioStates the Radio were in during
 * the Signal's time interval
 *
 */
class RadioStateAnalogueModel : public AnalogueModel
{
	friend class RSAMMapping;
	friend class RSAMConstMappingIterator;

protected:

	/**
	 * @brief data structure for the list elements,
	 * consists basically of a pair of a simtime_t and a double value (simple timestamp)
	 *
	 * further functionality can be added later if needed
	 *
	 */
	class ListEntry
	{

	protected:
		std::pair<simtime_t, double> basicTimestamp;

	public:
		ListEntry(simtime_t time, double value)
		{
			basicTimestamp = std::pair<simtime_t, double> (time, value);
		}

		virtual ~ListEntry() {}

		simtime_t getTime() const
		{
			return basicTimestamp.first;
		}

		void setTime(simtime_t time)
		{
			basicTimestamp.first = time;
		}

		double getValue() const
		{
			return basicTimestamp.second;
		}

		void setValue(double value)
		{
			basicTimestamp.second = value;
		}

		/**
		 * overload of operator < for class ListEntry to be able to use the STL Algorithms
		 * "lower_bound" and "upper_bound"
		 */
		friend bool operator<(const ListEntry& e, const simtime_t& t)
		{
			return (e.getTime() < t);
		}

		friend bool operator<(const simtime_t& t, const ListEntry& e)
		{
			return (t < e.getTime());
		}


	};


	/**
	 * @brief Indicator variable whether we are currently tracking changes
	 */
	bool currentlyTracking;

	/**
	 * @brief Data structure to track when the Radio is receiving
	 */
	std::list<ListEntry> radioIsReceiving;



public:

	/**
	 * @brief Standard constructor for a RadioStateAnalogueModel instance
	 *
	 * Default setting is: tracking on
	 */
	RadioStateAnalogueModel(double initValue,
							bool _currentlyTracking = true,
							simtime_t initTime = 0)
		: currentlyTracking(_currentlyTracking)
	{
		// put the initial Timestamp to the list
		radioIsReceiving.push_back(ListEntry(initTime, initValue));
	}

	virtual ~RadioStateAnalogueModel() {}

	/**
	 * @brief Filters the Signal according to the RadioState (passively),
	 * i.e. adding an appropriate instance of RSAMMapping to the Signal
	 *
	 * The Signal is added a new RSAMMapping that has a pointer to
	 * this instance RadioStateAnalogueModel, hence the pointer is valid as long
	 * as the Radio instance exists that has this RSAM as a member.
	 */
	virtual void filterSignal(Signal& s);

	/**
	 * @brief sets tracking mode
	 */
	void setTrackingModeTo(bool b)
	{
		currentlyTracking = b;
	}

	/**
	 * @brief Cleans up all stored information strictly before the given time-point,
	 * i.e. all elements with their timepoint strictly smaller than given key. That
	 * means multiple entries with same time are preserved.
	 *
	 * Intended to be used by the PhyLayer
	 *
	 * THIS SHOULD BE THE ONLY WAY TO DELETE ENTRIES IN THE RECEIVING LIST
	 *
	 */
	void cleanUpUntil(simtime_t t)
	{
		// assert that list is not empty
		assert(!radioIsReceiving.empty());

		/* the list contains at least one element */

		// CASE: t is smaller or equal the timepoint of the first element ==> nothing to do, return
		if ( t <= radioIsReceiving.front().getTime() )
		{
			return;
		}


		// CASE: t is greater than the timepoint of the last element
		// ==> clear complete list except the last element, return
		if ( t > radioIsReceiving.back().getTime() )
		{
			ListEntry lastEntry = radioIsReceiving.back();
			radioIsReceiving.clear();
			radioIsReceiving.push_back(lastEntry);
			return;
		}

		/*
		 * preconditions from now on:
		 * 1. list contains at least two elements, since 2. + 3.
		 * 2. t > first_timepoint
		 * 3. t <= last_timepoint
		 */

		// get an iterator and set it to the first timepoint >= t
		std::list<ListEntry>::iterator it;
		it = lower_bound(radioIsReceiving.begin(), radioIsReceiving.end(), t);


		// CASE: list contains an element with exactly the given key
		if ( it->getTime() == t )
		{
			radioIsReceiving.erase(radioIsReceiving.begin(), it);
			return;
		}

		// CASE: t is "in between two elements"
		// ==> set the iterators predecessors time to t, it becomes the first element
		it--; // go back one element, possible since this one has not been the first one

		it->setTime(t); // set this elements time to t
		radioIsReceiving.erase(radioIsReceiving.begin(), it); // and erase all previous elements

	}

	/**
	 * @brief Stores an entry of the form "time-point/attenuation (from now on)"
	 *
	 * Intended to be used by the Radio
	 */
	void writeRecvEntry(simtime_t time, double value)
	{
		// bugfixed on 08.04.2008
		assert( (radioIsReceiving.empty()) || (time >= radioIsReceiving.back().getTime()) );

		if (currentlyTracking)
		{
			radioIsReceiving.push_back(ListEntry(time, value));
		}
	}




}; // end class RadioStateAnalogueModel




/**
 * @brief The class that represents the Radio as a state machine.
 *
 * The Radio creates and updates its corresponding RadioStateAnalogueModel, that
 * is a filter representing the radios attenuation to a Signal depending on the
 * RadioState over time.
 *
 * For this basic version we assume a minimal attenuation when the Radio is in
 * state RX, and a maximum attenuation otherwise.
 *
 */
class Radio
{
public:
	/**
	* @brief The state of the radio of the nic.
	*
	* PLEASE INSERT NEW RADIOSTATES !!!BEFORE!!! NUM_RADIO_STATES
	*/
	enum RadioState {
		/* receiving state*/
		RX = 0,
		/* transmiting state*/
		TX,
		/* sleeping*/
		SLEEP,
		/* switching between two states*/
		SWITCHING,

		/* New entries from here... */

		/* ... to here. */

		/*
		 * NOTE: THIS IS NO REAL RADIOSTATE, JUST A COUNTER FOR THE NUMBER OF RADIOSTATES
		 * IT ALWAYS NEEDS TO BE THE LAST ENTRY IN THIS ENUM!
		 */
		NUM_RADIO_STATES
	};

protected:

	/**
	 * The current state the radio is in
	 */
	int state;
	int nextState;


	/**
	 * Array for storing switchtimes between states
	 */
	const int numRadioStates;
	simtime_t** swTimes;

	/**
	 * Constants to store the minimum and maximum attenuation for a Radio instance
	 */
	const double minAtt;
	const double maxAtt;

	/**
	 * The corresponding RadioStateAnalogueModel, depends on the characteristics
	 * of the radio
	 */
	RadioStateAnalogueModel rsam;

public:

	/**
	 * @brief Default constructor for instances of class Radio
	 */
	Radio(int _state = RX, double _minAtt = 1.0, double _maxAtt = 0.0)
		: state(_state), nextState(_state), numRadioStates(NUM_RADIO_STATES),
		minAtt(_minAtt), maxAtt(_maxAtt), rsam(mapStateToAtt(state))
	{
		// allocate memory for one dimension
		swTimes = new simtime_t* [numRadioStates];

		// go through the first dimension and
		for (int i = 0; i < numRadioStates; i++)
		{
			// allocate memory for the second dimension
			swTimes[i] = new simtime_t[numRadioStates];
		}

		// initialize all matrix entries to 0.0
		for (int i = 0; i < numRadioStates; i++)
		{
			for (int j = 0; j < numRadioStates; j++)
			{
				swTimes[i][j] = 0;
			}
		}


	}

	/**
	 * @brief Destructor for the Radio class
	 */
	virtual ~Radio()
	{
		// delete all allocated memory for the switching times matrix
		for (int i = 0; i < numRadioStates; i++)
		{
			delete[] swTimes[i];
		}

		delete[] swTimes;
		swTimes = 0;

	}

	/**
	 * @brief A function called by the Physical Layer to start the switching process to a new RadioState
	 *
	 *
	 *
	 * @return	-1: Error code if the Radio is currently switching
	 * 			else: switching time from the current RadioState to the new RadioState
	 *
	 *
	 * The actual simtime must be passed, to create properly RSAMEntry
	 */
	simtime_t switchTo(int newState, simtime_t now)
	{
		// state to switch to must be in a valid range, i.e. 0 <= newState < numRadioStates
		assert(0 <= newState && newState < numRadioStates);

		// state to switch to must not be SWITCHING
		assert(newState != SWITCHING);


		// return error value if newState is the same as the current state
		// if (newState == state) return -1;

		// return error value if Radio is currently switching
		if (state == SWITCHING) return -1;


		/* REGULAR CASE */


		// set the nextState to the newState and the current state to SWITCHING
		nextState = newState;
		int lastState = state;
		state = SWITCHING;

		// make entry to RSAM
		makeRSAMEntry(now, state);

		// return matching entry from the switch times matrix
		return swTimes[lastState][nextState];
	}

	/**
	 * @brief function called by PhyLayer in order to make an entry in the switch times matrix,
	 * i.e. set the time for switching from one state to another
	 *
	 */
	void setSwitchTime(int from, int to, simtime_t time)
	{
		// assert parameters are in valid range
		assert(time >= 0.0);
		assert(0 <= from && from < numRadioStates);
		assert(0 <= to && to < numRadioStates);

		// it shall not be possible to set times to/from SWITCHING
		assert(from != SWITCHING && to != SWITCHING);


		swTimes[from][to] = time;
		return;
	}

	/**
	 * Returns the state the Radio is currently in
	 *
	 */
	int getCurrentState() const
	{

		return state;
	}



	/**
	 * @brief called by PhyLayer when duration-time for the
	 * current switching process is up
	 *
	 * Radio checks whether it is in switching state (pre-condition)
	 * and switches to the target state
	 *
	 * The actual simtime must be passed, to create properly RSAMEntry
	 *
	 */
	void endSwitch(simtime_t now)
	{
		// make sure we are currently switching
		assert(state == SWITCHING);

		// set the current state finally to the next state
		state = nextState;

		// make entry to RSAM
		makeRSAMEntry(now, state);

		return;
	}

	/**
	 * @brief Returns a pointer to the RadioStateAnalogueModel
	 *
	 * This method is intended to be used by the PhyLayer to obtain a pointer
	 * to the corresponding RSAM to this Radio
	 */
	RadioStateAnalogueModel* getAnalogueModel()
	{
		return (&rsam);
	}

	/**
	 * @brief discards information in the RadioStateAnalogueModel before given time-point
	 *
	 */
	void cleanAnalogueModelUntil(simtime_t t)
	{
		rsam.cleanUpUntil(t);
	}


protected:
	/**
	 * @brief responsible for making entries to the RSAM
	 */
	virtual void makeRSAMEntry(simtime_t time, int state)
	{
		rsam.writeRecvEntry(time, mapStateToAtt(state));
	}

	/**
	 * @brief maps RadioState to attenuation, the Radios receiving characteristic
	 *
	 */
	virtual double mapStateToAtt(int state)
	{
		if (state == RX)
		{
			return minAtt;
		} else
		{
			return maxAtt;
		}
	}



}; // end class Radio



/**
 * @brief ConstMapingIterator implementation for a RSAM
 *
 */
class RSAMConstMappingIterator : public ConstMappingIterator
{
protected:

	const RadioStateAnalogueModel* rsam;

	typedef std::list<RadioStateAnalogueModel::ListEntry> CurrList;
	CurrList::const_iterator it;

	Argument position;
	Argument nextPosition;

	simtime_t signalStart;
	simtime_t signalEnd;



public:

	RSAMConstMappingIterator(const RadioStateAnalogueModel* _rsam,
								simtime_t _signalStart,
								simtime_t _signalEnd) :
		rsam(_rsam),
		signalStart(_signalStart),
		signalEnd(_signalEnd)
	{
		assert(_rsam);

		assert( !(signalStart < rsam->radioIsReceiving.front().getTime()) );

		jumpToBegin();
	}

	virtual ~RSAMConstMappingIterator() {}

	/**
	 * @brief Lets the iterator point to the passed position.
	 *
	 * The passed new position can be at arbitary places, jumping explicitly
	 * before signalStart is allowed.
	 */
	virtual void jumpTo(const Argument& pos)
	{
		// extract the time-component from the argument
		simtime_t t = pos.getTime();

		assert( !(rsam->radioIsReceiving.empty()) &&
				!(t < rsam->radioIsReceiving.front().getTime()) );

		// current position is already correct
		if( t == position.getTime() )
			return;

		// this automatically goes over all zero time switches
		it = upper_bound(rsam->radioIsReceiving.begin(), rsam->radioIsReceiving.end(), t);

		--it;
		position.setTime(t);
		setNextPosition();
	}

	/**
	 * Helper function that sets member nextPosition. Presumes that
	 * iterator it and member position are set correctly.
	 *
	 * This function does not care for zero time switches!
	 * This must be done before!
	 *
	 * Might be helpful if position of iterator it has not been set
	 * by upper_bound before (it has not just been standing on the "nextPosition").
	 */
	virtual void setNextPosition()
	{
		if (hasNext()) // iterator it does not stand on last entry and next entry is before signal end
		{
			if(position.getTime() < signalStart) //signal start is our first key entry
			{
				nextPosition.setTime(signalStart);
			} else
			{
				CurrList::const_iterator it2 = it;
				it2++;

				nextPosition.setTime(it2->getTime());
			}

		} else // iterator it stands on last entry or next entry whould be behind signal end
		{
			nextPosition.setTime(position.getTime() + 1);
		}

	}

	/**
	 * @brief Lets the iterator point to the begin of the function.
	 *
	 * The beginning of the function depends on the implementation.
	 */
	virtual void jumpToBegin()
	{
		jumpTo(signalStart);

		/*
		it = rsam->radioIsReceiving.begin();
		simtime_t t = it->getTime();

		assert( !(signalStart < t) );

		iterateToOverZeroSwitches(signalStart);

		position.setTime(signalStart);
		setNextPosition();
		*/
	}

	/**
	 * @brief Iterates to the specified position. This method
	 * should be used if the new position is near the current position.
	 */
	virtual void iterateTo(const Argument& pos)
	{
		// extract the time component from the passed Argument
		simtime_t t = pos.getTime();

		// ERROR CASE: iterating to a position before (time) the beginning of the mapping is forbidden
		assert( !(rsam->radioIsReceiving.empty()) &&
				!(t < rsam->radioIsReceiving.front().getTime()) );

		assert( !(t < position.getTime()) );

		// REGULAR CASES:
		// t >= position.getTime();

		// we are already exactly there
		if( t == position.getTime() )
			return;

		// we iterate there going over all zero time switches
		iterateToOverZeroSwitches(t);

		// update current position
		position.setTime(t);
		setNextPosition();

	}


	/**
	 * @brief Iterates to the next position of the function.
	 *
	 * The next position depends on the implementation of the
	 * Function.
	 */
	virtual void next()
	{
		iterateTo(nextPosition);
	}

	/**
	 * @brief Returns true if the current position of the iterator
	 * is in range of the function.
	 *
	 * This method should be used as end-condition when iterating
	 * over the function with the "next()" method.
	 */
	virtual bool inRange() const
	{
		simtime_t t = position.getTime();
		simtime_t lastEntryTime = std::max(rsam->radioIsReceiving.back().getTime(), signalStart);

		return 	signalStart <= t
				&& t <= signalEnd
				&& t <= lastEntryTime;

	}

	/**
	 * @brief Returns true if the iterator has a next value
	 * inside its range.
	 */
	virtual bool hasNext() const
	{
		assert( !(rsam->radioIsReceiving.empty()) );

		CurrList::const_iterator it2 = it;
		if (it2 != rsam->radioIsReceiving.end())
		{
			it2++;
		}

		return 	position.getTime() < signalStart
				|| (it2 != rsam->radioIsReceiving.end() && it2->getTime() <= signalEnd);
	}

	/**
	 * @brief Returns the current position of the iterator.
	 */
	virtual const Argument& getPosition() const
	{
		return position;
	}

	virtual const Argument& getNextPosition() const
	{
		return nextPosition;
	}

	/**
	 * @brief Returns the value of the function at the current
	 * position.
	 */
	virtual double getValue() const
	{
		return it->getValue();
	}


	/**
	 * @brief Iterates to valid entry for timepoint t over all zero-time switches
	 * starting from the current position of iterator it
	 *
	 */
	virtual void iterateToOverZeroSwitches(simtime_t t)
	{
		if( it != rsam->radioIsReceiving.end() && !(t < it->getTime()) )
		{
			// and go over (ignore) all zero-time-switches, to the next greater entry (time)
			while( it != rsam->radioIsReceiving.end() && !(t < it->getTime()) )
				it++;

			// go back one step, here the iterator 'it' is placed right
			it--;
		}
	}

}; // end class RSAMConstMappingIterator


/**
 * @brief This class represents the corresponding mapping
 * to a RadioStateAnalogueModel (RSAM).
 *
 * Since RSAM is modified dynamically over time,
 * RSAMMapping doesn't store the information itself, but
 * interacts with its RSAM to obtain information about the mapping.
 *
 * The relation to RSAM is very tight. RSAM creates an appropriate RSAMMapping
 * by passing a self-pointer to the constructor call.
 *
 * class RSAMMapping is a friend of class RadioStateAnalogueModel
 *
 *
 */
class RSAMMapping : public ConstMapping
{
protected:

	const RadioStateAnalogueModel* rsam;
	simtime_t signalStart;
	simtime_t signalEnd;

public:
	/**
	 * @brief Constructor taking a pointer to the corresponding RSAM
	 *
	 */
	RSAMMapping(const RadioStateAnalogueModel* _rsam,
				simtime_t _signalStart,
				simtime_t _signalEnd) :
		ConstMapping(),
		rsam(_rsam),
		signalStart(_signalStart),
		signalEnd(_signalEnd)
	{
		assert(_rsam);
		assert( !(signalStart < rsam->radioIsReceiving.front().getTime()) );
	}

	virtual ~RSAMMapping() {}

	/**
	 * @brief Returns the value of this Function at position specified
	 * by the passed Argument. Zero-time-switches are ignored here,
	 * i.e. in case of multiple entries at the same time-point, the last one
	 * is significant.
	 *
	 * In this case we have a function: simtime_t -> attenuation
	 *
	 */
	virtual double getValue(const Argument& pos) const
	{
		// extract the time-component from the argument
		simtime_t t = pos.getTime();

		// assert that t is not before the first timepoint in the RSAM
		// and receiving list is not empty
		assert( !(rsam->radioIsReceiving.empty()) &&
				!(t < rsam->radioIsReceiving.front().getTime()) );

		/* receiving list contains at least one entry */

		// set an iterator to the first entry with timepoint > t
		std::list<RadioStateAnalogueModel::ListEntry>::const_iterator it;
		it = upper_bound(rsam->radioIsReceiving.begin(), rsam->radioIsReceiving.end(), t);

		// REGULAR CASE: it points to an element that has a predecessor
		it--; // go back one entry, this one is significant!

		return it->getValue();
	}

	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function.
	 */
	virtual ConstMappingIterator* createConstIterator()
	{
		return new RSAMConstMappingIterator(rsam, signalStart, signalEnd);
	}

	/**
	 * @brief Returns a pointer of a new Iterator which is able to iterate
	 * over the function. The iterator starts at the passed position.
	 *
	 */
	virtual ConstMappingIterator* createConstIterator(const Argument& pos)
	{
		RSAMConstMappingIterator* rsamCMI
			= new RSAMConstMappingIterator(rsam, signalStart, signalEnd);

		rsamCMI->jumpTo(pos);

		return rsamCMI;
	}

	virtual ConstMapping* constClone() const
	{
		return new RSAMMapping(*this);
	}



}; // end class RSAMMapping





#endif /*PHYUTILS_H_*/
