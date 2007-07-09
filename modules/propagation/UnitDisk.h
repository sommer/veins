/* -*- mode:c++ -*- ********************************************************
 * file:        UnitDisk.h
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
 * description: propagation layer: unit disk model
 ***************************************************************************/
#ifndef UNIT_DISK_H
#define UNIT_DISK_H 1

#include "CollisionsModel.h"

class UnitDisk: public CollisionsModel
{
	protected:
		double radioRange;
	public:
		Module_Class_Members(UnitDisk, CollisionsModel, 0);
		void initialize(int stage);
		virtual NodeList * canHear(BasePhyLayer*);
};

#endif

