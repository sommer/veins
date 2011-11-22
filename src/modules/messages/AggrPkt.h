
#ifndef AGGR_PKT_
#define AGGR_PKT_

#include <list>

#include "MiXiMDefs.h"
#include "AggrPkt_m.h"

class MIXIM_API AggrPkt : public AggrPkt_Base
{
  public:
    AggrPkt(const char *name=NULL, int kind=0) : AggrPkt_Base(name,kind) {}
    AggrPkt(const AggrPkt& other) : AggrPkt_Base(other.getName()) {operator=(other);}
    AggrPkt& operator=(const AggrPkt& other) {AggrPkt_Base::operator=(other); return *this;}
    virtual AggrPkt *dup() const {return new AggrPkt(*this);}

    // array methods
    // do not use these
    virtual void setStoredPacketsArraySize(unsigned int size);
    virtual unsigned int getStoredPacketsArraySize() const;
    virtual pApplPkt& getStoredPackets(unsigned int k);
    virtual void setStoredPackets(unsigned int k, const pApplPkt& storedPackets_var);
    // instead, use those:
    virtual void storePacket(pApplPkt& storedPackets_var);
    virtual bool isEmpty();
    virtual pApplPkt& popFrontPacket();
  protected:
    std::list<pApplPkt> storedPackets;

};

#endif

