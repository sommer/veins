/* -*- mode:c++ -*- ********************************************************
 * file:        FoxtrotPatterned.cc
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: aggregation layer: Foxtrot with patterned spacial merging
 ***************************************************************************/

#include "FoxtrotPatterned.h"
#include <assert.h>
#include <climits>
#include "FWMath.h"

Define_Module_Like(FoxtrotPatterned, BaseAggLayer);

bool FoxtrotPatterned::merge(uint8_t * subset, uint8_t * delete_count)
{
	const uint8_t count = storage.size();
	int i;
	bool timeset = false;
	FoxtrotPacket *midrange = new FoxtrotPacket();
	midrange->setDataArraySize(pts);

	for (i = 0; i < count; i++)
	{
		print_data("Data Point (merge)", storage[i]);
		midrange->expandBox(storage[i]);
		midrange->expandData(storage[i]);
	}
	print_data("Full box", midrange);
	GRID_RESET(midrange->grid);
	for (i = 0; i < count; i++)
	{
		if (!ANY_CHK(i, subset))
			continue;
		print_data("Data Point (merge)", storage[i]);
		storage[i]->print_grid("storage");
		for (uint8_t j = 0; j < GRID_WIDTH; j++)
		{
			for (uint8_t k = 0; k < GRID_WIDTH; k++)
			{
				if (GRID_CHK(storage[i]->grid, j, k))
				{
					region_ft *pf = storage[i]->gridToReal(j, k);
					DBG("real for pt %d, %d is (%lf, %lf), (%lf, %lf)\n", j, k, pf->x.min, pf->y.min, pf->x.max, pf->y.max);
					grid_region *gr = midrange->realToGrid(pf);
					free(pf);
					DBG("grid (%d, %d), (%d, %d)\n", gr->top, gr->left, gr->bottom, gr->right);
					//midrange->print_grid("mr before");
					for (uint8_t l = gr->left; l <= gr->right; l++)
						for (uint8_t m = gr->top; m <= gr->bottom; m++)
						{
							if (!GRID_CHK(midrange->grid, l, m))
								DBG("Setting %d,%d\n", l, m);
							GRID_SET(midrange->grid, l, m);
							//break;
						}
					//midrange->print_grid("mr after");
					free(gr);

					//assert(0);
				}
			}
		}
		storage[i]->print("added");
		midrange->print_grid("mr end");
		//assert(0);
	}
	//midrange->print_grid("mr end");
	//assert(0);
	for (i = count - 1; i >= 0; i--)
	{
		if (!ANY_CHK(i, subset))
			continue;
		if (!timeset || midrange->getWhen() > storage[i]->getWhen())
		{
			timeset = true;
			midrange->setWhen(storage[i]->getWhen());
		}
		storage.erase(i);
	}
	storage.push_back(midrange);
	return true;
}

void FoxtrotPatterned::newData(const foxtrot_data * data)
{
	local->setDataArraySize(pts);
	local->setWhen(NO_TIME);
	//print_data("local before",local);
	local->setAllData(this, data);
	GRID_FILL(local->grid);
	print_data("local", local);
	try_merge(local, true);
	if (!isSink)
	{
		if (wait_data)
		{
			doSend();
			wait_data = false;
		}
	}
	return;
}

void FoxtrotPatterned::dataUp(FoxtrotPacket * pkt)
{
	pkt->print("to app");
	pkt->print_grid("to app");
	sendUp(pkt);
}
