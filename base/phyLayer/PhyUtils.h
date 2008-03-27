#ifndef PHYUTILS_H_
#define PHYUTILS_H_

#include "AnalogueModel.h"

#include <omnetpp.h>
#include <iostream>
#include <assert.h>
#include <list>

using namespace std;


/**
 * @brief The class that represents the Radio as a state machine.
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
	RadioState state;
	RadioState nextState;
	
	/**
	 * Array for storing switchtimes between states
	 */
	const int numRadioStates;
	double** swTimes;
	
public:
	
	/**
	 * @brief Default constructor for instances of class Radio
	 */
	Radio(RadioState _state = SLEEP)
		: state(_state), nextState(_state), numRadioStates(NUM_RADIO_STATES)
	{
		// allocate memory for one dimension
		swTimes = new double* [numRadioStates];
		
		// go through the first dimension and
		for (int i = 0; i < numRadioStates; i++)
		{
			// allocate memory for the second dimension
			swTimes[i] = new double[numRadioStates];
		}
		
		// initialize all matrix entries to 0.0
		for (int i = 0; i < numRadioStates; i++)
		{
			for (int j = 0; j < numRadioStates; j++)
			{
				swTimes[i][j] = 0.0;
			}
		}

		
	}
	
	/**
	 * @brief Destructor for the Radio class
	 * 
	 * TODO: Check if pointers are 0 before delete (no hard req)
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
	 * 			
	 * 			else: switching time from the current RadioState to the new RadioState 
	 */
	simtime_t switchTo(RadioState newState)
	{
		// state to switch to must be in a valid range, i.e. 0 <= newState < numRadioStates
		assert(0 <= newState && newState < numRadioStates);
		
		// state to switch to must not be SWITCHING
		assert(newState != SWITCHING);
		
		
		// return error value if newState is the same as the current state
		// if (newState == state) return -1;
		
		// return error value if Radio is currently switching
		if (state == SWITCHING) return -1;
		
		
		// set the nextState to the newState and the current state to SWITCHING 
		nextState = newState;
		RadioState lastState = state;
		state = SWITCHING;
		
		// return matching entry from the switch times matrix
		return swTimes[lastState][nextState];
	}
	
	/**
	 * @brief function called by PhyLayer in order to make an entry in the switch times matrix,
	 * i.e. set the time for switching from one state to another
	 * 
	 */
	void setSwitchTime(RadioState from, RadioState to, double time)
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
	RadioState getCurrentState()
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
	 */
	void endSwitch()
	{
		// make sure we are currently switching
		assert(state == SWITCHING);
		
		// set the current state finally to the next state
		state = nextState;
		
		return;
	}
	
	

}; // end class Radio



/**
 * \brief This special AnalogueModel provides filtering of a Signal
 * according to the actual RadioStates the Radio were in during
 * the Signal's time interval
 * 
 * TODO: implement
 * 
 */
class RadioStateAnalogueModel : public AnalogueModel
{

protected:
	
	/**
	 * data structure for the list elements, consists basically
	 * of a pair of a simtime_t and a bool entry (simple timestamp)
	 * 
	 * further functionality can be added later if needed 
	 * 
	 */
	class ListEntry
	{
	
	protected:
		pair<simtime_t, bool> basicTimestamp;
		
	public:
		ListEntry(simtime_t time, bool value)
		{
			basicTimestamp = pair<simtime_t, bool> (time, value);
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
		
		bool getValue() const
		{
			return basicTimestamp.second;
		}
		
		void setValue(bool value)
		{
			basicTimestamp.second = value;
		}
		
		/**
		 * overload of operator < for class ListEntry to be able to use the STL Algorithm "lower_bound"
		 */
		bool operator<(const simtime_t& t)
		{
			return (this->getTime() < t);			
		}
		
		
	};
	
	
	/**
	 * \brief Indicator variable whether we are currently tracking changes
	 */
	bool currentlyTracking;
	
	/**
	 * \brief Data structure to track when the Radio is receiving
	 */
	list<ListEntry> radioIsReceiving;
	
		
public:
	
	/**
	 * \brief Standard constructor for a RadioStateAnalogueModel instance
	 */
	RadioStateAnalogueModel(bool _currentlyTracking = false)
		: currentlyTracking(_currentlyTracking)
	{
	
	}
	
	/**
	 * \brief Filters the Signal according to the RadioState
	 */
	virtual void filterSignal(Signal& s)
	{
		// TODO implement
	}
	
	/**
	 * \brief Switches tracking mode on/off
	 */	
	void setTrackingModeTo(bool b)
	{
		currentlyTracking = b;
	}
	
	/**
	 * \brief Cleans up all stored information strictly before the given time-point,
	 * i.e. all elements with their timepoint strictly smaller than given key
	 * 
	 */
	void cleanUpUntil(simtime_t t)
	{
		// list is empty ==> nothing to do
		if ( radioIsReceiving.empty() ) return;
		
		/* the list contains at least one element */
		
		// CASE: t is smaller or equal the timepoint of the first element ==> nothing to do, return
		if ( t <= radioIsReceiving.front().getTime() )
		{ 
			return;
		}
		
		// CASE: t is greater than the timepoint of the last element ==> clear complete list, return
		if ( t > radioIsReceiving.back().getTime() )
		{ 
			radioIsReceiving.clear();
			return;
		}
		
		/*
		 * preconditions from now on:
		 * 1. list contains at least two elements, since 2. + 3.
		 * 2. t > first_timepoint
		 * 3. t <= last_timepoint
		 */
		
		// get an iterator and set it to the first timepoint >= t
		list<ListEntry>::iterator it;
		it = lower_bound(radioIsReceiving.begin(), radioIsReceiving.end(), t);
		
		// TODO Question: shall multiple entries with same timepoint be preserved
		// in this case or only the last one?
		// CASE: list contains an element with exactly the given key
		if ( it->getTime() == t )
		{
			radioIsReceiving.erase(radioIsReceiving.begin(), it);
			return;
		}
		
		// TODO: check whether this is allowed
		// CASE: t is "in between two elements" 
		// ==> set the iterators predecessors time to t, it becomes the first element
		it--; // go back one element, possible since this one has not been the first one
		
		it->setTime(t); // set this elements time to t 
		radioIsReceiving.erase(radioIsReceiving.begin(), it); // and erase all previous elements
		
	}
	
	/**
	 * \brief Stores an entry of the form "time-point/receiving or not (from now on)"
	 */
	void writeRecvEntry(simtime_t t, bool b)
	{
		assert( t >= (radioIsReceiving.end()->getTime()) );
		
		if (currentlyTracking)
		{
			radioIsReceiving.push_back(ListEntry(t, b));
		}
	}
	
	
	
	
}; // end class RadioStateAnalogueModel



#endif /*PHYUTILS_H_*/
