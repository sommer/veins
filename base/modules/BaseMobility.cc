/* -*- mode:c++ -*- ********************************************************
 * file:        BaseMobility.cc
 *
 * author:      Daniel Willkomm, Andras Varga
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              (C) 2005 Andras Varga
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


#include "BaseMobility.h"
#include "FWMath.h"
#include "BorderMsg_m.h"

Define_Module(BaseMobility);

/**
 * Assigns a pointer to ChannelControl and gets a pointer to its host.
 *
 * Creates a random position for a host if the position is not given
 * as a parameter in "omnetpp.ini".
 *
 * Additionally the registration with ChannelControl is done and it is
 * assured that the position display string tag (p) exists and contains
 * the exact (x) tag.
 *
 * If the speed of the host is bigger than 0 a first MOVE_HOST self
 * message is scheduled in stage 1
 */

void BaseMobility::initialize(int stage)
{
    BaseModule::initialize(stage);

    if (stage == 0){
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
        coreEV << "initializing BaseMobility stage " << stage << endl;
		cc = (ChannelControl*)getGlobalModule("ChannelControl");
        if (cc == NULL)
            error("Could not find channelcontrol module");

        //get a pointer to the host
        hostPtr = findHost();
        hostId = hostPtr->id();

        hasPar("updateInterval") ? updateInterval=par("updateInterval") :
            updateInterval = 0;

        //moveCategory = bb->getCategory(&move);

        // reading the out from omnetpp.ini makes predefined scenarios a lot easier
        if (hasPar("x") && hasPar("y") && hasPar("z")){
            move.startPos.x = par("x");
            move.startPos.y = par("y");
            move.startPos.z = par("z");
        }
        else{
            // Start at a random position
	    move.startPos.x = move.startPos.y = move.startPos.z = -1;
        }

	coreEV << "move.startPos: " << move.startPos.info() << endl;

	// initialize Move parameter
        move.speed = 0;
        move.startTime = simTime();
        move.direction = Coord(0,0);

    }
    else if (stage == 1){
        coreEV << "initializing BaseMobility stage " << stage << endl;
        // playground size gets set up by ChannelControl in stage==0 (Andras)
        // read the playgroundsize from ChannelControl
        Coord pgs =  cc->getPgs();

	coreEV << "move.startPos: " << move.startPos.info() << endl;

	// -1 indicates start at random position
	if (move.startPos.x == -1 || move.startPos.y == -1)
	    move.startPos = getRandomPosition();
	//we do not have negative positions
	//also checks whether position is within the playground
	else if (	move.startPos.x < 0 || move.startPos.y < 0 || move.startPos.z < 0 ||
			move.startPos.x > pgs.x || move.startPos.y > pgs.y || move.startPos.z > pgs.z)
	    error("node position specified in omnetpp.ini exceeds playgroundsize");

        // print new host position on the screen and update bb info
        updatePosition();

        if (move.speed > 0 && updateInterval > 0) {
	    coreEV << "Host is moving, speed=" << move.speed << " updateInterval=" << updateInterval << endl;
	    moveMsg = new cMessage("move", MOVE_HOST);
	    //host moves the first time after some random delay to avoid synchronized movements
            scheduleAt(simTime() + uniform(0, updateInterval), moveMsg);
        }
    }
}


/**
 * Dispatches border messages to handleBorderMsg() and all other
 * self-messages to handleSelfMsg()
 */
void BaseMobility::handleMessage(cMessage * msg)
{
    if (!msg->isSelfMessage())
        error("mobility modules can only receive self messages");


    if(msg->kind() == MOVE_TO_BORDER){
	handleBorderMsg(msg);
    }
    else{
	handleSelfMsg(msg);
    }
}


/**
 * The only self message possible is to indicate a new movement. If
 * the host is stationary this function is never called.
 *
 * every time a self message arrives makeMove is called to handle the
 * movement. Afterward updatePosition updates the position with the
 * blackboard and the display.
 */
void BaseMobility::handleSelfMsg(cMessage * msg)
{
    makeMove();
    updatePosition();

    if( !moveMsg->isScheduled() && move.speed > 0)
	scheduleAt(simTime() + updateInterval, msg);
    else{
	delete msg;
	moveMsg = NULL;
    }
}


