/* -*- mode:c++ -*- ********************************************************
 * file:        BaseWorldUtility.h
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
 * description: basic world utility class
 *              provides world-required values
 **************************************************************************/

#include "BaseWorldUtility.h"

Define_Module(BaseWorldUtility);

const double BaseWorldUtility::speedOfLight = 300000000.0;

void BaseWorldUtility::initialize(int stage)
{
	if (stage == 0)
	{
		playgroundSize.x = par("playgroundSizeX");
		playgroundSize.y = par("playgroundSizeY");
	}
}

Coord BaseWorldUtility::getRandomPosition() {
    Coord p;
    p.x = genk_uniform(0, 0, playgroundSize.x);
    p.y = genk_uniform(0, 0, playgroundSize.y);
    p.z = genk_uniform(0, 0, playgroundSize.z);
    return p;
}


