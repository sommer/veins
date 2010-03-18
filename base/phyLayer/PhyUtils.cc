#include "PhyUtils.h"

using namespace std;

void RadioStateAnalogueModel::filterSignal(Signal& s)
{
	simtime_t start = s.getSignalStart();
	simtime_t end = start + s.getSignalLength();

	RSAMMapping* attMapping = new RSAMMapping(this, start, end);
	s.addAttenuation(attMapping);
}

void RadioStateAnalogueModel::cleanUpUntil(simtime_t t)
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

void RadioStateAnalogueModel::writeRecvEntry(simtime_t time, double value)
{
	// bugfixed on 08.04.2008
	assert( (radioIsReceiving.empty()) || (time >= radioIsReceiving.back().getTime()) );

	radioIsReceiving.push_back(ListEntry(time, value));

	if (!currentlyTracking)
	{
		cleanUpUntil(time);

		assert(radioIsReceiving.back().getTime() == time);
	}
}




Radio::Radio(int numRadioStates,
			 bool recordStats,
			 int initialState,
			 double minAtt, double maxAtt):
	state(initialState), nextState(initialState),
	numRadioStates(numRadioStates),
	minAtt(minAtt), maxAtt(maxAtt),
	rsam(mapStateToAtt(initialState))
{
	radioStates.setName("Radio state");
	radioStates.setEnabled(recordStats);

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

Radio::~Radio()
{
	// delete all allocated memory for the switching times matrix
	for (int i = 0; i < numRadioStates; i++)
	{
		delete[] swTimes[i];
	}

	delete[] swTimes;
	swTimes = 0;
}

simtime_t Radio::switchTo(int newState, simtime_t now)
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
	radioStates.record(state);

	// make entry to RSAM
	makeRSAMEntry(now, state);

	// return matching entry from the switch times matrix
	return swTimes[lastState][nextState];
}

void Radio::setSwitchTime(int from, int to, simtime_t time)
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

void Radio::endSwitch(simtime_t now)
{
	// make sure we are currently switching
	assert(state == SWITCHING);

	// set the current state finally to the next state
	state = nextState;
	radioStates.record(state);

	// make entry to RSAM
	makeRSAMEntry(now, state);

	return;
}




RSAMConstMappingIterator::RSAMConstMappingIterator
							(const RadioStateAnalogueModel* rsam,
							 simtime_t signalStart,
							 simtime_t signalEnd) :
	rsam(rsam),
	signalStart(signalStart),
	signalEnd(signalEnd)
{
	assert(rsam);

	assert( !(signalStart < rsam->radioIsReceiving.front().getTime()) );

	jumpToBegin();
}

void RSAMConstMappingIterator::jumpTo(const Argument& pos)
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

void RSAMConstMappingIterator::setNextPosition()
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

void RSAMConstMappingIterator::iterateTo(const Argument& pos)
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

bool RSAMConstMappingIterator::inRange() const
{
	simtime_t t = position.getTime();
	simtime_t lastEntryTime = std::max(rsam->radioIsReceiving.back().getTime(), signalStart);

	return 	signalStart <= t
			&& t <= signalEnd
			&& t <= lastEntryTime;

}

bool RSAMConstMappingIterator::hasNext() const
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

void RSAMConstMappingIterator::iterateToOverZeroSwitches(simtime_t t)
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








double RSAMMapping::getValue(const Argument& pos) const
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

ConstMappingIterator* RSAMMapping::createConstIterator(const Argument& pos)
{
	RSAMConstMappingIterator* rsamCMI
		= new RSAMConstMappingIterator(rsam, signalStart, signalEnd);

	rsamCMI->jumpTo(pos);

	return rsamCMI;
}