/**
 * The host actually reached the border, so the startPos has to be
 * updated.
 *
 * Additionally fixIfHostGetsOutside has to be called again to catch
 * cases where the host moved in both (x and y) direction outside the
 * playground.
 **/
void BaseMobility::handleBorderMsg(cMessage * msg)
{
    coreEV << "start MOVE_TO_BORDER:" << move.info() << endl;

    BorderMsg* bMsg = static_cast<BorderMsg*>(msg);

    switch( bMsg->getPolicy() ){
    case REFLECT:
	move.startPos = bMsg->getStartPos();
	move.direction = bMsg->getDirection();
	break;
    case WRAP:
	move.startPos = bMsg->getStartPos();
	break;
    case PLACERANDOMLY:
	move.startPos = bMsg->getStartPos();
	coreEV << "new random position: " << move.startPos.info() << endl;
	break;
    case RAISEERROR:
	error("node moved outside the playground");		      
	break;
    default:
	error("Unknown BorderPolicy!");
    }

    fixIfHostGetsOutside();

    updatePosition();

    delete bMsg;

    coreEV << "end MOVE_TO_BORDER:" << move.info() << endl;
}


/**
 * This function tells the Blackboard that the position has changed,
 * and it also moves the host's icon to the new position on the
 * screen.
 *
 * This function has to be called every time the position of the host
 * changes!
 */
void BaseMobility::updatePosition() {
    coreEV << "updatePosition: " << move.info() << endl;
    
    //bb->publishBBItem(moveCategory, &move, hostId);
    char xStr[32], yStr[32], zStr[32];
    sprintf(xStr, "%d", FWMath::round(move.startPos.x));
    sprintf(yStr, "%d", FWMath::round(move.startPos.y));
    sprintf(zStr, "%d", FWMath::round(move.startPos.z));
    hostPtr->displayString().setTagArg("p", 0, xStr);
    hostPtr->displayString().setTagArg("p", 1, yStr);
    //hostPtr->displayString().setTagArg("p", 2, zStr);
}


/**
 * You can redefine this function if you want to use another
 * calculation
 */
Coord BaseMobility::getRandomPosition() {
    Coord p;
    p.x = genk_uniform(0, 0, cc->getPgs()->x);
    p.y = genk_uniform(0, 0, cc->getPgs()->y);
    p.z = genk_uniform(0, 0, cc->getPgs()->z);
    return p;
}


/**
 * Reflects the host from the playground border.
 *
 * This function can update the target position, the step (for non
 * atomic movements) and the angle.
 *
 * @param stepTarget target position of the current step of the host
 * @param targetPos target position of the host (for non atomic movements)
 * @param step step size and direction of the host (for non atomic movements)
 * @param angle direction to which the host is moving
 **/
void BaseMobility::reflectIfOutside(BorderHandling wo, Coord& stepTarget, Coord& targetPos, Coord& step, double& angle) {
    switch( wo ){
    case X_SMALLER:
        targetPos.x = -targetPos.x;
        stepTarget.x = -stepTarget.x;
        step.x = -step.x;
	angle = 180 - angle;
	break;
    case X_BIGGER:
        targetPos.x = 2*playgroundSizeX() - targetPos.x;
        stepTarget.x = 2*playgroundSizeX() - stepTarget.x;
        step.x = -step.x;
	angle = 180 - angle;
	break;
    case Y_SMALLER:
        targetPos.y = -targetPos.y;
        stepTarget.y = -stepTarget.y;
        step.y = -step.y;
	angle = -angle;
	break;
    case Y_BIGGER:
        targetPos.y = 2*playgroundSizeY() - targetPos.y;
        stepTarget.y = 2*playgroundSizeY() - stepTarget.y;
        step.y = -step.y;
	angle = -angle;
	break;
    case Z_SMALLER:
        targetPos.z = -targetPos.z;
        stepTarget.z = -stepTarget.z;
        step.z = -step.z;
	angle = -angle;
	break;
    case Z_BIGGER:
        targetPos.z = 2*playgroundSizeZ() - targetPos.z;
        stepTarget.z = 2*playgroundSizeZ() - stepTarget.z;
        step.z = -step.z;
	angle = -angle;
	break;
    case NOWHERE:
    default:
	error("wrong border handling case!");
    }
}


