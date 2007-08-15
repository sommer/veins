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

const double BaseWorldUtility::speedOfLight = 299792458.0;

void BaseWorldUtility::initialize(int stage)
{
	if (stage == 0)
	{
        use2DFlag = par("use2D");

        if (use2DFlag) {
            playgroundSize = Coord(par("playgroundSizeX"),
                                   par("playgroundSizeY"));
        } else {
            playgroundSize = Coord(par("playgroundSizeX"),
                                   par("playgroundSizeY"),
                                   par("playgroundSizeZ"));
        }

		useTorusFlag = par("useTorus");
       
	}
}

Coord BaseWorldUtility::getRandomPosition()
{
    if (use2DFlag) {
        return Coord(genk_uniform(0, 0, playgroundSize.getX()),
                     genk_uniform(0, 0, playgroundSize.getY()));
    } else {
        return Coord(genk_uniform(0, 0, playgroundSize.getX()),
                     genk_uniform(0, 0, playgroundSize.getY()),
                     genk_uniform(0, 0, playgroundSize.getZ()));
    }
}
