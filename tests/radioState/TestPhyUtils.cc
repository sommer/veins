#include "PhyUtils.h"

#include <asserts.h>


/* ------ Testing stuff for Radio ------ */

// global variables needed for tests
const Radio::RadioState initialState =  Radio::RX;

const double RX2TX = 1.0;
const double RX2SLEEP = 2.0;
const double TX2RX = 3.0;
const double TX2SLEEP = 4.0;
const double SLEEP2RX = 5.0;
const double SLEEP2TX = 6.0;
const double RX2RX = 7.0;
const double TX2TX = 8.0;
const double SLEEP2SLEEP = 9.0;



// test functions


void testRadioConstructor()
{
	Radio radio1; // default constructor
		
	// check members of radio1
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::SLEEP);
	
	
	
	
	Radio radio2(initialState); // contructor with argument
	
	//check members of radio2
	assertTrue("RadioState is correct", radio2.getCurrentState() == initialState);
	
	
	cout << "Constructor Test passed." << endl;
	
	
	return;
}

int testSwitching(Radio& radio, Radio::RadioState to, double refValue)
{
	
	// check whether radio is currently switching
	if (radio.getCurrentState() == Radio::SWITCHING) return -1;
	
	
	
	// call switchTo
	double result = radio.switchTo(to);
	
	assertFalse("Radio does return a valid switch time", result < 0);
	assertTrue("Radio is now switching", radio.getCurrentState() == Radio::SWITCHING);
	assertEqual("Radio has returned the correct switch time", refValue, result);
	
	result = radio.switchTo(to);
	assertTrue("Radio is switching, error value awaited", result < 0);
	
	// call endSwitch
	radio.endSwitch();
	
	assertTrue("Radio has switched to correct state", radio.getCurrentState() == to);
	
	return 0;	
}

void testRadioFunctionality()
{
	Radio radio1;
	
	
	
	// call setSwTime() for every matrix entry
	//radio1.setSwitchTime(RX, RX, value);//not a regular case
	radio1.setSwitchTime(Radio::RX, Radio::TX, RX2TX);
	radio1.setSwitchTime(Radio::RX, Radio::SLEEP, RX2SLEEP);
	//radio1.setSwitchTime(RX, SWITCHING, value); //not a regular case
	
	radio1.setSwitchTime(Radio::TX, Radio::RX, TX2RX);
	//radio1.setSwitchTime(TX, TX, value);//not a regular case
	radio1.setSwitchTime(Radio::TX, Radio::SLEEP, TX2SLEEP);
	//radio1.setSwitchTime(TX, SWITCHING, value);//not a regular case
	
	radio1.setSwitchTime(Radio::SLEEP, Radio::RX, SLEEP2RX);
	radio1.setSwitchTime(Radio::SLEEP, Radio::TX, SLEEP2TX);
	//radio1.setSwitchTime(SLEEP, SLEEP, value);//not a regular case
	//radio1.setSwitchTime(SLEEP, SWITCHING, value);//not a regular case
	
	//radio1.setSwitchTime(SWITCHING, RX, value);//not a regular case
	//radio1.setSwitchTime(SWITCHING, TX, value);//not a regular case
	//radio1.setSwitchTime(SWITCHING, SLEEP, value);//not a regular case
	//radio1.setSwitchTime(SWITCHING, SWITCHING, value);//not a regular case
	
	// testing switching to the same state, this might not be needed
	radio1.setSwitchTime(Radio::RX, Radio::RX, RX2RX);
	radio1.setSwitchTime(Radio::TX, Radio::TX, TX2TX);
	radio1.setSwitchTime(Radio::SLEEP, Radio::SLEEP, SLEEP2SLEEP);
	
	int switchResult;
	
	// SLEEP -> RX
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::SLEEP);
	switchResult = testSwitching(radio1, Radio::RX, SLEEP2RX);
	assertTrue("Switching test", switchResult == 0);
	
	// RX -> RX
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::RX);
	switchResult = testSwitching(radio1, Radio::RX, RX2RX);
	assertTrue("Switching test", switchResult == 0);
	
	// RX -> TX
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::RX);
	switchResult = testSwitching(radio1, Radio::TX, RX2TX);
	assertTrue("Switching test", switchResult == 0);
	
	// TX -> TX
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::TX);
	switchResult = testSwitching(radio1, Radio::TX, TX2TX);
	assertTrue("Switching test", switchResult == 0);
	
	// TX -> SLEEP
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::TX);
	switchResult = testSwitching(radio1, Radio::SLEEP, TX2SLEEP);
	assertTrue("Switching test", switchResult == 0);
	
	// SLEEP -> SLEEP
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::SLEEP);
	switchResult = testSwitching(radio1, Radio::SLEEP, SLEEP2SLEEP);
	assertTrue("Switching test", switchResult == 0);
	
	// SLEEP -> TX
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::SLEEP);
	switchResult = testSwitching(radio1, Radio::TX, SLEEP2TX);
	assertTrue("Switching test", switchResult == 0);

	// TX - > RX
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::TX);
	switchResult = testSwitching(radio1, Radio::RX, TX2RX);
	assertTrue("Switching test", switchResult == 0);
		
	// RX - > SLEEP
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::RX);
	switchResult = testSwitching(radio1, Radio::SLEEP, RX2SLEEP);
	assertTrue("Switching test", switchResult == 0);
	
	
	assertTrue("RadioState is correct", radio1.getCurrentState() == Radio::SLEEP);
	
	
	
	
	cout << "SetSwitchTime test passed." << endl;
	
	
	return;	
}

