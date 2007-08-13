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
    
public:
    
    std::string info() {
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

