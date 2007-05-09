/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelControl.cc
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


#include "ChannelControl.h"
#include "ChannelAccess.h"
#include "FWMath.h"
#include <cassert>
#include <set>

#include "NicEntryDebug.h"
#include "NicEntryDirect.h"

#ifndef ccEV
#define ccEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << "ChannelControl: "
#endif

Define_Module( ChannelControl );

/**
 * Sets up the playgroundSize and calculates the
 * maxInterferenceDistance
 *
 * @ref calcInterfDist
 **/
void ChannelControl::initialize()
{
    unsigned numX, numY;
    if(hasPar("coreDebug"))
        coreDebug = par("coreDebug").boolValue();
    else
        coreDebug = false;
    NicEntries *entries = new NicEntries();
    RowVector *row = new RowVector();
    row->push_back(*entries);
    
  ccEV <<"initializing ChannelControl\n";
  //todo
  if(hasPar("sendDirect"))
    sendDirect = par("sendDirect").boolValue();
  else
    sendDirect = false;

  if(hasPar("useTorus")) {
      useTorus = par("useTorus").boolValue();
  } else {
      useTorus = false;
  }
  if(useTorus) {
      ccEV<<"wrapping connections around playground edges"<<endl;
  }
  else {
      ccEV<<"not wrapping connections around playground edges"<<endl;
  }
  playgroundSize.x = par("playgroundSizeX");
  playgroundSize.y = par("playgroundSizeY");
  
  maxInterferenceDistance = calcInterfDist();
  maxDistSquared = maxInterferenceDistance*maxInterferenceDistance;

  findDistance = ceil(maxInterferenceDistance);
  if(ceil(maxInterferenceDistance) == maxInterferenceDistance) findDistance += EPSILON;
  numX = static_cast<unsigned>(playgroundSize.x/maxInterferenceDistance)+1;
  numY = static_cast<unsigned>(playgroundSize.y/maxInterferenceDistance)+1;

  if((numX <= 3) && (numY <= 3))
  {
      if(playgroundSize.x < playgroundSize.y) {
          findDistance = ceil(playgroundSize.y) + 1.0;
      }
      else {
          findDistance = ceil(playgroundSize.x) + 1.0;
      }
      ccEV <<" using 1x1 grid"<<endl;
      nics.push_back(*row);
  } else {
      for(unsigned i = 1; i < numY; ++i) {
          row->push_back(*entries);
      }
      for(unsigned i = 0; i < numX; ++i) {
          nics.push_back(*row);
      }
      ccEV <<" using "<<numX<<"x"<<numY<<" grid"<<endl;
  }
  maxX = nics.size()-1;
  maxY = nics[0].size()-1;
}

const double ChannelControl::speedOfLight = 300000000.0;

/**
 * Calculation of the interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power
 *
 * You may want to overwrite this function in order to do your own
 * interference calculation
 **/
double ChannelControl::calcInterfDist()
{
  double interfDistance;

  //the minimum carrier frequency for this cell
  double carrierFrequency = par("carrierFrequency");
  //maximum transmission power possible
  double pMax             = par("pMax");
  //minimum signal attenuation threshold
  double sat              = par("sat");
  //minimum path loss coefficient
  double alpha            = par("alpha");

  double waveLength     = (speedOfLight/carrierFrequency);
  //minimum power level to be able to physically receive a signal
  double minReceivePower = pow(10.0, sat/10.0);
  
  interfDistance = pow(waveLength * waveLength * pMax / 
		       (16.0*M_PI*M_PI*minReceivePower), 1.0/alpha);
  
  ccEV <<"max interference distance:"<<interfDistance<<endl;
  
  return interfDistance;
}


/**
 * Called by ChannelAccess for the nic module upon
 * initialization. The nics are written into a list.
 *
 *
 * @param ptr the module pointer of the registered nic 
 * @param pos Position of the nic
 *
 * @return Returns whether ChannelControl uses sendDirect or not
 **/
