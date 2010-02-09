
#ifndef UWBIRMACPKT_
#define UWBIRMACPKT_

#include "UWBIRMacPkt_m.h"
#include <list>

using namespace std;

class UWBIRMacPkt : public UWBIRMacPkt_Base
{
   public:
     UWBIRMacPkt(const char *name=NULL, int kind=0) : UWBIRMacPkt_Base(name,kind) {}
     UWBIRMacPkt(const UWBIRMacPkt& other) : UWBIRMacPkt_Base(other.getName()) {operator=(other);}
     UWBIRMacPkt& operator=(const UWBIRMacPkt& other);
     virtual UWBIRMacPkt *dup() const {return new UWBIRMacPkt(*this);}
     // ADD CODE HERE to redefine and implement pure virtual functions from UWBIRMacPkt_Base
     virtual void setBitValuesArraySize(unsigned int size);
     virtual unsigned int getBitValuesArraySize() const;
     virtual bool getBitValues(unsigned int k) const;
     virtual void setBitValues(unsigned int k, bool bitValues_var);
     virtual void pushBitvalue(bool bitValue);
     virtual bool popBitValue();
     virtual bool isEmpty();
     list<bool> bitValues;
};

#endif
