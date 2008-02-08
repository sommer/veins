#include "PhyUtils.h"

#include <FWMath.h>
#include <iostream>

using namespace std;


/**
 * assert helper functions
 * 
 */
void assertTrue(string msg, bool value) {
    if (!value) {
        cout << msg << " -- failed" << endl;
        exit(1);
    }
}

void assertFalse(string msg, bool value) { assertTrue(msg, !value); }

void assertEqual(string msg, double target, double actual) {
    if (!FWMath::close(target, actual)) {
        cout << msg << " -- failed: value was "
             << actual << " instead of " << target << endl;
        exit(1);
    }
}



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


void testConstructor()
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

void testFunctionality()
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








int main()
{
	
	testConstructor();
	testFunctionality();
	
}

