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
 * Assigns a pointer to ConnectionManager and gets a pointer to its host.
 *
 * Creates a random position for a host if the position is not given
 * as a parameter in "omnetpp.ini".
 *
 * Additionally the registration with ConnectionManager is done and it is
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
        
        // get utility pointers (world and host)
	world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == NULL)
            error("Could not find BaseWorldUtility module");

        coreEV << "initializing BaseUtility stage " << stage << endl; // for node position
	baseUtility = FindModule<BaseUtility*>::findSubModule(findHost());
        if (baseUtility == NULL)
            error("Could not find BaseUtility module");
        
        //get a pointer to the host
        hostPtr = findHost();
        hostId = hostPtr->id();

        
        if (hasPar("updateInterval")) {
        	updateInterval = par("updateInterval");
        } else {
            updateInterval = 0;
        }        

		// initialize Move parameter
        bool use2D = world->use2D();
        
        double x = -1.0;
        double y = -1.0;
        double z = -1.0;
        //read coordinates from parameters if available
        if (hasPar("x") && hasPar("y") && (hasPar("z") || use2D)){ 
            x = par("x");
            y = par("y");
            if(!use2D) {
                z = par("z");
            }      
		} 

        //a coordinate of -1.0 means random position
        if (x == -1.0 || y == -1.0 || (z == -1.0 && !use2D)) {
            move.startPos = world->getRandomPosition();
        } else {
            if (use2D) {
                move.startPos = Coord(x, y);
            } else {
                move.startPos = Coord(x, y, z);
            }
            
        }
		coreEV << "start pos: " << move.startPos.info() << endl;
        //check whether position is within the playground
        if (!move.startPos.isInRectangle(Coord(use2D), world->getPgs())) {
            error("node position specified in omnetpp.ini exceeds playgroundsize");    
        }        
            
        move.speed = 0;
        move.startTime = simTime();
        move.direction = Coord(use2D);
        
        //get BBItem category for Move
        moveCategory = baseUtility->getCategory(&move);

    }
    else if (stage == 1){
        coreEV << "initializing BaseMobility stage " << stage << endl;
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
    EV << "updatePosition: " << move.info() << endl;

    //publish the the new move
    baseUtility->publishBBItem(moveCategory, &move, hostId);
    
    char xStr[32], yStr[32], zStr[32];
    sprintf(xStr, "%d", FWMath::round(move.startPos.getX()));
    sprintf(yStr, "%d", FWMath::round(move.startPos.getY()));
    sprintf(zStr, "%d", FWMath::round(move.startPos.getZ()));
    hostPtr->displayString().setTagArg("p", 0, xStr);
    hostPtr->displayString().setTagArg("p", 1, yStr);

	/* p parameter *does not* accept z co-ordinates. Tk has a 2-d view */
	//hostPtr->displayString().setTagArg("p", 2, zStr);
}

/**
 * Helper function for BaseMobility::reflectIfOutside().
 *
 * Reflects a given coordinate according to the given
 * BorderHandling.
 *
 */
void BaseMobility::reflectCoordinate(BorderHandling border, Coord& c)
{
    switch( border ){
    case X_SMALLER:
        c.setX(-c.getX());
	    break;
    case X_BIGGER:
        c.setX(2 * playgroundSizeX() - c.getX());
	    break;
    case Y_SMALLER:
        c.setY(-c.getY());
	    break;
    case Y_BIGGER:
        c.setY(2 * playgroundSizeY() - c.getY());
        break;
    case Z_SMALLER:
        c.setZ(-c.getZ());
        break;
    case Z_BIGGER:
        c.setZ(2 * playgroundSizeZ() - c.getZ());
	    break;
    case NOWHERE:
    default:
	    error("wrong border handling case!");
    }
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

    reflectCoordinate(wo, targetPos);
    reflectCoordinate(wo, stepTarget);

    switch( wo ){
    case X_SMALLER:
    case X_BIGGER:
        step.setX(-step.getX());
	    angle = 180 - angle;
	    break;
    case Y_SMALLER:
    case Y_BIGGER:
        step.setY(-step.getY());
	    angle = -angle;
	    break;
    case Z_SMALLER:
    case Z_BIGGER:
        step.setZ(-step.getZ());
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
    case X_BIGGER:
        targetPos.setX(fmod(targetPos.getX(), playgroundSizeX()));
        stepTarget.setX(fmod(stepTarget.getX(), playgroundSizeX()));
	    break;
    case Y_SMALLER:
    case Y_BIGGER:
        targetPos.setY(fmod(targetPos.getY(), playgroundSizeY()));
        stepTarget.setY(fmod(stepTarget.getY(), playgroundSizeY()));
	    break;
    case Z_SMALLER:
    case Z_BIGGER:
        targetPos.setZ(fmod(targetPos.getZ(), playgroundSizeZ()));
        stepTarget.setZ(fmod(stepTarget.getZ(), playgroundSizeZ()));
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
    targetPos = world->getRandomPosition();
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

    if (targetPos.getX() < 0){
	borderStep.setX(-move.startPos.getX());
	outside = X_SMALLER;
    }
    else if (targetPos.getX() >= playgroundSizeX()){
        borderStep.setX(playgroundSizeX() - move.startPos.getX());
	outside = X_BIGGER;
    }

    if (targetPos.getY() < 0){
	borderStep.setY(-move.startPos.getY());

	if( outside==NOWHERE || fabs(borderStep.getX()/move.direction.getX()) > fabs(borderStep.getY()/move.direction.getY()) )
	    outside = Y_SMALLER;
    }
    else if (targetPos.getY() >= playgroundSizeY()){
        borderStep.setY(playgroundSizeY() - move.startPos.getY());

	if( outside==NOWHERE || fabs(borderStep.getX()/move.direction.getX()) > fabs(borderStep.getY()/move.direction.getY()) )
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
	factor = borderStep.getX() / move.direction.getX();
	borderStep.setY(factor * move.direction.getY());

	if( policy == WRAP ){
	    borderStart.setX(playgroundSizeX());
	    borderStart.setY(move.startPos.getY() + borderStep.getY());
	}
	break;
    case X_BIGGER:
	factor = borderStep.getX() / move.direction.getX();
	borderStep.setY(factor * move.direction.getY());

	if( policy == WRAP ){
	    borderStart.setX(0);
	    borderStart.setY(move.startPos.getY() + borderStep.getY());
	}
	break;
    case Y_SMALLER:
	factor = borderStep.getY() / move.direction.getY();
	borderStep.setX(factor * move.direction.getX());

	if( policy == WRAP ){
	    borderStart.setY(playgroundSizeY());
	    borderStart.setX(move.startPos.getX() + borderStep.getX());
	}
	break;
    case Y_BIGGER:
	factor = borderStep.getY() / move.direction.getY();
	borderStep.setX(factor * move.direction.getX());

	if( policy == WRAP ){
	    borderStart.setY(0);
	    borderStart.setX(move.startPos.getX() + borderStep.getX());
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
    borderInterval = sqrt(borderStep.getX()*borderStep.getX() + borderStep.getY()*borderStep.getY()) / move.speed;

    // calculate new start position
    // NOTE: for WRAP this is done in goToBorder
    switch( policy ){
    case REFLECT:
	double d;

	borderStart = move.startPos + borderStep;
	d = stepTarget.distance( borderStart );
	borderDirection.setX((stepTarget.getX() - borderStart.getX()) / d);
	borderDirection.setY((stepTarget.getY() - borderStart.getY()) / d);
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

