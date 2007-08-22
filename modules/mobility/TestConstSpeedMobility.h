/* -*- mode:c++ -*- ********************************************************
 * file:        TestConstSpeedMobility.h
 *
 * author:      Steffen Sroka
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
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


#ifndef TEST_CONST_SPEED_MOBILITY_H
#define TEST_CONST_SPEED_MOBILITY_H

#include <TestBaseMobility.h>


/**
 * @brief Controls all movement related things of a host
 *
 * Parameters to be specified in omnetpp.ini
 *  - vHost : Speed of a host [m/s]
 *  - updateInterval : Time interval to update the hosts position
 *  - x, y : Starting position of the host, -1 = random
 *
 * @ingroup mobility
 * @author Steffen Sroka, Marc Loebbers, Daniel Willkomm
 * @sa ConnectionManager
 */
class TestConstSpeedMobility : public TestBaseMobility
{
  protected:
    /** @brief parameters to handle the movement of the host*/
    /*@{*/
    /** @brief Size of a step*/
    Coord stepSize;
    /** @brief Total number of steps */
    int numSteps;
    /** @brief Number of steps already moved*/
    int step;
    /*@}*/

    Coord targetPos;
    Coord stepTarget;
    
    //    double lastStep;

  public:
    Module_Class_Members( TestConstSpeedMobility, TestBaseMobility, 0 );

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

  protected:
    /** @brief Calculate the target position to move to*/
    virtual void setTargetPosition();

    /** @brief Move the host*/
    virtual void makeMove();

    //    void fixIfHostGetsOutside();
};

#endif
