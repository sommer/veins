/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelControl.h
 *
 * author:      Steffen Sroka, Daniel Willkomm
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
 * description: Module to control the channel and handle all connection
 *              related stuff
 **************************************************************************/

#ifndef CHANNELCONTROL_H
#define CHANNELCONTROL_H

#include <omnetpp.h>
#include <list>
#include <vector>

#include "BaseModule.h"

#include "Coord.h"
#include "NicEntry.h"

/**
 * @brief Module to control the channel and handle all connection
 * related stuff
 *
 * The central module that coordinates the connections between all
 * nodes, and handles dynamic gate creation. ChannelControl therefore
 * periodically communicates with the ChannelAccess modules
 *       
 * @ingroup channelControl
 * @author Steffen Sroka, Daniel Willkomm
 * @sa ChannelAccess
 */
class ChannelControl : public BaseModule
{
private:
	const Coord* playgroundSize;
protected:
    /** @brief Set debugging for the basic module*/
    bool coreDebug;

    typedef std::map<int, NicEntry*> NicEntries;
    typedef std::vector<NicEntries> RowVector;
    typedef std::vector<RowVector> NicMatrix;

   /** @brief Registry of all Nics
    *
    * This matrix keeps all Nics according to their position.  It
    * allows to restrict the position update to a subset of all nics.
    */
    NicMatrix nics;

    /** @brief Does the ChannelControl use sendDirect or not?*/
    bool sendDirect;

   
    /** @brief the biggest interference distance in the network.*/
    double maxInterferenceDistance;

    /** @brief Square of maxInterferenceDistance
     * cache a value that is often used
     */
    double maxDistSquared;

    /**
     * Distance that helps to find a node under a certain
     * position. Can be larger then @see maxInterferenceDistance to
     * allow nodes to be placed into the same square if the playground
     * is too small for the grid speedup to work. */
    double findDistance;

    /**
     * further cached values
     */
    unsigned maxX, maxY;

    /**
     * If set, node connections wrap arounnd the borders of the
     * playground. This eliminiates certain border effects. 
     */
    bool useTorus;
    
protected:
    /**
     * Helper functions dealing with the connection handling in the grid
     *@{
     */
    /**
     * find the next larger coordinate in grid, return true if the
     * connections in this position should be updated.
     */
    bool increment(unsigned max, unsigned src, unsigned* target) {
        bool res = true;
        unsigned v = src + 1;
        if(src == max) {
            v = 0;
            if(!useTorus) res = false;
        }
        *target = v;
        return res;
    }
    
    /**
     * find the next smaller coordinate in grid, return true if the
     * connections in this position should be updated.
     */
    bool decrement(unsigned max, unsigned src, unsigned* target) {
        bool res = true;
        unsigned v = src - 1;
        if(src == 0) {
            v = max;
            if(!useTorus) res = false;
        }
        *target = v;
        return res;
    }

    /**
     * check whether nodes are connected in euclidean space
     */
    bool inRangeEuclid(const Coord& a, const Coord& b) {
        return a.sqrdist(b) <= maxDistSquared;
    }

    /**
     * check whether nodes are connected on torus (connections wrap around corners)
     * @{ 
     */
    bool inRangeTorus(const Coord& a, const Coord& b);
    /**
     *@}
     */

public:
    /**
     * @brief Constructor
     **/
    Module_Class_Members(ChannelControl, BaseModule, 0);
    
    /**
     * @brief Reads init parameters and calculates a maximal interfence
     * distance
     **/
    virtual void initialize(int stage);
	virtual ~ChannelControl();
    
    /** @brief Returns the x and y coordinates of the given nic. */
    // const Coord* getNicPos( int );
    
    /** @brief Registers a nic to have its connections managed by ChannelControl.*/
    bool registerNic( BaseModule*);
    
    /** @brief Updates the position information of a registered nic.*/
    void updateNicPos(int, const Coord* oldPos, const Coord* newPos);
    
    
    const NicEntry::GateList& getGateList( int, const Coord* );

    const cGate* getOutGateTo(int, int, const Coord*);

protected:
    /** @brief Manages the connections of a registered nic. */ 
    void updateConnections(NicEntries& nmap, NicEntry* nic);
    
    /** @brief Calculate interference distance*/
    virtual double calcInterfDist();

    /**
     * check connections of a nic in the grid
     */
    void checkGrid(unsigned oldX, unsigned oldY,
                   unsigned newX, unsigned newY,
                   int id);
};

#endif