bool ChannelControl::registerNic( cModule* ptr, const Coord* pos )
{
  // register the nic
  assert(ptr != 0);

  NicEntry *nic;
  int id = ptr->id();
  ccEV <<" registering nic #"<<id<<endl;

  if(sendDirect)
      nic = new NicEntryDirect(coreDebug);
  else
      nic = new NicEntryDebug(coreDebug);
  
  nic->nicPtr = ptr;
  nic->nicId = id;
  nic->hostId = ptr->parentModule()->id();

  // copy the position
  nic->pos = *pos;

  unsigned x,y;
  x = static_cast<unsigned>(pos->x/findDistance);
  y = static_cast<unsigned>(pos->y/findDistance);

  // add to matrix
  nics[x][y][id] = nic;
  
  // update all connections for this nic
  checkGrid(x,y,x,y,id);

  return sendDirect;
}


/**
 * Called periodically by ChannelAccess to indicate that the nic
 * with "id" moved.
 *
 * Updates the nics' position and all its connections
 *
 * @param id the module id of the registered nic
 * @param pos the coordinates of the registered nic
 **/
void ChannelControl::updateNicPos(int id, const Coord* oldPos, const Coord* newPos)
{
    unsigned oldX,oldY,newX,newY;
    ccEV <<"nic #"<<id<<" moved from " << oldPos->info() << " to " << newPos->info() << " pgs: " << playgroundSize.info() << "\n";
    oldX = static_cast<unsigned>(oldPos->x/findDistance);
    oldY = static_cast<unsigned>(oldPos->y/findDistance);
    newX = static_cast<unsigned>(newPos->x/findDistance);
    newY = static_cast<unsigned>(newPos->y/findDistance);

    nics[oldX][oldY][id]->pos = *newPos;
//    ccEV <<"nic #"<<id<<" moved from " << oldPos->info() << " to " << newPos->info() << " pgs: " << playgroundSize.info() << " oldX: "<< oldX << " oldY: " << oldY << " newX: " << newX << " newY: " << newY << "\n";
    // update all connections for this nic
    checkGrid(oldX, oldY, newX, newY, id );
}

/**
 * Called by ChannelControl::updateNodePosition(...) when a nic has
 * moved. 
 **/
void ChannelControl::checkGrid(unsigned oldX, unsigned oldY,
                               unsigned newX, unsigned newY,
                               int id)
    
