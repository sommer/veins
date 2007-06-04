#include "FoxtrotPacket.h"
#include "foxtrot_types.h"
#include "BaseUtility.h"
#include "Foxtrot.h"

#include <assert.h>

Register_Class(FoxtrotPacket);

void FoxtrotPacket::setAllData(const FoxtrotPacket *other)
{
	this->data_var = (other->data_arraysize==0) ? NULL : new foxtrot_data[other->data_arraysize];
	setDataArraySize(other->data_arraysize);
    for (unsigned int i=0; i<data_arraysize; i++)
        memcpy(&(this->data_var[i]),&(other->data_var[i]),sizeof(foxtrot_data));
}

void FoxtrotPacket::setAllData(Foxtrot *f, const foxtrot_data *other)
{
	BaseUtility *bs = dynamic_cast<BaseUtility*>(f->getNodeModule("BaseUtility"));
	const Coord *loc = bs->getPos();
	foxtrot_data_uniset(&this->data_var[0],loc->x);
	foxtrot_data_uniset(&this->data_var[1],loc->y);
	foxtrot_data_uniset(&this->data_var[2],loc->z);
	assert(data_arraysize!=0);
    for (unsigned int i=0; i<data_arraysize-PT(0); i++)
        memcpy(&(this->data_var[PT(i)]),&(other[i]),sizeof(foxtrot_data));
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
	EV << beg <<" (time="<<getWhen()<<"):";
	for (j=0;j<data_arraysize;j++)
		EV_clear << " ("<<getData(j).min<<"-"<<getData(j).max<<")";
	EV_clear << "\n";
}

