#include "AirFrame.h"

Register_Class(AirFrame);

/**
 * Returns a pointer to the Signal of this AirFrame. 
 */
Signal* AirFrame::getSignalPointer() {
	return signal;
}


/**
 * Sets the Signal of this AirFrame.
 * NOTE: The Signal has to be created by the new operator and
 * as soon as the Signal is passed to the AirFrame the AirFrame
 * keeps ownership of it!
 */
void AirFrame::setSignal(Signal* signal_var) {
	if(signal) {
		delete signal;
	}
	
	signal = signal_var;
}

/**
 * Free the signal of this AirFrame.
 */
AirFrame::~AirFrame() {
	if(signal) {
		delete signal;
	}
}

/**
 * Returns a reference to the Signal of this AirFrame.
 * 
 * NOTE: This method is only added for compability with OMNeT.
 * Using getSignalPointer() instead is highly recomended.
 */
Signal& AirFrame::getSignal() {
	return *signal;
}

/**
 * Sets the signal of this AirFrame by copying the passed
 * AirFrame.
 * 
 * Note: This method is mostly added for compability with OMNeT.
 * Creating the Signal with the new operator and passing the
 * pointer to "setSignal(Signal* signal)" is faster and therfore
 * recommended!
 */
void AirFrame::setSignal(const Signal& signal_var) {
	setSignal(new Signal(signal_var));
}
