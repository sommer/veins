/* -*- mode:c++ -*- ********************************************************
 * file:        BaseWorldUtility.h
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
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

#ifndef BASE_WORLD_UTIL_H
#define BASE_WORLD_UTIL_H

#include <BaseModule.h>
#include "Coord.h"

/**
 * @brief Provides information and utility methods for the
 * whole simulation.
 *
 * @ingroup basicModules
 */
class BaseWorldUtility : public BaseModule
{
protected:
	/** @brief Size of the area the nodes are in (in meters)*/
    Coord playgroundSize;

    /** @brief Should the playground be treatend as a torus?*/
    bool useTorusFlag;

    /** @brief Should the world be 2-dimensional? */
    bool use2DFlag;

    /** @brief Provides a unique number for AirFrames per simulation */
    long airFrameId;
public:
	//Module_Class_Members(BaseWorldUtility,BaseModule,0);

	void initialize(int stage);

    /** @brief Returns the playgroundSize*/
    const Coord* getPgs(){
        return &playgroundSize;
    };

    /** @brief Returns true if our playground represents a torus (borders are connected)*/
    bool useTorus(){
    	return useTorusFlag;
    };

	/* @brief Random position somewhere in the playground */
	Coord getRandomPosition();

    /** @brief Speed of light */
    static const double speedOfLight;

    /** @brief Returns true if the world is 2-dimensional */
    bool use2D() { return use2DFlag; }

    /** @brief Returns an Id for an AirFrame, at the moment simply an incremented long-value */
    // TODO: return a really unique Id, handle overflow of range if needed
    long getUniqueAirFrameId(){

    	// if counter has done one complete cycle and will be set to a value it already had
    	if (airFrameId == -1){
    		// print a warning
    		ev << "WARNING: AirFrameId-Counter has done one complete cycle."
    		<< " AirFrameIds are repeating now and may not be unique anymore." << endl;
    	}

    	return airFrameId++;
    }
 };

#endif
