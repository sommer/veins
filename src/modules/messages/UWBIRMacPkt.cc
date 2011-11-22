
#include "UWBIRMacPkt.h"

#include <cassert>

Register_Class(UWBIRMacPkt);

UWBIRMacPkt& UWBIRMacPkt::operator=(const UWBIRMacPkt& other) {
	if (this==&other) return *this;
	UWBIRMacPkt_Base::operator=(other);
	bitValues.clear();
   	std::list<bool>::const_iterator citer = other.bitValues.begin();
   	while(citer != other.bitValues.end()) {
   		bitValues.push_back(*citer);
   		citer++;
   	}
	return *this;
}

void UWBIRMacPkt::setBitValuesArraySize(unsigned int size) {
	// do nothing, dynamic array
}

unsigned int UWBIRMacPkt::getBitValuesArraySize() const {
	return bitValues.size();
}

bool UWBIRMacPkt::getBitValues(unsigned int k) const {
	assert(k <= bitValues.size());
	return bitValues.front();   // do not use -- implemented because omnet wants it
}

void UWBIRMacPkt::setBitValues(unsigned int k, bool bitValue) { }

void UWBIRMacPkt::pushBitvalue(bool bitValue) {
  bitValues.push_back(bitValue);
}

bool UWBIRMacPkt::popBitValue() {
	bool bitValue = bitValues.front();
	bitValues.pop_front();
	return bitValue;
}

bool UWBIRMacPkt::isEmpty() {
	return bitValues.size() == 0;
}

