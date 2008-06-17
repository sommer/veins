/* -*- mode:c++ -*- ********************************************************
 * file:        Move.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#ifndef MOVE_H
#define MOVE_H

#include <string>
#include <cmath>

#include <omnetpp.h>

#include "Coord.h"
#include "BaseUtility.h"
#include "ImNotifiable.h"

/**
 * @brief Class to store object position and movement
 *
 * @ingroup basicUtils
 * @ingroups utils
 * @ingroup blackboard
 *
 * @author Andreas Koepke
 **/

/** TODO: Think about making members private and encapsulate access to members
* to guarantee consistence and properly working functions.
*/

class Move : public BBItem {
//class Move {
    BBITEM_METAINFO(BBItem);

 public:
    /** @brief Start position of the host **/
    Coord startPos;
    /** @brief start time at which host started at startPos **/
    simtime_t startTime;
    /** @brief direction the host is moving to **/
    Coord direction;
    /** speed of the host **/
    double speed;

public:
    void setDirection(const Coord& target) {
		
    	double d = startPos.distance( target );
	    direction = (target - startPos) / d;

        //double d = sqrt(dir.x*dir.x + dir.y*dir.y);
        //direction.x = dir.x/d;
        //direction.y = dir.y/d;
    }
    
    /*
     * @brief Returns the position of the Move (Host) at the specified point in time.
     * It is intended to be passed simTime() as actualTime and returns the actual position.
     * 
     * Assumes that direction represents a normalized vector (length equals 1.0).
     * Further this function does not check whether the given time point is before
     * the startTime of the actual move pattern. So in this case one might obtain
     * an unintended result.
     * 
     */
    virtual Coord getPositionAt(simtime_t actualTime) const
    {
    	// if speed is very close to 0.0, the host is practically standing still
    	if ( FWMath::close(speed, 0.0) ) return startPos;
    	
    	// otherwise: actualPos = startPos + ( direction * v * t )
    	return startPos + ( direction * speed * (actualTime - startTime) );
    }
    
public:
    
    std::string info() const {
        std::ostringstream ost;
        ost << " HostMove "
            << " startPos: " << startPos.info()
            << " direction: " << direction.info()
            << " startTime: " << startTime
            << " speed: " << speed;
        return ost.str();
    }
};

#endif