/* ------ Testing stuff for RadioStateAnalogueModel ------ */


// further global variables

// timepoints to store in the analogue model
const double time1 = 0.2; // first timepoint
const double time2 = 0.3;
const double time3 = 0.51;
const double time4 = 0.6; // last timepoint

// timepoint for testing purposes
const double time5 = 0.5; // in between two timepoints, before duplicate
const double time6 = 0.52; // in between two timepoints, after duplicate
const double time7 = 0.19; // before first timepoint
const double time8 = 0.65; // after last timepoint
const double time9 = 0.59; // before last timepoint

typedef std::list<std::pair<simtime_t, bool> > RSList;

// reference lists
RSList emptyList = RSList();
RSList oneElemList = RSList();
RSList manyElemList = RSList();
RSList variableList = RSList();



// just an inherited class to get access to the members
class DiagRSAM : public RadioStateAnalogueModel
{

public:
	
	DiagRSAM(bool _currentlyTracking = false) : RadioStateAnalogueModel(_currentlyTracking) {}
	
	bool getTrackingFlag() { return currentlyTracking; }
	
	
	int getRecvListSize() { return ((int) radioIsReceiving.size()); }
	
	
	bool compareRecvLists(RSList refRecvList)
	{
		if ( ((int) radioIsReceiving.size()) != ((int) refRecvList.size()) ) return false;
		
		std::list<ListEntry>::iterator it1;
		RSList::iterator it2;
		
		for (it1 = radioIsReceiving.begin(), it2 = refRecvList.begin(); it1 != radioIsReceiving.end(); it1++, it2++)
		{
			if ( it1->getTime() != it2->first ) return false;
			if ( it1->getValue() != it2->second ) return false;
		}
		
		return true;
	}
	
	
};


void fillReferenceLists()
{
	// contains only last timestamp
	// (time4,true)
	oneElemList.push_back( std::pair<simtime_t, bool> (time4, true) );
	
	// contains all timestamps
	// (time1,true)--(time2,true)--(time3,false)--(time3,true)--(time4,true)
	manyElemList.push_back( std::pair<simtime_t, bool> (time1, true) );
	manyElemList.push_back( std::pair<simtime_t, bool> (time2, true) );
	manyElemList.push_back( std::pair<simtime_t, bool> (time3, false) );
	manyElemList.push_back( std::pair<simtime_t, bool> (time3, true) );
	manyElemList.push_back( std::pair<simtime_t, bool> (time4, true) );
	
	
	
}


void testRSAMConstructor()
{
	DiagRSAM m;
	assertFalse("Default constructor sets tracking off.", m.getTrackingFlag());
	assertTrue("Default constructor creates empty list.", m.getRecvListSize() == 0);
	
	m = DiagRSAM(false);
	assertFalse("DiagRSAM(false) sets tracking off.", m.getTrackingFlag());
	assertTrue("DiagRSAM(false) creates empty list.", m.getRecvListSize() == 0);
	
	m = DiagRSAM(true);
	assertTrue("DiagRSAM(true) sets tracking on.", m.getTrackingFlag());
	assertTrue("DiagRSAM(true) creates empty list.", m.getRecvListSize() == 0);
	
}


