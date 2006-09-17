/* -*- mode:c++ -*- ********************************************************
 * file:        HostMove.h
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

#ifndef HOST_MOVE_H
#define HOST_MOVE_H

#include <string>
#include "Coord.h"
#include "Blackboard.h"
#include <omnetpp.h>
#include <cmath>

/**
 * @brief Class to store host position and movement
 *
 * @ingroup basicUtils
 * @ingroups utils
 * @ingroup blackboard
 *
 * @author Andreas Koepke
 **/
class HostMove : public BBItem 
{
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
	direction.x = (target.x - startPos.x) / d;
	direction.y = (target.y - startPos.y) / d;

        //double d = sqrt(dir.x*dir.x + dir.y*dir.y);
        //direction.x = dir.x/d;
        //direction.y = dir.y/d;
    }
    
public:
    
    std::string info() {
        std::ostringstream ost;
        ost << " HostMove "
            << " startPos.x: "<<startPos.x
            << " startPos.y: "<<startPos.y
            << " direction.x: "<< direction.x
            << " direction.y: "<< direction.y
            << " startTime: " << startTime
            << " speed: " << speed;
        return ost.str();
    }
};

#endif
