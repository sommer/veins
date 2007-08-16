#include "FoxtrotPacket.h"
#include "foxtrot_types.h"
#include "BaseUtility.h"
#include "Foxtrot.h"

#include <assert.h>
#include "FWMath.h"

Register_Class(FoxtrotPacket);

FoxtrotPacket::FoxtrotPacket(const char *name, int kind):FoxtrotPacket_Base(name, kind)
{
	debug = true;
	for (uint8_t i = 0; i < AXES; i++)
		loc_var[i].min = loc_var[i].max = MAX_FOXTROT_VALUE;
}

void FoxtrotPacket::setDataArraySize(unsigned int size)
{
	FoxtrotPacket_Base::setDataArraySize(size);
	for (uint8_t i = 0; i < size; i++)
		data_var[i].min = data_var[i].max = MAX_FOXTROT_VALUE;
}

void FoxtrotPacket::setAllLoc(const FoxtrotPacket * other)
{
	for (unsigned int i = 0; i < AXES; i++)
		memcpy(&(this->loc_var[i]), &(other->loc_var[i]), sizeof(foxtrot_data));
}

void FoxtrotPacket::setAllData(const FoxtrotPacket * other)
{
	setDataArraySize(other->data_arraysize);
	for (unsigned int i = 0; i < data_arraysize; i++)
		memcpy(&(this->data_var[i]), &(other->data_var[i]), sizeof(foxtrot_data));
}

void FoxtrotPacket::setLocalPos(Foxtrot * f)
{
	BaseUtility *bs = dynamic_cast < BaseUtility * >(f->getNodeModule("BaseUtility"));
	const Coord *loc = bs->getPos();
	foxtrot_data_uniset(&this->loc_var[0], loc->getX());
	foxtrot_data_uniset(&this->loc_var[1], loc->getY());
#if AXES == 3
	foxtrot_data_uniset(&this->loc_var[2], loc->getZ());
#endif
}

void FoxtrotPacket::setAllData(Foxtrot * f, const foxtrot_data * other)
{
	assert(data_arraysize != 0);
	for (unsigned int i = 0; i < data_arraysize; i++)
		memcpy(&(this->data_var[i]), &(other[i]), sizeof(foxtrot_data));
}

bool FoxtrotPacket::isEqual(const FoxtrotPacket * compare)
{
	if (data_arraysize != compare->data_arraysize)
		return false;
	for (uint8_t i = 0; i < data_arraysize; i++)
	{
		if (data_var[i].min != compare->data_var[i].min)
		{
			DBG(FP_PRINTF " != " FP_PRINTF " for %d\n", data_var[i].min, compare->data_var[i].min, i);
			return false;
		}
		if (data_var[i].max != compare->data_var[i].max)
		{
			DBG(FP_PRINTF " != " FP_PRINTF " for %d\n", data_var[i].max, compare->data_var[i].max, i);
			return false;
		}
	}
	for (uint8_t i = 0; i < AXES; i++)
	{
		if (loc_var[i].min != compare->loc_var[i].min)
		{
			DBG("(loc)" FP_PRINTF " != " FP_PRINTF " for %d\n", loc_var[i].min, compare->loc_var[i].min, i);
			return false;
		}
		if (loc_var[i].max != compare->loc_var[i].max)
		{
			DBG("(loc)" FP_PRINTF " != " FP_PRINTF " for %d\n", loc_var[i].max, compare->loc_var[i].max, i);
			return false;
		}
	}
	return true;
}

void foxtrot_data_set(foxtrot_data * pt, const foxtrot_point a, const foxtrot_point b)
{
	pt->min = a;
	pt->max = b;
}

void foxtrot_data_uniset(foxtrot_data * pt, const foxtrot_point a)
{
	foxtrot_data_set(pt, a, a);
}

void FoxtrotPacket::print(const char *beg) const
{
	uint8_t j;
	DBG("%s (time=", beg);
	if (getWhen() != NO_TIME)
		EV_clear << getWhen();
	else
		EV_clear << "<invalid>";
	EV_clear << ") loc is";
	for (j = 0; j < AXES; j++)
		EV_clear << " (" << getLoc(j).min << "-" << getLoc(j).max << ")";
	DBG_clear(" data is");
	for (j = 0; j < data_arraysize; j++)
		EV_clear << " (" << getData(j).min << "-" << getData(j).max << ")";
	EV_clear << "\n";
}

