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

        //get a pointer to the host
        hostPtr = findHost();
        hostId = hostPtr->getId();


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

        // set start-position and start-time (i.e. current simulation-time) of the Move
        //a coordinate of -1.0 means random position
        if (x == -1.0 || y == -1.0 || (z == -1.0 && !use2D)) {
            move.setStart(world->getRandomPosition());
        } else {
            if (use2D) {
                move.setStart(Coord(x, y));
            } else {
                move.setStart(Coord(x, y, z));
            }

        }
		coreEV << "start pos: " << move.getStartPos().info() << endl;
        //check whether position is within the playground
        if (!move.getStartPos().isInRectangle(Coord(use2D), world->getPgs())) {
            error("node position specified in omnetpp.ini exceeds playgroundsize");
        }

        // set speed and direction of the Move
        move.setSpeed(0);
        move.setDirectionByVector(Coord(use2D));

        //get BBItem category for Move
        moveCategory = utility->getCategory(&move);

    }
    else if (stage == 1){
        coreEV << "initializing BaseMobility stage " << stage << endl;
        // print new host position on the screen and update bb info
        updatePosition();

        if (move.getSpeed() > 0 && updateInterval > 0) {
	    coreEV << "Host is moving, speed=" << move.getSpeed() << " updateInterval=" << updateInterval << endl;
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


    if(msg->getKind() == MOVE_TO_BORDER){
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

    if( !moveMsg->isScheduled() && move.getSpeed() > 0)
	scheduleAt(simTime() + updateInterval, msg);
    else{
	delete msg;
	moveMsg = NULL;
    }
}


/**
 * The host actually reached the border, so the startPos and startTime
 * has to be updated.
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
	move.setStart(bMsg->getStartPos());
	move.setDirectionByVector(bMsg->getDirection());
	break;
    case WRAP:
	move.setStart(bMsg->getStartPos());
	break;
    case PLACERANDOMLY:
	move.setStart(bMsg->getStartPos());
	coreEV << "new random position: " << move.getStartPos().info() << endl;
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
    utility->publishBBItem(moveCategory, &move, hostId);

    if(ev.isGUI())
    {
		char xStr[32], yStr[32];
		sprintf(xStr, "%d", FWMath::round(move.getStartPos().getX()));
		sprintf(yStr, "%d", FWMath::round(move.getStartPos().getY()));

		cDisplayString& disp = hostPtr->getDisplayString();
		disp.setTagArg("p", 0, xStr);
		disp.setTagArg("p", 1, yStr);

		if(!world->use2D())
		{
			//scale host dependent on their z coordinate to
			//simulate a depth effect
			//z-coordinate of zero maps to a size of 50 (16+34) (very close)
			//z-coordinate of playground size z maps to size of 16 (far away)
			double width = 16.0 + 34.0 *  ((1.0 - move.getStartPos().getZ()/playgroundSizeZ()));

			char sizeStr[32];
			sprintf(sizeStr, "%d", FWMath::round(width));
			disp.setTagArg("b", 0, sizeStr);
			disp.setTagArg("b", 1, sizeStr);

			//choose a appropriate icon size
			if(width >= 40)
				disp.setTagArg("is", 0, "n");
			else if(width >= 24)
				disp.setTagArg("is", 0, "s");
			else
				disp.setTagArg("is", 0, "vs");

		}
    }
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
        targetPos.setX(FWMath::modulo(targetPos.getX(), playgroundSizeX()));
        stepTarget.setX(FWMath::modulo(stepTarget.getX(), playgroundSizeX()));
	    break;
    case Y_SMALLER:
    case Y_BIGGER:
        targetPos.setY(FWMath::modulo(targetPos.getY(), playgroundSizeY()));
        stepTarget.setY(FWMath::modulo(stepTarget.getY(), playgroundSizeY()));
	    break;
    case Z_SMALLER:
    case Z_BIGGER:
        targetPos.setZ(FWMath::modulo(targetPos.getZ(), playgroundSizeZ()));
        stepTarget.setZ(FWMath::modulo(stepTarget.getZ(), playgroundSizeZ()));
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
 **/
BaseMobility::BorderHandling BaseMobility::checkIfOutside( Coord targetPos, Coord& borderStep )
{
    BorderHandling outside = NOWHERE;

    // Testing x-value
    if (targetPos.getX() < 0){
	borderStep.setX(-move.getStartPos().getX());
	outside = X_SMALLER;
    }
    else if (targetPos.getX() >= playgroundSizeX()){
        borderStep.setX(playgroundSizeX() - move.getStartPos().getX());
	outside = X_BIGGER;
    }

    // Testing y-value
    if (targetPos.getY() < 0){
	borderStep.setY(-move.getStartPos().getY());

	if( outside==NOWHERE || fabs(borderStep.getX()/move.getDirection().getX()) > fabs(borderStep.getY()/move.getDirection().getY()) )
	    outside = Y_SMALLER;
    }
    else if (targetPos.getY() >= playgroundSizeY()){
        borderStep.setY(playgroundSizeY() - move.getStartPos().getY());

	if( outside==NOWHERE || fabs(borderStep.getX()/move.getDirection().getX()) > fabs(borderStep.getY()/move.getDirection().getY()) )
	    outside = Y_BIGGER;
    }

    // Testing z-value
    if (!world->use2D())
    {
	    // going to reach the lower z-border
    	if (targetPos.getZ() < 0)
	    {
	    	borderStep.setZ(-move.getStartPos().getZ());

	    	// no border reached so far
	    	if( outside==NOWHERE )
	    	{
	    		outside = Z_SMALLER;
	    	}
	    	// an y-border is reached earliest so far, test whether z-border is reached even earlier
	    	else if( (outside==Y_SMALLER || outside==Y_BIGGER) &&
	    			fabs(borderStep.getY()/move.getDirection().getY()) > fabs(borderStep.getZ()/move.getDirection().getZ()) )
	    	{
	    		outside = Z_SMALLER;
	    	}
	    	// an x-border is reached earliest so far, test whether z-border is reached even earlier
	    	else if( (outside==X_SMALLER || outside==X_BIGGER) &&
	    			fabs(borderStep.getX()/move.getDirection().getX()) > fabs(borderStep.getZ()/move.getDirection().getZ()) )
	    	{
	    		outside = Z_SMALLER;
	    	}

	    }
    	// going to reach the upper z-border
	    else if (targetPos.getZ() >= playgroundSizeZ())
	    {
	        borderStep.setZ(playgroundSizeZ() - move.getStartPos().getZ());

	        // no border reached so far
	        if( outside==NOWHERE )
	        {
	        	outside = Z_BIGGER;
	        }
	        // an y-border is reached earliest so far, test whether z-border is reached even earlier
	        else if( (outside==Y_SMALLER || outside==Y_BIGGER) &&
	    			fabs(borderStep.getY()/move.getDirection().getY()) > fabs(borderStep.getZ()/move.getDirection().getZ()) )
	    	{
	    		outside = Z_BIGGER;
	    	}
	        // an x-border is reached earliest so far, test whether z-border is reached even earlier
	    	else if( (outside==X_SMALLER || outside==X_BIGGER) &&
	    			fabs(borderStep.getX()/move.getDirection().getX()) > fabs(borderStep.getZ()/move.getDirection().getZ()) )
	    	{
	    		outside = Z_BIGGER;
	    	}


	    }
    }

    coreEV << "checkIfOutside, outside="<<outside<<" borderStep: " << borderStep.info() << endl;

    return outside;
}


/**
 * Calculate the step to reach the border. Additionally for the WRAP
 * policy the new start position after reaching the border is
 * calculated.
 **/
void BaseMobility::goToBorder(BorderPolicy policy, BorderHandling wo, Coord& borderStep, Coord& borderStart)
{
    double factor;

    coreEV << "goToBorder: startPos: "<< move.getStartPos().info() << " borderStep: " << borderStep.info()
	   << " BorderPolicy: " << policy << " BorderHandling: " << wo << endl;

    switch( wo ){
    case X_SMALLER:
	factor = borderStep.getX() / move.getDirection().getX();
	borderStep.setY(factor * move.getDirection().getY());
	if (!world->use2D())
	{
		borderStep.setZ(factor * move.getDirection().getZ()); // 3D case
	}


	if( policy == WRAP ){
	    borderStart.setX(playgroundSizeX());
	    borderStart.setY(move.getStartPos().getY() + borderStep.getY());
	    if (!world->use2D())
	    {
	    	borderStart.setZ(move.getStartPos().getZ() + borderStep.getZ()); // 3D case
	    }
	}
	break;
    case X_BIGGER:
	factor = borderStep.getX() / move.getDirection().getX();
	borderStep.setY(factor * move.getDirection().getY());
	if (!world->use2D())
	{
		borderStep.setZ(factor * move.getDirection().getZ()); // 3D case
	}

	if( policy == WRAP ){
	    borderStart.setX(0);
	    borderStart.setY(move.getStartPos().getY() + borderStep.getY());
	    if (!world->use2D())
	    {
	    	borderStart.setZ(move.getStartPos().getZ() + borderStep.getZ()); // 3D case
	    }
	}
	break;
    case Y_SMALLER:
	factor = borderStep.getY() / move.getDirection().getY();
	borderStep.setX(factor * move.getDirection().getX());
	if (!world->use2D())
	{
		borderStep.setZ(factor * move.getDirection().getZ()); // 3D case
	}

	if( policy == WRAP ){
	    borderStart.setY(playgroundSizeY());
	    borderStart.setX(move.getStartPos().getX() + borderStep.getX());
	    if (!world->use2D())
	    {
	    	borderStart.setZ(move.getStartPos().getZ() + borderStep.getZ()); // 3D case
	    }
	}
	break;
    case Y_BIGGER:
	factor = borderStep.getY() / move.getDirection().getY();
	borderStep.setX(factor * move.getDirection().getX());
	if (!world->use2D())
	{
		borderStep.setZ(factor * move.getDirection().getZ()); // 3D case
	}

	if( policy == WRAP ){
	    borderStart.setY(0);
	    borderStart.setX(move.getStartPos().getX() + borderStep.getX());
	    if (!world->use2D())
	    {
	    	borderStart.setZ(move.getStartPos().getZ() + borderStep.getZ()); // 3D case
	    }
	}
	break;
    case Z_SMALLER: // here we are definitely in 3D
	factor = borderStep.getZ() / move.getDirection().getZ();
	borderStep.setX(factor * move.getDirection().getX());
	borderStep.setY(factor * move.getDirection().getY());

	if( policy == WRAP )
	{
	    borderStart.setZ(playgroundSizeZ());
	    borderStart.setX(move.getStartPos().getX() + borderStep.getX());
	    borderStart.setY(move.getStartPos().getY() + borderStep.getY());
	}
    break;
    case Z_BIGGER: // here we are definitely in 3D
	factor = borderStep.getZ() / move.getDirection().getZ();
	borderStep.setX(factor * move.getDirection().getX());
	borderStep.setY(factor * move.getDirection().getY());

	if( policy == WRAP )
	{
	    borderStart.setZ(0);
	    borderStart.setX(move.getStartPos().getX() + borderStep.getX());
	    borderStart.setY(move.getStartPos().getY() + borderStep.getY());
	}
    break;
    default:
        factor = 0;
	error("invalid state in goToBorder switch!");
    }

    coreEV << "goToBorder: startPos: "<< move.getStartPos().info() << " borderStep: " << borderStep.info()
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
 **/
bool BaseMobility::handleIfOutside(BorderPolicy policy, Coord& stepTarget, Coord& targetPos, Coord& step, double& angle) {
    // where did the host leave the playground?
    BorderHandling wo;

    // step to reach the border
    Coord borderStep(world->use2D());

    wo = checkIfOutside(stepTarget, borderStep);

    // just return if the next step is not outside the playground
    if( wo == NOWHERE )
	return false;

    coreEV << "handleIfOutside:stepTarget = " << stepTarget.info() << endl;

    // new start position after the host reaches the border
    Coord borderStart(world->use2D());
    // new direction the host has to move to
    Coord borderDirection(world->use2D());
    // time to reach the border
    simtime_t borderInterval;

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

    // calculate the time to reach the border
    //borderInterval = sqrt(borderStep.getX()*borderStep.getX() + borderStep.getY()*borderStep.getY()) / move.speed;
    borderInterval = (borderStep.length()) / move.getSpeed();

    // calculate new start position
    // NOTE: for WRAP this is done in goToBorder
    switch( policy ){
    case REFLECT:
	double d;

	borderStart = move.getStartPos() + borderStep;
	d = stepTarget.distance( borderStart );
	//borderDirection.setX((stepTarget.getX() - borderStart.getX()) / d);
	//borderDirection.setY((stepTarget.getY() - borderStart.getY()) / d);
	//borderDirection.setZ((stepTarget.getZ() - borderStart.getZ()) / d);
	borderDirection = (stepTarget - borderStart) / d;
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

