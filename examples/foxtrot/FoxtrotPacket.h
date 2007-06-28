#ifndef FOXTROT_PACKET_H
#define FOXTROT_PACKET_H

#include "FoxtrotPacket_m.h"

class Foxtrot;

class FoxtrotPacket : public FoxtrotPacket_Base
{
  public:
    FoxtrotPacket(const char *name=NULL, int kind=0);
    FoxtrotPacket(const FoxtrotPacket& other) : FoxtrotPacket_Base(other.name()) {operator=(other);debug=true;}
    FoxtrotPacket& operator=(const FoxtrotPacket& other) {FoxtrotPacket_Base::operator=(other); return *this;}
    virtual cPolymorphic *dup() const {return new FoxtrotPacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from FoxtrotPacket_Base
	
    virtual void setDataArraySize(unsigned int size);
	void setLocalPos(Foxtrot *);
	void setAllLoc(const FoxtrotPacket *other);
	void setAllData(const FoxtrotPacket *other);
	void setAllData(Foxtrot *f, const foxtrot_data *other);
	
	bool isEqual(const FoxtrotPacket *compare);

	void setMinLoc(unsigned int index, foxtrot_point val) {loc_var[index].min = val;}
	void setMaxLoc(unsigned int index, foxtrot_point val) {loc_var[index].max = val;}

	void setMinData(unsigned int index, foxtrot_point val) {data_var[index].min = val;}
	void setMaxData(unsigned int index, foxtrot_point val) {data_var[index].max = val;}
	void print(const char* beg) const;

	void expandData(const FoxtrotPacket *other); // expand local data box such that it contains other's box
};

#endif
