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

#include "MiXiMDefs.h"
#include "ConnectionManager.h"

//TODO: clean up UnitDisk code, maybe extends its ned file from baseConnectionManager
class MIXIM_API UnitDisk: public ConnectionManager
{
protected:
	/** @brief Holds the maximum interference range.*/
	double radioRange;

	/** @brief Enables debugging for this module.*/
	bool debug;

public:
	/** @brief Called by Omnet++ during initialisation.*/
	void initialize(int stage);

	/** @brief Returns a constant value as maximum interference range.*/
	virtual double calcInterfDist() { return par("radioRange").doubleValue(); }
};

#endif

