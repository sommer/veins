/* -*- mode:c++ -*- ********************************************************
 * file:        SimTracer.h
 *
 * author:      Jérôme Rousselot
 *
 * copyright:   (C) 2007-2008 CSEM SA, Neuchatel, Switzerland
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     Modifications to the MF Framework by CSEM
 ***************************************************************************/

#ifndef SIMTRACER_H
#define SIMTRACER_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <BaseModule.h>
#include <BaseLayer.h>
//#include <ChannelControl.h>
#include "Packet.h"

using namespace std;

/**
 * @class SimTracer
 * @ingroup utils
 * @author Jérôme Rousselot
 */

class SimTracer:public BaseModule
{

public:


	/** @brief Initialization of the module and some variables*/
  virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
  virtual void finish();

    /** @brief Called by any module wanting to log a nam event. */
  void namLog(string namString);

  void radioEnergyLog(unsigned long mac, int state, simtime_t duration,
		      double power, double newPower);

  /** @brief Called by a routing protocol to log a link in a tree topology. */
  void logLink(int parent, int child);
  /** @brief Called by the MAC or NET layer to log the node position. */
  void logPosition(int node, double x, double y);

  /** @brief Called by the Blackboard whenever a change occurs we're interested in */
  virtual void receiveBBItem(int category, const BBItem * details, int scopeModuleId);

protected:
   ofstream namFile, radioEnergyFile, treeFile;;
   vector < string > packetsColors;
   map < unsigned long, double >powerConsumptions;
   int catPacket;
   Packet packet;
   long nbApplPacketsSent;
   long nbApplPacketsReceived;
   int catEnergy;
   map < unsigned long, double >powerConsumptions2;
   map < unsigned long, double >currPower;
   map < unsigned long, simtime_t> lastUpdates;
};


#endif