void testRSAMModification()
{
	// empty map, tracking off
	DiagRSAM m = DiagRSAM();
	
	// call writeRecvEntry, should not write to list
	m.writeRecvEntry(time1, true);
	assertTrue("Nothing has been written to list.", m.getRecvListSize() == 0);
	
	
	m.setTrackingModeTo(true);
	assertTrue("Tracking is on.", m.getTrackingFlag());
	
	m.setTrackingModeTo(false);
	assertFalse("Tracking is off.", m.getTrackingFlag());
	
	
	// tests for lists storing many entries
	
	// add the elements to the model
	// should create:
	
	// (time1,true)--(time2,true)--(time3,false)--(time3,true)--(time4,true)
	
	m = DiagRSAM();
	
	m.setTrackingModeTo(true);
	
	m.writeRecvEntry(time1, true);
	m.writeRecvEntry(time2, true);
	m.writeRecvEntry(time3, false);
	m.writeRecvEntry(time3, true);
	m.writeRecvEntry(time4, true);
	
	assertTrue("Elements have been written to list.", m.getRecvListSize() == 5);
	

	// compare lists
	assertTrue("Lists are equal. (many entries)", m.compareRecvLists(manyElemList));
	
	
	
	// make copies for cleanup-tests
	DiagRSAM temp;
	
	// special cases
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint before the first entry
	temp.cleanUpUntil(time7); // nothing should happen
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint after the last entry
	temp.cleanUpUntil(time8); // complete list should be cleaned
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(emptyList));
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint equal to the first entry
	temp.cleanUpUntil(time1); // nothing should happen
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint equal to the last entry
	temp.cleanUpUntil(time4); // only last entry should remain
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(oneElemList));
	
	
	// expected regular cases
	
	// (time3,false)--(time3,true)--(time4,true)
	variableList.clear();
	variableList.push_back( std::pair<simtime_t, bool> (time3, false) );
	variableList.push_back( std::pair<simtime_t, bool> (time3, true) );
	variableList.push_back( std::pair<simtime_t, bool> (time4, true) );
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint equal to the first entry of
	// a bunch with same timepoint
	temp.cleanUpUntil(time3); // all entries with timepoint >= time3 should remain
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));
	
	
	
	// (time5,true)--(time3,false)--(time3,true)--(time4,true)
	variableList.clear();
	variableList.push_back( std::pair<simtime_t, bool> (time5, true) );
	variableList.push_back( std::pair<simtime_t, bool> (time3, false) );
	variableList.push_back( std::pair<simtime_t, bool> (time3, true) );
	variableList.push_back( std::pair<simtime_t, bool> (time4, true) );
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint in between two timepoint, one entry must be modified,
	// all others before that one must be deleted
	temp.cleanUpUntil(time5); // (time2, true) must become (time5, true), (time1, true) must be deleted
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));
	
	
	
	// (time2,true)--(time3,false)--(time3,true)--(time4,true)
	variableList.clear();
	variableList.push_back( std::pair<simtime_t, bool> (time2, true) );
	variableList.push_back( std::pair<simtime_t, bool> (time3, false) );
	variableList.push_back( std::pair<simtime_t, bool> (time3, true) );
	variableList.push_back( std::pair<simtime_t, bool> (time4, true) );
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with exact timepoint of one entry in the middle
	temp.cleanUpUntil(time2); // all entries before (time2, true) must be deleted
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(variableList));
	
	
	// (time9,true)--(time4,true)
	variableList.clear();
	variableList.push_back( std::pair<simtime_t, bool> (time9, true) );
	variableList.push_back( std::pair<simtime_t, bool> (time4, true) );
	
	
	temp = m;
	assertTrue("Lists are equal. (many entries)", temp.compareRecvLists(manyElemList));
	// called with timepoint before a single entry 
	temp.cleanUpUntil(time9); // (time3, true) must become (time9, true) , all before must be deleted
	assertTrue("Lists are equal. (many entries) END", temp.compareRecvLists(variableList));
	
	
	
	
	// tests for lists storing exactly one entry
	DiagRSAM m2 = DiagRSAM(true);
	
	m2.writeRecvEntry(time4, true);
	
	assertTrue("Element has been written to list.", m2.getRecvListSize() == 1);
	assertTrue("Lists are equal. (one entry)", m2.compareRecvLists(oneElemList));
	
	DiagRSAM temp2;
	
	temp2 = m2;
	assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
	// called with timepoint before entry
	temp2.cleanUpUntil(time9); // nothing should happen
	assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
	
	temp2 = m2;
	assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
	// called with exactly the timepoint contained
	temp2.cleanUpUntil(time4); // nothing should happen
	assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
	
	temp2 = m2;
	assertTrue("Lists are equal. (one entry)", temp2.compareRecvLists(oneElemList));
	// called with timepoint after entry
	temp2.cleanUpUntil(time8); // list should be cleaned
	assertTrue("Lists are equal. (one entry) END", temp2.compareRecvLists(emptyList));

	
}



int main()
{
	
	// Radio
	testRadioConstructor();
	testRadioFunctionality();
	
	// RadioStateAnalogueModel
	fillReferenceLists();
	testRSAMConstructor();
	testRSAMModification();
	
}