/**
 * Wraps the host to the other playground size. Updates the target
 * position.
 *
 * @param stepTarget target position of the current step of the host
 * @param targetPos target position of the host (for non atomic movements)
 **/
void BaseMobility::wrapIfOutside(BorderHandling wo, Coord& stepTarget, Coord& targetPos) {
    switch( wo ){
    case X_SMALLER:
        targetPos.x += playgroundSizeX();
        stepTarget.x += playgroundSizeX();
	break;
    case X_BIGGER:
        targetPos.x -= playgroundSizeX();
        stepTarget.x -= playgroundSizeX();
	break;
    case Y_SMALLER:
        targetPos.y += playgroundSizeY();
        stepTarget.y += playgroundSizeY();
	break;
    case Y_BIGGER:
        targetPos.y -= playgroundSizeY();
        stepTarget.y -= playgroundSizeY();
	break;
    case Z_SMALLER:
        targetPos.z += playgroundSizeZ();
        stepTarget.z += playgroundSizeZ();
	break;
    case Z_BIGGER:
        targetPos.z -= playgroundSizeZ();
        stepTarget.z -= playgroundSizeZ();
	break;
    case NOWHERE:
    default:
	error("wrong border handling case!");
    }
}

/**
 * Start the host at a new random position. Here the target position
 * is set to the new start position. 
 *
 * You have to define a new target postion in fixIfHostGetsOutside to
 * keep the host moving.
 **/
void BaseMobility::placeRandomlyIfOutside( Coord& targetPos )
{
    targetPos = getRandomPosition();
}


/**
 * Checks whether the host moved outside and return the border it
 * crossed.
 *
 * Additionally the calculation of the step to reach the border is
 * started.
 * TODO fix for 3D
 **/
BaseMobility::BorderHandling BaseMobility::checkIfOutside( Coord targetPos, Coord& borderStep )
{
    BorderHandling outside = NOWHERE;

    if (targetPos.x < 0){
	borderStep.x = -move.startPos.x;
	outside = X_SMALLER;
    }
    else if (targetPos.x >= playgroundSizeX()){
        borderStep.x = playgroundSizeX() - move.startPos.x;
	outside = X_BIGGER;
    }

    if (targetPos.y < 0){
	borderStep.y = -move.startPos.y;

	if( outside==NOWHERE || fabs(borderStep.x/move.direction.x) > fabs(borderStep.y/move.direction.y) )
	    outside = Y_SMALLER;
    }
    else if (targetPos.y >= playgroundSizeY()){
        borderStep.y = playgroundSizeY() - move.startPos.y;

	if( outside==NOWHERE || fabs(borderStep.x/move.direction.x) > fabs(borderStep.y/move.direction.y) )
	    outside = Y_BIGGER;
    }

    coreEV << "checkIfOutside, outside="<<outside<<" borderStep: " << borderStep.info() << endl;

    return outside;
}


/**
 * Calculate the step to reach the border. Additionally for the WRAP
 * policy the new start position after reaching the border is
 * calculated.
 *
 * TODO fix for 3D
 **/
void BaseMobility::goToBorder(BorderPolicy policy, BorderHandling wo, Coord& borderStep, Coord& borderStart)
{
    double factor;

    coreEV << "goToBorder: startPos: "<< move.startPos.info() << " borderStep: " << borderStep.info() 
	   << " BorderPolicy: " << policy << " BorderHandling: " << wo << endl;	

    switch( wo ){
    case X_SMALLER:
	factor = borderStep.x / move.direction.x;
	borderStep.y = factor * move.direction.y;

	if( policy == WRAP ){
	    borderStart.x = playgroundSizeX();
	    borderStart.y = move.startPos.y + borderStep.y;
	}
	break;
    case X_BIGGER:
	factor = borderStep.x / move.direction.x;
	borderStep.y = factor * move.direction.y;

	if( policy == WRAP ){
	    borderStart.x = 0;
	    borderStart.y = move.startPos.y + borderStep.y;
	}
	break;
    case Y_SMALLER:
	factor = borderStep.y / move.direction.y;
	borderStep.x = factor * move.direction.x;

	if( policy == WRAP ){
	    borderStart.y = playgroundSizeY();
	    borderStart.x = move.startPos.x + borderStep.x;
	}
	break;
    case Y_BIGGER:
	factor = borderStep.y / move.direction.y;
	borderStep.x = factor * move.direction.x;

	if( policy == WRAP ){
	    borderStart.y = 0;
	    borderStart.x = move.startPos.x + borderStep.x;
	}
	break;
    default:
        factor = 0;
	error("invalid state in goToBorder switch!");
    }
	
    coreEV << "goToBorder: startPos: "<< move.startPos.info() << " borderStep: " << borderStep.info() 
	   << " borderStart: " << borderStart.info() << " factor: " << factor << endl;	
}


