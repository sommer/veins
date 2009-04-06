/* -*- mode:c++ -*- ********************************************************
 * file:        SimTracer.cc
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


#include "SimTracer.h"


Define_Module(SimTracer);

/*
 * Open some log files and write some static initializations stuff.
 */
void SimTracer::initialize(int stage)
{
  BaseModule::initialize(stage);
  if (stage == 0) {
	char treeName[250];
	int n;
	n = sprintf(treeName, "results/tree-%d.txt", cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getActiveRunNumber());
	treeFile.open(treeName);
    if (!treeFile) {
      EV << "Error opening output stream for routing tree statistics."
          << endl;
    } else {
      treeFile << "graph aRoutingTree " << endl << "{" << endl;
    }

    // world utility
	// get pointer to the simulation-wide blackboard module
	utility2 = FindModule<BaseUtility*>::findGlobalModule();

    // goodput code
    Packet apacket;
    catPacket = utility2->subscribe(this, &apacket, -1);
    nbApplPacketsSent = 0;
    nbApplPacketsReceived = 0;
  }
}

/*
 * Close the nam log file.
 */
void SimTracer::finish()
{
  ofstream summaryPowerFile;

  map < unsigned long, double >::iterator iter =
    powerConsumptions.begin();

  double sensorAvgP = 0;
  int nbSensors = 0;
  for (; iter != powerConsumptions.end(); iter++) {
	  iter->second = iter->second +  (simTime()-lastUpdates[iter->first]).dbl()*currPower[iter->first];
	  iter->second = iter->second * 1000 / simTime();
	  if(iter->first != 0) {
		  nbSensors++;
		  sensorAvgP += iter->second;
	  }
  }
  sensorAvgP = sensorAvgP / nbSensors;
  summaryPowerFile.close();

  double goodput = 0;
  for(map<int, pair<long,long> >::iterator iter = goodputStats.begin();
	  iter != goodputStats.end(); iter++) {
	  if(nbApplPacketsSent  > 0) {
		  goodput = ((double) iter->second.second)/(nbApplPacketsSent - iter->second.first); // goodput in broadcast
	  } else {
		  goodput = 0;
	  }
	  char description[250];
	  int n;
	  n = sprintf(description, "goodput-node_%d", iter->first);
	  recordScalar(description, goodput);
  }
  recordScalar("Application packets received", nbApplPacketsReceived);
  recordScalar("Application packets sent", nbApplPacketsSent);
  recordScalar("Sink power consumption", powerConsumptions[0]);
  recordScalar("Sensor average power consumption", sensorAvgP);
  if (treeFile) {
    treeFile << "}" << endl;
    treeFile.close();
  }

}

/*
 * Record a line into the nam log file.
 */
void SimTracer::namLog(string namString)
{
  //Enter_Method_Silent();
  //namFile << namString << endl;
}

void SimTracer::radioEnergyLog(unsigned long mac, int state,
			       simtime_t duration, double power, double newPower)
{
  Enter_Method_Silent();
  /*
  radioEnergyFile << mac << "\t" << state << "\t" << duration << "\t" << power
    << endl;
    */
  if (powerConsumptions.count(mac) == 0) {
    powerConsumptions[mac] = 0;
  }
  powerConsumptions[mac] = powerConsumptions[mac] + power * duration.dbl();
  currPower[mac] = newPower;
  lastUpdates[mac] = simTime();
}



void SimTracer::logLink(int parent, int child)
{
  treeFile << "   " << parent << " -- " << child << " ;" << endl;
}

void SimTracer::logPosition(int node, double x, double y)
{
	treeFile << node << "[pos=\""<< x << ", " << y << "!\"];" << endl;
}

void SimTracer::receiveBBItem(int category, const BBItem * details,
	       int scope) {
	if (category == catPacket) {
		packet = *(static_cast<const Packet*>(details));
	//	nbApplPacketsSent = nbApplPacketsSent + packet.getNbPacketsSent();
	//	nbApplPacketsReceived = nbApplPacketsReceived + packet.getNbPacketsReceived();

		if(packet.isSent()) {
			goodputStats[packet.getHost()].first++;
			nbApplPacketsSent = nbApplPacketsSent + 1;
		} else {
			goodputStats[packet.getHost()].second++;
			nbApplPacketsReceived = nbApplPacketsReceived + 1;
		}
	}
}