{
    unsigned cX, cY;
    
    // structure to find union of grid squares
    std::map<unsigned, std::set<unsigned> > gridUnion;
    std::map<unsigned, std::set<unsigned> >::const_iterator gUmIt;
    std::set<unsigned>::const_iterator gUsIt;
    
    // find nic at old position
    NicEntries::iterator it = nics[oldX][oldY].find(id);
    NicEntry *nic = it->second;
    
    // move nic to a new position in matrix
    if((newX != oldX) || (newY != oldY)) {
        nics[oldX][oldY].erase(it);
        nics[newX][newY][id] = nic;
    }

    if((maxX == 0) && (maxY == 0)) {
        gridUnion[oldX].insert(oldY);
    } else {
        if(decrement(maxX, oldX, &cX)) {
            // square top left of old square
            if(decrement(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
            // square top of old square
            gridUnion[cX].insert(oldY);
            // square top right of old square
            if(increment(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
        }
        
        // square left of old square
        if(decrement(maxY, oldY, &cY)) gridUnion[oldX].insert(cY);
        // old square
        gridUnion[oldX].insert(oldY);
        // square right of old square
        if(increment(maxY, oldY, &cY)) gridUnion[oldX].insert(cY);

        if(increment(maxX, oldX, &cX)) {
            // square bottom left of old square
            if(decrement(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
            // square bottom of old square
            gridUnion[cX].insert(oldY);
            // square bottom right of old square
            if(increment(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
        }

        if((newX != oldX) || (newY != oldY)) {
            if(decrement(maxX, newX, &cX)) {
                // square top left of new square
                if(decrement(maxY, newY, &cY)) gridUnion[cX].insert(cY);
                // square top of new square
                gridUnion[cX].insert(newY);
                // square top right of new square
                if(increment(maxY, newY, &cY)) gridUnion[cX].insert(cY);
            }
            
            // square left of new square
            if(decrement(maxY, newY, &cY)) gridUnion[newX].insert(cY);
            // new square
            gridUnion[newX].insert(newY);
            // square right of new square
            if(increment(maxY, newY, &cY)) gridUnion[newX].insert(cY);

            if(increment(maxX, newX, &cX)) {
                // square bottom left of new square
                if(decrement(maxY, newY, &cY)) gridUnion[cX].insert(cY);
                // square bottom of new square
                gridUnion[cX].insert(newY);
                // square bottom right of new square
                if(increment(maxY, newY, &cY)) gridUnion[cX].insert(cY);
            }
        }
    }
    for(gUmIt = gridUnion.begin(); gUmIt != gridUnion.end(); ++gUmIt)
    {
        cX = gUmIt->first;
        for(gUsIt = gUmIt->second.begin(); gUsIt != gUmIt->second.end(); ++gUsIt)
        {
            cY = *gUsIt;
            ccEV << "Update cons in ["<<cX<<", "<<cY<<"]"<<endl;
            updateConnections(nics[cX][cY], nic);
        }
    }
}

/**
 * Called by ChannelControl::updateNodePosition(...) when a nic has
 * moved. Sets up a new connection between two nics, if they are
 * within interference range. Accordingly tears down a connection,
 * if the nics move out of range.
 *
 * @param id Id of the nic that will have its' connections
 * updated
 **/
void ChannelControl::updateConnections(NicEntries& nmap, NicEntry* nic)
{
    int id = nic->nicId;
    bool inRange;

    for(NicEntries::iterator i=nmap.begin(); i!=nmap.end(); ++i)
    {
        // no recursive connections
        if ( i->second->nicId == id ) continue;

        NicEntry* nic_i = i->second;

        inRange = (useTorus) ? inRangeTorus(nic->pos,nic_i->pos) : inRangeEuclid(nic->pos,nic_i->pos);
        
        if ( inRange && !nic->isConnected(nic_i) ){
            // nodes within communication range: connect
            // nodes within communication range && not yet connected
            ccEV <<"nic #"<<nic->nicId<<" and #"<<nic_i->nicId << " are in range"<<endl;
            nic->connectTo( nic_i );
            nic_i->connectTo( nic );
        }
        else if ( !inRange && nic->isConnected(nic_i) ){
            // out of range: disconnect
            // out of range, and still connected
            ccEV <<"nic #"<<nic->nicId<<" and #"<<nic_i->nicId << " are NOT in range"<<endl;
            nic->disconnectFrom( nic_i );
            nic_i->disconnectFrom( nic );
        }
    }
}

bool ChannelControl::inRangeTorus(const Coord& a, const Coord& b) 
{
    if(FWMath::torDist(a.x,                  b.x, a.y,                  b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x+playgroundSize.x, b.x, a.y,                  b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x-playgroundSize.x, b.x, a.y,                  b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x,                  b.x, a.y+playgroundSize.y, b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x,                  b.x, a.y-playgroundSize.y, b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x+playgroundSize.x, b.x, a.y+playgroundSize.y, b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x+playgroundSize.x, b.x, a.y-playgroundSize.y, b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x-playgroundSize.x, b.x, a.y+playgroundSize.y, b.y) < maxDistSquared) return true;
    if(FWMath::torDist(a.x-playgroundSize.x, b.x, a.y-playgroundSize.y, b.y) < maxDistSquared) return true;
    return false;
}


const NicEntry::GateList& ChannelControl::getGateList( int id, const Coord* pos )
{
    unsigned x,y;

    x = static_cast<unsigned>(pos->x/findDistance);
    y = static_cast<unsigned>(pos->y/findDistance);
	
	if (nics[x][y].find(id)==nics[x][y].end())
	{
		opp_error("id not found in nics (id=%d)\n",id);
	}
	return nics[x][y][id]->getGateList();
}

/**
 * Called by P2PPhyLayer. Needed to send a packet directly to a
 * certain nic without other nodes 'hearing' it. This is only useful
 * for physical layers that work with bit error probability like
 * P2PPhyLayer.
 *
 * @param from id of the nic from which the a packet is about to be sent
 * @param to id of the nic to which the packet is about to be sent
 */
const cGate* ChannelControl::getOutGateTo(int from, int to, const Coord* pos)
{
    unsigned x,y;

    x = static_cast<unsigned>(pos->x/findDistance);
    y = static_cast<unsigned>(pos->y/findDistance);

    return nics[x][y][from]->getOutGateTo(to);
}

