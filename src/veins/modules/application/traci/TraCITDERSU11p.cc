//
// Copyright (C) 2015 Daniel Febrian Sengkey <danielsengkey.cio13@mail.ugm.ac.id>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Part of my Master of Engineering thesis project.
//
// Department of Electrical Engineering and Information Technology, Gadjah Mada University
// Yogyakarta, INDONESIA
//
// Most parts of this file was derived from TraCIDemo11p.{cc,h} of Veins by Dr.-Ing. Christoph Sommer
#include "veins/modules/application/traci/TraCITDERSU11p.h"

using Veins::AnnotationManagerAccess;

Define_Module(TraCITDERSU11p);

void TraCITDERSU11p::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
        mobi = dynamic_cast<BaseMobility*> (getParentModule()->getSubmodule("mobility"));
        ASSERT(mobi);
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);
        sentMessage = false;

        debugAppTDE = par("debugAppTDE").boolValue();
        enableTDE = par("enableTDE").boolValue();
        statInterval = par("statInterval").doubleValue();
        timeoutInterval = (simtime_t) par("timeoutInterval").doubleValue();
        timeoutMsgInterval = par("timeoutMsgInterval").doubleValue();

        /** @brief registering all signals for the statistics output */
        vehNumberLV = registerSignal("detectedVehiclesLV");
        vehNumberHV = registerSignal("detectedVehiclesHV");
        vehNumberMC = registerSignal("detectedVehiclesMC");

        /** @brief Total detected vehicle set to 0 */
        currentNumberofTotalDetectedVehicles = 0;

        /** Scheduling the first statistics signals update */
        if(enableTDE) {
            updateStatMsg = new cMessage("statistics update", UPDATE_STATS);
            scheduleAt(simTime() + statInterval, updateStatMsg);
            if(timeoutInterval>0){
                timeoutMsg = new cMessage("check for timed out node", CHECK_TIMEOUT);
                scheduleAt(simTime() + timeoutMsgInterval, timeoutMsg);
            }
        }

        /** @brief initializing vehicles array with -1 to prevent conflict with node id within simulation.
         * Store self id in the first index.
         */
        for (int i=0; i < maxVehicles; i++)
        {
            listedVehicles[i].id = -1;
            listedVehicles[i].vType = "undefined";
        }
        if (debugAppTDE==true) showInfo(currentNumberofTotalDetectedVehicles);
	}
}

void TraCITDERSU11p::onBeacon(WaveShortMessage* wsm) {
    EV << "### REPORT OF RSU WITH ID " << myId << ", type " << myType <<" ###" << endl;
    EV << "Received BEACON from " << wsm->getSenderAddress() << " on " << wsm->getArrivalTime() << " type " << wsm->getWsmData() << "."<< endl;
    if((enableTDE)&&((wsm->getWsmData()=="MC")||(wsm->getWsmData()=="LV")||(wsm->getWsmData()=="HV"))) {
        if (indexedCar(wsm->getSenderAddress(), currentNumberofTotalDetectedVehicles)==false) {
            append2List(wsm->getSenderAddress(), currentNumberofTotalDetectedVehicles,  wsm->getArrivalTime(), wsm->getWsmData());
        }
        else updateLastSeenTime(wsm->getSenderAddress(), currentNumberofTotalDetectedVehicles, wsm->getArrivalTime());
        if (debugAppTDE==true) showInfo(currentNumberofTotalDetectedVehicles);
    }
    EV << "########## END OF REPORT ##########" << endl;
}

void TraCITDERSU11p::onData(WaveShortMessage* wsm) {
	findHost()->getDisplayString().updateWith("r=16,green");

	annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobi->getCurrentPosition(), "blue"));

	EV << "Received DATA from " << wsm->getSenderAddress() << " on " << wsm->getArrivalTime() << endl;

	if (!sentMessage) sendMessage(wsm->getWsmData());
}

void TraCITDERSU11p::sendMessage(std::string blockedRoadId) {
	sentMessage = true;
	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
	wsm->setWsmData(blockedRoadId.c_str());
	sendWSM(wsm);
}
void TraCITDERSU11p::sendWSM(WaveShortMessage* wsm) {
	sendDelayedDown(wsm,individualOffset);
}

/* Following lines of methods are those I built, (some are overloaded) in regards of my thesis */
bool TraCITDERSU11p::indexedCar(short carId, short counter) {
    int i;
    bool indexedStatus = false;
    for(i=0; i < counter; i++) {
        if(listedVehicles[i].id == carId) {
            indexedStatus = true;

            break;
        } //endif
    } //endfor
    if (indexedStatus) EV << "Vehicle with ID = " << carId << " is already indexed." << endl;
    else EV << "Vehicle with ID = " << carId << " is still unindexed." << endl;
    return indexedStatus;
} //end method

