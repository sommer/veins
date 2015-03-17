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
// Copyright (C) 2014 Daniel Febrian Sengkey <danielsengkey.cio13@mail.ugm.ac.id>
// Department of Electrical Engineering and Information Technology, Gadjah Mada University
// Yogyakarta, INDONESIA
//
// Most parts of this file was derived from TraCIDemo11p.{cc,h} of Veins 3.0 by Dr.-Ing. Christoph Sommer

#include "veins/modules/application/traci/TraCITDE.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t TraCITDE::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(TraCITDE);

void TraCITDE::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        sentMessage = false;
        lastDroveAt = simTime();
        findHost()->subscribe(parkingStateChangedSignal, this);
        isParking = false;
        sendWhileParking = par("sendWhileParking").boolValue();

        debugAppTDE = par("debugAppTDE").boolValue();
        statInterval = par("statInterval").doubleValue();
        timeoutInterval = (simtime_t) par("timeoutInterval").doubleValue();
        timeoutMsgInterval = par("timeoutMsgInterval").doubleValue();

        myType = traciVehicle->getTypeId();

        /** @brief registering all signals for the statistics output */
        vehNumberLV = registerSignal("detectedVehiclesLV");
        vehNumberHV = registerSignal("detectedVehiclesHV");
        vehNumberMC = registerSignal("detectedVehiclesMC");
        myTypeInt = registerSignal("vTypeCode");

        /** @brief initialized the number of detected vehicles with 0 in the beginning.
         * Please note that every single type of detected vehicle number is stored within single variable.
         * @TODO Create an array of this when there is array support in .ned files.
         */
        currentNumberofDetectedLV = 0;
        currentNumberofDetectedHV = 0;
        currentNumberofDetectedMC = 0;

        /** @brief Total detected vehicle set to 0 */
        currentNumberofTotalDetectedVehicles = 0;

        /** Scheduling the first statistics signals update */
        updateStatMsg = new cMessage("statistics update", UPDATE_STATS);
        scheduleAt(simTime() + statInterval, updateStatMsg);

        if(timeoutInterval>0){
            timeoutMsg = new cMessage("check for timed out node", CHECK_TIMEOUT);
            scheduleAt(simTime() + timeoutMsgInterval, timeoutMsg);
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

        /** @brief storing vTypeInt in scalar */
        if(myType.compare("MC")==0) {
            vTypeInt = MC;
            //                    currentNumberofDetectedMC++;
            //                    emit(vehNumberMC, currentNumberofDetectedMC);
        }
        else if(myType.compare("LV")==0) {
            vTypeInt = LV;
            //                    currentNumberofDetectedLV++;
            //                    emit(vehNumberLV, currentNumberofDetectedLV);
        }
        else if(myType.compare("HV")==0) {
            vTypeInt = HV;
            //                    currentNumberofDetectedHV++;
            //                    emit(vehNumberHV, currentNumberofDetectedHV);
        }

        if(debugAppTDE==true) EV << "[" << myId << "] My type is " << myType << ", storing scalar type with " << vTypeInt << endl;
        emit(myTypeInt, vTypeInt);

        // Set the first vehicle type as itself
        if (listedVehicles[0].vType=="undefined") {
            if(!myType.empty()) append2List(myId, 0, simTime(), myType);
            else DBG << "App Error: myType is UNSET.";
        }

        else DBG << "APP: Error: Unknown type: " << myType << endl;

        if (debugAppTDE==true) showInfo(currentNumberofTotalDetectedVehicles);
    }
}

void TraCITDE::onBeacon(WaveShortMessage* wsm) {
    EV << "### REPORT OF VEHICLE WITH ID " << myId << ", type " << myType <<" ###" << endl;
    EV << "Received BEACON from " << wsm->getSenderAddress() << " on " << wsm->getArrivalTime() << " type " << wsm->getWsmData() << "."<< endl;

    if (indexedCar(wsm->getSenderAddress(), currentNumberofTotalDetectedVehicles)==false) {
        append2List(wsm->getSenderAddress(), currentNumberofTotalDetectedVehicles,  wsm->getArrivalTime(), wsm->getWsmData());
    }
    else updateLastSeenTime(wsm->getSenderAddress(), currentNumberofTotalDetectedVehicles, wsm->getArrivalTime());
    if (debugAppTDE==true) showInfo(currentNumberofTotalDetectedVehicles);
    EV << "########## END OF REPORT ##########" << endl;
}

void TraCITDE::onData(WaveShortMessage* wsm) {
    findHost()->getDisplayString().updateWith("r=16,green");
    annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobility->getPositionAt(simTime()), "blue"));

    EV << "Received DATA from " << wsm->getSenderAddress() << " on " << wsm->getArrivalTime() << endl;

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getWsmData(), 9999);
    if (!sentMessage) sendMessage(wsm->getWsmData());
}

void TraCITDE::sendMessage(std::string blockedRoadId) {
    sentMessage = true;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}
void TraCITDE::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}
void TraCITDE::handleParkingUpdate(cObject* obj) {
    isParking = mobility->getParkingState();
    if (sendWhileParking == false) {
        if (isParking == true) {
            (FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(this->getParentModule()->getSubmodule("nic"));
        }
        else {
            Coord pos = mobility->getCurrentPosition();
            (FindModule<BaseConnectionManager*>::findGlobalModule())->registerNic(this->getParentModule()->getSubmodule("nic"), (ChannelAccess*) this->getParentModule()->getSubmodule("nic")->getSubmodule("phy80211p"), &pos);
        }
    }
}
void TraCITDE::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10) {
            findHost()->getDisplayString().updateWith("r=16,red");
            if (!sentMessage) sendMessage(mobility->getRoadId());
        }
    }
    else {
        lastDroveAt = simTime();
    }
}
void TraCITDE::sendWSM(WaveShortMessage* wsm) {
    if (isParking && !sendWhileParking) return;
    sendDelayedDown(wsm,individualOffset);
}