void FoxtrotPacket::expandData(const FoxtrotPacket * other)	// expand local data box such that it contains other's box
{
	for (uint8_t i = 0; i < data_arraysize; i++)
	{
		if (data_var[i].min == MAX_FOXTROT_VALUE || data_var[i].min > other->data_var[i].min)
			data_var[i].min = other->data_var[i].min;
		if (data_var[i].max == MAX_FOXTROT_VALUE || data_var[i].max < other->data_var[i].max)
			data_var[i].max = other->data_var[i].max;
	}
}

void FoxtrotPacket::expandBox(const FoxtrotPacket * other)	// expand local data box such that it contains other's box
{
	for (uint8_t i = 0; i < AXES; i++)
	{
		if (loc_var[i].min == MAX_FOXTROT_VALUE || loc_var[i].min > other->loc_var[i].min)
			loc_var[i].min = other->loc_var[i].min;
		if (loc_var[i].max == MAX_FOXTROT_VALUE || loc_var[i].max < other->loc_var[i].max)
			loc_var[i].max = other->loc_var[i].max;
	}
}

region_ft *FoxtrotPacket::gridToReal(unsigned int x, unsigned int y)
{
	assert(AXES == 2);

	region_ft *ret = (region_ft *) malloc(sizeof(region_ft));

	const double width = loc_var[0].max - loc_var[0].min, height = loc_var[1].max - loc_var[1].min;
	const double scx = width / (1.0 * GRID_WIDTH), scy = height / (1.0 * GRID_WIDTH);

	ret->x.min = (x * scx) + loc_var[0].min;
	ret->y.min = (y * scy) + loc_var[1].min;
	ret->x.max = ret->x.min + scx;
	if (ret->x.max > loc_var[0].max)	// work around rounding errors
		ret->x.max = loc_var[0].max;
	ret->y.max = ret->y.min + scy;
	if (ret->y.max > loc_var[1].max)	// work around rounding errors
		ret->y.max = loc_var[1].max;

	return ret;
}

grid_region *FoxtrotPacket::realToGrid(const region_ft * rf)
{
	assert(AXES == 2);
	assert(rf->x.min >= loc_var[0].min);
	assert(rf->x.max <= loc_var[0].max);
	assert(rf->y.min >= loc_var[1].min);
	assert(rf->y.max <= loc_var[1].max);

	grid_region *ret = (grid_region *) malloc(sizeof(grid_region));
	const double width = loc_var[0].max - loc_var[0].min, height = loc_var[1].max - loc_var[1].min;
	const double scx = width / (1.0 * GRID_WIDTH), scy = height / (1.0 * GRID_WIDTH);

	/*print("curr is");
	   DBG("scx = %lf, scy = %lf\n",scx,scy); */

	ret->left = (unsigned int)fabs((rf->x.min - loc_var[0].min) / scx);
	if (ret->left == GRID_WIDTH)
		ret->left = GRID_WIDTH - 1;
	ret->right = GRID_WIDTH - (unsigned int)fabs((loc_var[0].max - rf->x.max) / scx);
	if (ret->right == GRID_WIDTH)
		ret->right = GRID_WIDTH - 1;
	ret->top = (unsigned int)fabs((rf->y.min - loc_var[1].min) / scy);
	if (ret->top == GRID_WIDTH)
		ret->top = GRID_WIDTH - 1;
	ret->bottom = GRID_WIDTH - (unsigned int)fabs(scy > 0 ? ((loc_var[1].max - rf->y.max) / scy) : 0);
	if (ret->bottom == GRID_WIDTH)
		ret->bottom = GRID_WIDTH - 1;

	return ret;
}


FoxtrotPacket & FoxtrotPacket::operator=(const FoxtrotPacket & other)
{
	FoxtrotPacket_Base::operator=(other);
	memcpy(grid, other.grid, GRID_BYTES);
	return *this;
}

void FoxtrotPacket::print_grid(const char *beg)
{
	DBG("%s\n", beg);
	for (uint8_t j = 0; j < GRID_WIDTH; j++)
	{
		DBG(" ");
		for (uint8_t k = 0; k < GRID_WIDTH; k++)
		{
			if (GRID_CHK(grid, k, j))
			{
				DBG_clear("*");
			}
			else
			{
				DBG_clear("o");
			}
		}
		DBG_clear("\n");
	}
}
