#include "FoxtrotPacket.h"
#include "foxtrot_types.h"
#include "BaseUtility.h"
#include "Foxtrot.h"

#include <assert.h>

Register_Class(FoxtrotPacket);

FoxtrotPacket::FoxtrotPacket(const char *name, int kind) : FoxtrotPacket_Base(name,kind)
{
	debug=true;
}

void FoxtrotPacket::setDataArraySize(unsigned int size)
{
	FoxtrotPacket_Base::setDataArraySize(size);
	for (uint8_t i=0;i<size;i++)
		data_var[i].min = data_var[i].max = MAX_FOXTROT_VALUE;
}

void FoxtrotPacket::setAllLoc(const FoxtrotPacket *other)
{
    for (unsigned int i=0; i<AXES; i++)
        memcpy(&(this->loc_var[i]),&(other->loc_var[i]),sizeof(foxtrot_data));
}

void FoxtrotPacket::setAllData(const FoxtrotPacket *other)
{
	setDataArraySize(other->data_arraysize);
    for (unsigned int i=0; i<data_arraysize; i++)
        memcpy(&(this->data_var[i]),&(other->data_var[i]),sizeof(foxtrot_data));
}

void FoxtrotPacket::setLocalPos(Foxtrot *f)
{
	BaseUtility *bs = dynamic_cast<BaseUtility*>(f->getNodeModule("BaseUtility"));
	const Coord *loc = bs->getPos();
	foxtrot_data_uniset(&this->loc_var[0],loc->x);
	foxtrot_data_uniset(&this->loc_var[1],loc->y);
	foxtrot_data_uniset(&this->loc_var[2],loc->z);
}

void FoxtrotPacket::setAllData(Foxtrot *f, const foxtrot_data *other)
{
	assert(data_arraysize!=0);
    for (unsigned int i=0; i<data_arraysize; i++)
        memcpy(&(this->data_var[i]),&(other[i]),sizeof(foxtrot_data));
}

bool FoxtrotPacket::isEqual(const FoxtrotPacket *compare)
{
	if (data_arraysize!=compare->data_arraysize)
		return false;
	for (uint8_t i=0;i<data_arraysize;i++)
	{
		if (data_var[i].min!= compare->data_var[i].min)
		{
			DBG(FP_PRINTF " != " FP_PRINTF " for %d\n", data_var[i].min, compare->data_var[i].min, i);
			return false;
		}
		if (data_var[i].max!= compare->data_var[i].max)
		{
			DBG(FP_PRINTF " != " FP_PRINTF " for %d\n", data_var[i].max, compare->data_var[i].max, i);
			return false;
		}
	}
	for (uint8_t i=0;i<AXES;i++)
	{
		if (loc_var[i].min!= compare->loc_var[i].min)
		{
			DBG("(loc)" FP_PRINTF " != " FP_PRINTF " for %d\n", loc_var[i].min, compare->loc_var[i].min, i);
			return false;
		}
		if (loc_var[i].max!= compare->loc_var[i].max)
		{
			DBG("(loc)" FP_PRINTF " != " FP_PRINTF " for %d\n", loc_var[i].max, compare->loc_var[i].max, i);
			return false;
		}
	}
	return true;
}

void foxtrot_data_set(foxtrot_data *pt, const foxtrot_point a, const foxtrot_point b)
{
	pt->min=a;
	pt->max=b;
}

void foxtrot_data_uniset(foxtrot_data *pt, const foxtrot_point a)
{
	foxtrot_data_set(pt,a,a);
}

void FoxtrotPacket::print(const char* beg) const
{
	uint8_t j;
	EV << beg <<" (time=";
	if (getWhen()!=NO_TIME)
		EV_clear << getWhen();
	else
		EV_clear << "<invalid>";
	EV_clear << ") loc is";
	for (j=0;j<AXES;j++)
		EV_clear << " ("<<getLoc(j).min<<"-"<<getLoc(j).max<<")";
	DBG_clear(" data is");	
	for (j=0;j<data_arraysize;j++)
		EV_clear << " ("<<getData(j).min<<"-"<<getData(j).max<<")";
	EV_clear << "\n";
}

void FoxtrotPacket::expandData(const FoxtrotPacket *other) // expand local data box such that it contains other's box
{
	for (uint8_t i=0;i<data_arraysize;i++)
	{
		if (data_var[i].min==MAX_FOXTROT_VALUE || data_var[i].min>other->data_var[i].min)
			data_var[i].min = other->data_var[i].min;
		if (data_var[i].max==MAX_FOXTROT_VALUE || data_var[i].max<other->data_var[i].max)
			data_var[i].max = other->data_var[i].max;
	}
}