void TraCITDERSU11p::append2List(short carId, short firstEmptyArrayIndex, simtime_t messageTime, std::string vType) {
    listedVehicles[firstEmptyArrayIndex].id = carId;
    listedVehicles[firstEmptyArrayIndex].lastSeenAt = messageTime;
    listedVehicles[firstEmptyArrayIndex].vType = vType;
    EV << "Appending car with id " << carId <<" type "<< vType << " to the list of known vehicle." <<endl;

    /* @brief Increase related counting variable
     * The total number always increased for each vehicle
     */
    if (vType == "MC") { currentNumberofDetectedMC++;
    } else if (vType == "LV") { currentNumberofDetectedLV++;
    } else if (vType == "HV") { currentNumberofDetectedHV++;
    } else DBG << "Unknown vehicle type" << vType << endl;
    currentNumberofTotalDetectedVehicles++;
}

void TraCITDERSU11p::showInfo(short counter){
    EV << "Listed vehicles are:"<<endl;
    for(int i=0; i < counter; i++) EV << "[" << i << "] " << listedVehicles[i].id << "\ttype\t" << listedVehicles[i].vType << "\tat\t" << listedVehicles[i].lastSeenAt << endl;
    EV << endl;

    EV << "Number of detected light vehicles\t: " << currentNumberofDetectedLV << endl;
    EV << "Number of detected heavy vehicles\t: " << currentNumberofDetectedHV << endl;
    EV << "Number of detected motorcycles\t: " << currentNumberofDetectedMC << endl;
    EV << "Total number of detected vehicle\t: " << currentNumberofTotalDetectedVehicles << endl;
}

void TraCITDERSU11p::updateLastSeenTime(short carId, short counter, simtime_t messageTime){
    int i;
    for (i=1; i< counter; i++) {
        if (listedVehicles[i].id == carId) {
            EV << "Updating record for vehicle with ID " << carId << " with previously last seen at " << listedVehicles[i].lastSeenAt << ", now lastSeenAt = " << messageTime <<"."<<endl;
            listedVehicles[i].lastSeenAt = messageTime;
            break;
        }
    }
}

void TraCITDERSU11p::updateStats() {
    if(debugAppTDE==true) EV << "Updating statistics record" << endl;
    emit(vehNumberMC, currentNumberofDetectedMC);
    emit(vehNumberLV, currentNumberofDetectedLV);
    emit(vehNumberHV, currentNumberofDetectedHV);
}

/** delete timed out vehicle from the list and restructure the array */
void TraCITDERSU11p::deleteAndRestructureArray() {
    int i,j;

    for (i=1; i<currentNumberofTotalDetectedVehicles; i++) {
        if(debugAppTDE) EV << "Checking last seen time for list index [" << i << "], ID: " << listedVehicles[i].id << endl;

        if (listedVehicles[i].lastSeenAt < (simTime()-timeoutInterval)) {
            EV << "Found timed out node in index. ID: " << listedVehicles[i].id
                    << ", type " << listedVehicles[i].vType
                    << " was last seen at " << listedVehicles[i].lastSeenAt
                    << " while current time is " << simTime() << "." << endl;

            if (listedVehicles[i].vType == "MC") {
                currentNumberofDetectedMC--;
            } else if (listedVehicles[i].vType == "LV") {
                currentNumberofDetectedLV--;
            } else if (listedVehicles[i].vType == "HV") {
                currentNumberofDetectedHV--;
            } else EV << "Node ID: " << myId << "Unknown vehicle type from node: " << listedVehicles[i].id << endl;

            for (j=i; j<currentNumberofTotalDetectedVehicles; j++) {
                listedVehicles[j] = listedVehicles[j+1];
            }
            currentNumberofTotalDetectedVehicles--;
        }
    }

    if(debugAppTDE) showInfo(currentNumberofTotalDetectedVehicles);
}

void TraCITDERSU11p::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
    case UPDATE_STATS: {
        updateStats();
        scheduleAt(simTime() + statInterval, updateStatMsg);
        break;
    }
    case CHECK_TIMEOUT: {
        if((currentNumberofTotalDetectedVehicles>1)&&enableTDE) deleteAndRestructureArray();
        scheduleAt(simTime() + timeoutMsgInterval, timeoutMsg);
        break;
    }
    default: {
        if (msg)
            DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
        break;
    }
    }
}

void TraCITDERSU11p::finish() {
    if (sendBeaconEvt->isScheduled()) {
        cancelAndDelete(sendBeaconEvt);
    } else {
        delete sendBeaconEvt;
    }

    if (timeoutMsg->isScheduled()) {
        cancelAndDelete(timeoutMsg);
    } else {
        delete timeoutMsg;
    }

    if (updateStatMsg->isScheduled()) {
        cancelAndDelete(updateStatMsg);
    } else {
        delete updateStatMsg;
    }
    findHost()->unsubscribe(mobilityStateChangedSignal, this);

}
