/* -*- mode:c++ -*- ********************************************************
 * file:        SimTracer.cc
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/
#include "SimTracer.h"

#include "FindModule.h"
#include "BaseLayer.h"

using std::map;
using std::string;

Define_Module(SimTracer);

/*
 * Open some log files and write some static initializations stuff.
 */
void SimTracer::initialize(int stage)
{
  cSimpleModule::initialize(stage);
  if (stage == 0) {
	char treeName[250];
	/*int n;
	n =*/ sprintf(treeName, "results/tree-%d.txt",
	cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getActiveRunNumber());
    treeFile.open(treeName);
    if (!treeFile) {
      EV << "Error opening output stream for routing tree statistics."
          << endl;
    } else {
      treeFile << "graph aRoutingTree " << endl << "{" << endl;
    }
    goodputVec.setName("goodput");
    pSinkVec.setName("sinkPowerConsumption");
    pSensorVec.setName("sensorPowerConsumption");
    nbApplPacketsSent = 0;
    nbApplPacketsReceived = 0;

    // retrieve pointer to BaseWorldUtility module
    world = FindModule<BaseWorldUtility*>::findGlobalModule();
    //world = check_and_cast<BaseWorldUtility*>(cSimulation::getActiveSimulation()->getModuleByPath("sim.world"));
    if (world) {
        world->subscribe(BaseLayer::catPacketSignal, this);
    }
    else {
      error("No BaseWorldUtility module found, please check your ned configuration.");
    }

//  } else if(stage == 1) {  // it seems that we are initialized only once. Why ?
  }
}

// compute current average sensor power consumption
double SimTracer::getAvgSensorPowerConsumption() {
	double sensorAvgP = 0;
	int nbSensors = 0;
	map < unsigned long, double >::iterator iter = powerConsumptions.begin(); // address, powerConsumption
	for (; iter != powerConsumptions.end(); iter++) {	// iterate over all nodes power consumptions
		  double eval = iter->second;
		  eval = eval +  SIMTIME_DBL(simTime()-lastUpdates[iter->first])*currPower[iter->first];
		  eval = eval * 1000 / simTime();
		  if(iter->first != 0) {
			  nbSensors++;
			  sensorAvgP += eval;
		  }
	  }
	  sensorAvgP = sensorAvgP / nbSensors;
	return sensorAvgP;
}

double SimTracer::getSinkPowerConsumption() {
  double sinkP = powerConsumptions[0];
  sinkP = sinkP + SIMTIME_DBL(simTime() - lastUpdates[0]) * currPower[0];
  sinkP = sinkP * 1000 / simTime();
  return sinkP;
}

/*
 * Close the nam log file.
 */
void SimTracer::finish()
{
  double goodput = 0;
  if(nbApplPacketsSent > 0) {
	  goodput = ((double) nbApplPacketsReceived)/nbApplPacketsSent;
  } else {
	  goodput = 0;
  }
  recordScalar("Application Packet Success Rate", goodput);
  recordScalar("Application packets received", nbApplPacketsReceived);
  recordScalar("Application packets sent", nbApplPacketsSent);
  recordScalar("Sink power consumption", getSinkPowerConsumption());
  recordScalar("Sensor average power consumption", getAvgSensorPowerConsumption());
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
			       simtime_t_cref duration, double power, double newPower)
{
  Enter_Method_Silent();
  /*
  radioEnergyFile << mac << "\t" << state << "\t" << duration << "\t" << power
    << endl;
    */
  if (powerConsumptions.count(mac) == 0) {
    powerConsumptions[mac] = 0;
  }
  powerConsumptions[mac] = powerConsumptions[mac] + power * SIMTIME_DBL(duration);
  currPower[mac] = newPower;
  lastUpdates[mac] = simTime();
  if(mac != 0) {
	  pSensorVec.record(getAvgSensorPowerConsumption());
  } else {
	  pSinkVec.record(getSinkPowerConsumption());
  }
}

void SimTracer::logPosition(int node, double x, double y, double z)
{
	treeFile << node << "[pos=\""<< x << ", " << y << "!\"];" << endl;
}

void SimTracer::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
	if (signalID == BaseLayer::catPacketSignal) {
		packet = *(static_cast<const Packet*>(obj));
	//	nbApplPacketsSent = nbApplPacketsSent + packet.getNbPacketsSent();
	//	nbApplPacketsReceived = nbApplPacketsReceived + packet.getNbPacketsReceived();
		if(packet.isSent()) {
			nbApplPacketsSent = nbApplPacketsSent + 1;
		} else {
			nbApplPacketsReceived = nbApplPacketsReceived + 1;
		}
		goodputVec.record(((double) nbApplPacketsReceived)/nbApplPacketsSent);
	}
}