/* Following lines of methods are those I built, (some are overloaded) in regards of my thesis */
bool TraCITDE::indexedCar(short carId, short counter) {
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

void TraCITDE::append2List(short carId, short firstEmptyArrayIndex, simtime_t messageTime, std::string vType) {
    listedVehicles[firstEmptyArrayIndex].id = carId;
    listedVehicles[firstEmptyArrayIndex].lastSeenAt = messageTime;
    listedVehicles[firstEmptyArrayIndex].vType = vType;
    EV << "Appending car with id " << carId <<" type "<< vType << " to the list of known vehicle." <<endl;

    /* @brief Increase related counting variable
     * The total number always increased for each vehicle
     */
    if (vType == "MC") { currentNumberofDetectedMC++; /* emit(vehNumberMC, currentNumberofDetectedMC); */ }
    if (vType == "LV") { currentNumberofDetectedLV++; /* emit(vehNumberLV, currentNumberofDetectedLV); */ }
    if (vType == "HV") { currentNumberofDetectedHV++; /* emit(vehNumberHV, currentNumberofDetectedHV); */ }
    /*
     * @TODO Find out why this line failed to act as expected.
     *else DBG << "Unknown vehicle type" << vType << endl;
     */
    currentNumberofTotalDetectedVehicles++;
}

void TraCITDE::showInfo(short counter){
    EV << "Listed vehicles are:"<<endl;
    for(int i=0; i < counter; i++) EV << "[" << i << "] " << listedVehicles[i].id << "\ttype\t" << listedVehicles[i].vType << "\tat\t" << listedVehicles[i].lastSeenAt << endl;
    EV << endl;

    EV << "Number of detected light vehicles\t: " << currentNumberofDetectedLV << endl;
    EV << "Number of detected heavy vehicles\t: " << currentNumberofDetectedHV << endl;
    EV << "Number of detected motorcycles\t: " << currentNumberofDetectedMC << endl;
    EV << "Total number of detected vehicle\t: " << currentNumberofTotalDetectedVehicles << endl;
}

void TraCITDE::updateLastSeenTime(short carId, short counter, simtime_t messageTime){
    int i;
    for (i=1; i< counter; i++) {
        if (listedVehicles[i].id == carId) {
            EV << "Updating record for vehicle with ID " << carId << " with previously last seen at " << listedVehicles[i].lastSeenAt << ", now lastSeenAt = " << messageTime <<"."<<endl;
            listedVehicles[i].lastSeenAt = messageTime;
            break;
        }
    }
}

void TraCITDE::updateStats() {
    if(debugAppTDE==true) EV << "Updating statistics record" << endl;
    emit(vehNumberMC, currentNumberofDetectedMC);
    emit(vehNumberLV, currentNumberofDetectedLV);
    emit(vehNumberHV, currentNumberofDetectedHV);
}

/** delete timed out vehicle from the list and restructure the array */
void TraCITDE::deleteAndRestructureArray(short nodeIndex) {
    short i;
    if(listedVehicles[nodeIndex].id != myId){
    /** Decreasing counters */
    if (listedVehicles[nodeIndex].vType == "MC") currentNumberofDetectedMC--;
    if (listedVehicles[nodeIndex].vType == "LV") currentNumberofDetectedLV--;
    if (listedVehicles[nodeIndex].vType == "HV") currentNumberofDetectedHV--;
    currentNumberofTotalDetectedVehicles--;

    EV << "NODE " << myId << ": Deleting node with ID: " << listedVehicles[nodeIndex].id <<", type: " << listedVehicles[nodeIndex].vType << endl;
    for (i=nodeIndex; i<currentNumberofTotalDetectedVehicles; i++) listedVehicles[i] = listedVehicles[i+1];
    } else {
        EV << "Illegal operation: deleting myself from list." << endl;
    }
    if(debugAppTDE) showInfo(currentNumberofTotalDetectedVehicles);
}

/** Search for timed-out vehicle in the list, returning node ID of timed out vehicle */
short TraCITDE::searchForTimedOutVehicle() {
    short i;
    short returnedNodeIndex;

    for (i=1; i<=currentNumberofTotalDetectedVehicles; i++) {
        EV << "Checking DB for node with index " << i << endl;
        if (listedVehicles[i].lastSeenAt < (simTime()-timeoutInterval)) {
            EV << "NODE " << myId << ": Found obsolete item in list. ID " << listedVehicles[i].id << ", type: " << listedVehicles[i].vType << ". Last seen at: " << listedVehicles[i].lastSeenAt << " current simulation time is: " << simTime() << endl;
            returnedNodeIndex = i;
            break;
        }
    }
    return returnedNodeIndex;
}

void TraCITDE::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
    case SEND_BEACON_EVT: {
        WaveShortMessage* beacon = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
        beacon->setWsmData(myType.c_str());
        sendWSM(beacon);
        scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
        break;
    }
    case UPDATE_STATS: {
        updateStats();
        scheduleAt(simTime() + statInterval, updateStatMsg);
        break;
    }
    case CHECK_TIMEOUT: {
        if(currentNumberofTotalDetectedVehicles>1) deleteAndRestructureArray(searchForTimedOutVehicle());
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
