/* -*- mode:c++ -*- ********************************************************
 * file:        MoBANLocal.cc
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
 *
 *              http://www.es.ele.tue.nl/nes
 *
 * copyright:   (C) 2010 Electronic Systems group(ES),
 *              Eindhoven University of Technology (TU/e), the Netherlands.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:    MoBAN (Mobility Model for wireless Body Area Networks)
 * description:     Implementation of the local module of the MoBAN mobility model
 ***************************************************************************
 * Citation of the following publication is appreciated if you use MoBAN for
 * a publication of your own.
 *
 * M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
 * In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
 *
 * BibTeX:
 *		@inproceedings{MoBAN,
 * 		author = "M. Nabi and M. Geilen and T. Basten.",
 * 	 	title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
 *    	booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
 *    	series = {SIMUTools '11},
 *    	isbn = {978-963-9799-41-7},
 *	    year = {2011},
 *    	location = {Barcelona, Spain},
 *	    publisher = {ICST} }
 *
 **************************************************************************/
#include "MoBANLocal.h"

#include "FWMath.h"
#include "MoBANBBItem.h"

Define_Module(MoBANLocal);

const simsignalwrap_t MoBANLocal::catBBMoBANMsgSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBANMSG_NAME);

void MoBANLocal::initialize(int stage)
{
    BaseMobility::initialize(stage);
    if (stage == 0) {

		speed = 1;
    	move.setSpeed(speed);//It should be done in the first stage for sure. If it is not set, then no move message will be initiated. Any value is okay, but Zero!
    	numSteps = 0;
        step = -1;
        stepSize = Coord(0,0,0);

        findHost()->subscribe(catBBMoBANMsgSignal, this);
    }
    else if( stage == 1 ){
    	referencePoint = move.getStartPos();
    }
}

/**
 * Calculate a new random position within a sphere around the reference point with the given radius.
 * It also calculates the number of steps the node needs to reach this position with the given speed
 */
void MoBANLocal::setTargetPosition()
{
	Coord currentRelativePosition;

	if (speed != 0)
	{
		// Find a uniformly random position within a sphere around the reference point
		double x  = uniform(-1*radius, radius);
		double y  = uniform(-1*radius, radius);
		double z =  uniform(-1*radius, radius);
		while (x*x+y*y+z*z > radius*radius)
		{
			x  = uniform(-1*radius, radius);
			y  = uniform(-1*radius, radius);
			z =  uniform(-1*radius, radius);
		}

		targetPos.x = (x);
		targetPos.y = (y);
		targetPos.z = (z);

		currentRelativePosition = move.getStartPos()- referencePoint;
		double distance = currentRelativePosition.distance(targetPos);

		simtime_t totalTime = distance / move.getSpeed();
		if (totalTime >= updateInterval)
			numSteps = FWMath::round(totalTime / updateInterval);
		else
			numSteps = 1; //The selected target position is quite close so that in less than one step, it will be reached with the given velocity.

		stepSize = (targetPos - currentRelativePosition)/numSteps;
		stepTarget = (move.getStartPos()-referencePoint) + stepSize;
	}
	else
	{
		numSteps = 1;
		targetPos = referencePoint;
		stepSize = Coord(0,0,0);
		stepTarget = Coord(0,0,0);
	}


	move.setStart(insideWorld(stepTarget + referencePoint),simTime());

    step = 0;

	EV << "new targetPos: " << targetPos.info() << " numSteps=" << numSteps << endl;

}

/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position. This function is called by the
 * BaseMobility module considering the given update time interval.
*/
void MoBANLocal::makeMove()
{
    // increment number of steps
    step++;

    if( step == numSteps ){
    	EV << "destination reached. " << move.info() << endl;
    	setTargetPosition();
	}
    else if( step < numSteps ){
    	// step forward
    	stepTarget += stepSize;
    	move.setStart(insideWorld(stepTarget+referencePoint),simTime());
    }
    else{
    	error("step cannot be bigger than numSteps");
    }

}

/**
* This function is called once something is written to the signaling system of this node. If the written item has specific category dedicated for the
* type that MoBAN coordinator writes, the item will be read and the corresponding variables are set.
*/
void MoBANLocal::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if(signalID == catBBMoBANMsgSignal)
    {
    	BBMoBANMessage m(*static_cast<const BBMoBANMessage*>(obj));
    	referencePoint= m.position;

    	speed = m.speed;
    	if ( !FWMath::close(speed,0.0) )
    		 move.setSpeed(speed); // IF WE SET ZERO, it is not going to move anymore

        radius = m.radius;

    	move.setStart(insideWorld(stepTarget+referencePoint),simTime());

        EV<<"Node "<< getParentModule()->getIndex() <<" received new reference point."<<endl;
        EV<< "New speed:" << speed <<" , new radius: "<< radius <<endl;

    }
}

/**
 * Gets a position and return the nearest point inside the simulation area if the point is outside the area
*/
Coord MoBANLocal::insideWorld(Coord apoint)
{
	double xmax, ymax, zmax;

	Coord NearestBorder = apoint;

	xmax = world->getPgs()->x;
	ymax = world->getPgs()->y;
	zmax = world->getPgs()->z;

	if (NearestBorder.x < 0)
		NearestBorder.x = (0.0);

	if (NearestBorder.y < 0)
		NearestBorder.y = (0.0);

	if (NearestBorder.z < 0)
		NearestBorder.z = (0.0);

	if (NearestBorder.x > xmax)
		NearestBorder.x = (xmax);

	if (NearestBorder.y > ymax)
		NearestBorder.y = (ymax);

	if (NearestBorder.z > zmax)
		NearestBorder.z = (zmax);

	return NearestBorder;

}