/**
 * This function takes the BorderPolicy and all varaibles to be
 * modified in case a border is reached and invokes the appropriate
 * action. Pass dummy variables if you do not need them.
 *
 * The supproted border policies are REFLECT, WRAP, PLACERANOMLY, and
 * RAISEERROR.
 *
 * The policy and stepTarget are mandatory parameters to
 * pass. stepTarget is used to check whether the host actually moved
 * outside the playground. 
 *
 * Additional parameters to pass (in case of non atomic movements) can
 * be targetPos (the target the host is moving to) and step (the size
 * of a step).
 *
 * Angle is the direction in which the host is moving.
 *
 * @param policy BorderPolicy to use
 * @param stepTarget target position of the next step of the host
 * @param targetPos target position of the host (for non atomic movement)
 * @param step step size of the host (for non atomic movement)
 * @param angle direction in which the host is moving
 *
 * @return true if host was outside, false otherwise.
 *
 * TODO fix for 3D
 **/
bool BaseMobility::handleIfOutside(BorderPolicy policy, Coord& stepTarget, Coord& targetPos, Coord& step, double& angle) {
    // where did the host leave the playground?
    BorderHandling wo;

    // step to reach the border
    Coord borderStep;

    wo = checkIfOutside(stepTarget, borderStep);

    // just return if the next step is not outside the playground
    if( wo == NOWHERE )
	return false;

    coreEV << "handleIfOutside:stepTarget = " << stepTarget.info() << endl;

    // new start position after the host reaches teh border
    Coord borderStart;
    // new direction the host has to move to
    Coord borderDirection;
    // time to reach the border
    double borderInterval;

    coreEV << "old values: stepTarget: " << stepTarget.info() << " step: " << step.info() 
	   << " targetPos: " << targetPos.info() << " angle: " << angle << endl;

    // which border policy is to be followed
    switch (policy){
    case REFLECT:       
	reflectIfOutside( wo, stepTarget, targetPos, step, angle ); 
	break;
    case WRAP:          
	wrapIfOutside( wo, stepTarget, targetPos ); 
	break;
    case PLACERANDOMLY: 
	placeRandomlyIfOutside( targetPos ); 
	break;
    case RAISEERROR: 
	break;
    }

    coreEV << "new values: stepTarget: " << stepTarget.info() << " step: " << step.info() << " angle: " << angle << endl;

    // calculate the step to reach the border
    goToBorder(policy, wo, borderStep, borderStart);

    // calculate teh time to reach the border
    borderInterval = sqrt(borderStep.x*borderStep.x + borderStep.y*borderStep.y) / move.speed;

    // calculate new start position
    // NOTE: for WRAP this is done in goToBorder
    switch( policy ){
    case REFLECT:
	double d;

	borderStart = move.startPos + borderStep;
	d = stepTarget.distance( borderStart );
	borderDirection.x = (stepTarget.x - borderStart.x) / d;
	borderDirection.y = (stepTarget.y - borderStart.y) / d;
	break;
    case PLACERANDOMLY:
	borderStart = targetPos;
	stepTarget = targetPos;
	break;
    case WRAP:
    case RAISEERROR:
	break;
    default:
	error("unknown BorderPolicy");
    }

    coreEV << "border handled, borderStep: "<< borderStep.info() << "borderStart: " << borderStart.info()
	   << " stepTarget " << stepTarget.info() << endl;

    // create a border self message and schedule it appropriately
    BorderMsg *bMsg = new BorderMsg("borderMove", MOVE_TO_BORDER);
    bMsg->setPolicy(policy);
    bMsg->setStartPos(borderStart);
    bMsg->setDirection(borderDirection);

    scheduleAt(simTime() + borderInterval, bMsg);

    return true;
}

