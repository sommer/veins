/* -*- mode:c++ -*- ********************************************************
 * file:        BaseMobility.h
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


#ifndef BASE_MOBILITY_H
#define BASE_MOBILITY_H

#include "BaseModule.h"
#include "BaseWorldUtility.h"
#include "Coord.h"
#include "Move.h"

/**
 * @brief Base module for all mobility modules.
 *
 * It does not provide mobility at all, so you can use
 * it if you only want to simulate static networks.
 *
 * BaseMobility provides random placement of hosts and display
 * updates.
 * Change notifications about position changes are also posted to the
 * Blackboard.
 *
 * Another service provided by BaseMobility is border handling. If a
 * host wants to move outside the playground, this situation has to be
 * handled somehow. BaseMobility provides handling for the 4 most
 * common ways for that: reflection, wrapping, random placement and
 * raising an error. The only thing you have to do is to specify your
 * desired border handling in fixIfHostGetsOutside and call it in
 * makeMove.
 *
 * For most mobility modules the only two functions you need to
 * implement to define your own mobility module are makeMove and
 * fixIfHostGetsOutside..
 *
 * @ingroup mobility
 * @ingroup basicModules
 * @author Daniel Willkomm, Andras Varga
 */
class BaseMobility : public BaseModule
{
  public:
    /**
     * @brief Selects how a node should behave if it reaches the edge
     * of the playground.  
     *
     * @sa handleIfOutside()
     **/
    enum BorderPolicy {
        REFLECT,       ///< reflect off the wall
        WRAP,          ///< reappear at the opposite edge (torus)
        PLACERANDOMLY, ///< placed at a randomly chosen position on the playground
        RAISEERROR     ///< stop the simulation with error
    };

    /** 
     * @brief The kind field of messages
     * 
     * that are used internally by this class have one of these values
     **/
    enum BaseMobilityMsgKinds {
        MOVE_HOST = 1050198237,
	MOVE_TO_BORDER,
	LAST_BASIC
    };

    /**
     * @brief Specifies which border actually has been reached
     **/
    enum BorderHandling {
	NOWHERE,   ///< not outside the playground
        X_SMALLER, ///< x smaller than 0
	X_BIGGER,  ///< x bigger than playground size
	Y_SMALLER, ///< y smaller than 0
	Y_BIGGER,  ///< y bigger than playground size
	Z_SMALLER, ///< z smaller than 0
	Z_BIGGER   ///< z bigger than playground size
    };

  protected:
	
    /** @brief Pointer to BaseWorldUtility -- these two must know each other */
    BaseWorldUtility *world;

    /** @brief Pointer to BaseUtility -- these two must know each other (for node position) */
    BaseUtility *baseUtility;

    /** @brief Pointer to host module, to speed up repeated access*/
    cModule* hostPtr;
    int hostId;
    
    /** @brief Stores the current position and move pattern of the host*/
    Move move;

    /** @brief Store the category of HostMove */
    int moveCategory;
    
    /** @brief Time interval to update the hosts position*/
    double updateInterval;

    /** @brief Self message to trigger movement */
    cMessage* moveMsg;

    /** @brief debug this core module? */
    bool coreDebug;

  public:
    Module_Class_Members( BaseMobility , BaseModule , 0 );

    /** @brief This modules should only receive self-messages*/
    void handleMessage(cMessage *msg);

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

    /** @brief Delete dynamically allocated objects*/
    virtual void finish(){};

  protected:
    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg( cMessage* );
    
    /** @brief Called upon arrival of a border messages*/
    virtual void handleBorderMsg( cMessage* );
    
    /** 
     * @brief Moves the host
     *
     * This function is called every time a MOVE_HOST self message
     * arrives. Here you can define the movement pattern for your
     * host.
     *
     * You should call fixIfHostGetsOutside here for border handling
     **/
    virtual void makeMove(){
	error("BaseMobility does not move the host");
    };

    /** @brief Update the position information for this node*/
    void updatePosition();

    /** @brief Returns the width of the playground */
    double playgroundSizeX() const  {return world->getPgs()->getX();}

    /** @brief Returns the height of the playground */
    double playgroundSizeY() const  {return world->getPgs()->getY();}

    /** @brief Returns the height of the playground */
    double playgroundSizeZ() const  {return world->getPgs()->getZ();}

	/* @brief Random position somewhere in the playground. DEPRECATED: Use BaseWorldUtility::getRandomPosition() instead */
	Coord getRandomPosition() { return world->getRandomPosition();}

    /**
     * @name Border handling 
     *
     * @brief Utility functions to handle hosts that move outside the
     * playground
     *
     **/
    /*@{*/

    /** @brief Main border handling function */
    bool handleIfOutside(BorderPolicy, Coord&, Coord&, Coord&, double&);

    /**
     * @brief Should be redefined in subclasses. 
     * 
     * Should invoke handleIfOutside() and pass the references to the
     * parameters to be modified.
     *
     * Additional action after border handling (such as choosing a new
     * target position if the BorderPolicy is PLACERANDOMLY) should be
     * implemented here.
     *
     * @sa HandleIfOutside
     */
    virtual void fixIfHostGetsOutside(){
	error("fixIfHostGetsOutside has to be redefined by the user");
    };

    /** 
     * @brief Checks whether the host is outside the playground and
     * returns where
    **/
    BorderHandling checkIfOutside( Coord, Coord& );

    /** @brief calculate the step to reach the border **/
    void goToBorder( BorderPolicy, BorderHandling, Coord&, Coord& );

    /**
     * @brief helperfunction for reflectIfOutside() to reflect
     * a Coordinate at a given border
     **/
    void reflectCoordinate(BorderHandling border, Coord& c);
    /** 
     * @brief Utility function to reflect the node if it goes outside
     * the playground.
     **/
    void reflectIfOutside(BorderHandling, Coord&, Coord&, Coord&, double&);

    /** 
     * @brief Utility function to wrap the node to the opposite edge
     * (torus) if it goes outside the playground.
     **/
    void wrapIfOutside(BorderHandling, Coord&, Coord&);

    /** 
     * @brief Utility function to place the node randomly if it goes
     * outside the playground.
     **/
    void placeRandomlyIfOutside( Coord& );

    /*@}*/

};

#endif

