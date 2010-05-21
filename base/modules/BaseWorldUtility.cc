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
#include "FindModule.h"
#include "BaseConnectionManager.h"

Define_Module(BaseWorldUtility);


const double BaseWorldUtility::speedOfLight = 299792458.0; //metres per second



void BaseWorldUtility::initialize(int stage) {
	Blackboard::initialize(stage);

	if (stage == 0) {
        use2DFlag = par("use2D");

        if (use2DFlag) {
            playgroundSize = Coord(par("playgroundSizeX"),
                                   par("playgroundSizeY"));
        } else {
            playgroundSize = Coord(par("playgroundSizeX"),
                                   par("playgroundSizeY"),
                                   par("playgroundSizeZ"));
        }

        if(playgroundSize.getX() <= 0) {
        	opp_error("Playground size in X direction is invalid: "\
        			  "(%f). Should be greater zero.", playgroundSize.getX());
        }
        if(playgroundSize.getY() <= 0) {
			opp_error("Playground size in Y direction is invalid: "\
					  "(%f). Should be greater zero.", playgroundSize.getY());
		}
        if(!use2DFlag && playgroundSize.getZ() <= 0) {
			opp_error("Playground size in Z direction is invalid: "\
					  "(%f). Should be greater zero (or use 2D mode).", playgroundSize.getZ());
		}

		useTorusFlag = par("useTorus");

		airFrameId = 0;
	}
	else if(stage == 1) {
		//check if necessary modules are there
		//Connection Manager
		if(!FindModule<BaseConnectionManager*>::findGlobalModule()) {
			opp_warning("Could not find a connection manager module in the network!");
		}
	}
}

Coord BaseWorldUtility::getRandomPosition()
{
    if (use2DFlag) {
        return Coord(uniform(0, playgroundSize.getX()),
                     uniform(0, playgroundSize.getY()));
    } else {
        return Coord(uniform(0, playgroundSize.getX()),
                     uniform(0, playgroundSize.getY()),
                     uniform(0, playgroundSize.getZ()));
    }
}

