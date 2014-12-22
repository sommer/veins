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
// Department of Electrical Engineering, Gadjah Mada University
// Yogyakarta, INDONESIA
//
// Most parts of this file was derived from TraCIDemo11p.{cc,h} of Veins 3.0 by Dr.-Ing. Christoph Sommer#ifndef TraCITDE_H

#ifndef TraCITDE_H
#define TraCITDE_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

/**
 * @brief
 * IVC with Traffic Density Estimation Capability
 * Trying to exploit the exchanged beacon
 * as vehicle existence indicator.
 *
 * @author Daniel Febrian Sengkey
 */
class TraCITDE : public BaseWaveApplLayer {
private:
    /** @brief data collector  for storing number of detected vehicles*/
    simsignal_t vehNumberLV, vehNumberHV, vehNumberMC;

    /** @brief signal for storing vehicle type in integer */
    simsignal_t myTypeInt;

    struct knownVehicle {
        int id;
        std::string vType;
        simtime_t lastSeenAt;
    };
    bool debugAppTDE;

    /** @brief Declaring enumerated values for each type of vehicle.
     * This values have no use with application layer but important for post-processing.
     */
    enum vType { MC, LV, HV };

public:
    virtual void initialize(int stage);
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

protected:
    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    TraCICommandInterface::Vehicle* traciVehicle;
    AnnotationManager* annotations;
    simtime_t lastDroveAt;
    bool sentMessage;
    bool isParking;
    bool sendWhileParking;
    static const simsignalwrap_t parkingStateChangedSignal;

    /** @brief maximum number of listed vehicles for each node */
    static const short maxVehicles = 100;

    /** @brief array of listed vehicles */
    knownVehicle listedVehicles[100];

    short
    /** @brief counting variable for light vehicle */
    currentNumberofDetectedLV,

    /** @brief counting variable for heavy vehicle */
    currentNumberofDetectedHV,

    /** @brief counting variable for motorcylce */
    currentNumberofDetectedMC,

    /** @brief variable  that stores total number of detected vehicles */
    currentNumberofTotalDetectedVehicles;

    /** @brief variable containing the vehicle type */
    std::string myType;

    /** @brief variable storing vehicle type. Will be feed to scalar later */
    vType vTypeInt;

protected:
    /** @brief Method to execute while receiving a beacon */
    virtual void onBeacon(WaveShortMessage* wsm);

    /** @brief Method to execute while receiving a data */
    virtual void onData(WaveShortMessage* wsm);

    void sendMessage(std::string blockedRoadId);
    virtual void handlePositionUpdate(cObject* obj);
    virtual void handleParkingUpdate(cObject* obj);
    virtual void sendWSM(WaveShortMessage* wsm);

    /** @brief
     * Method to append vehicles to the list
     * @param carId is used to pass the address of sender node.
     * @param firstEmptyArrayIndex is used to pass the first empty array index to be found.
     */
    virtual void append2List(short carId, short firstEmptyArrayIndex, simtime_t messageTime, std::string vType);

    /** @brief
     * This method is used to determine if a vehicle already listed or not.
     * @param carId is used to pass the address of sender node.
     * @param counter is used to pass the number of listed vehicles as in currentNumberofVehicles.
     * @return indexing status of a car
     * @see currentNumberofVehicles
     */
    virtual bool indexedCar(short carId, short counter);

    /**
     * @brief
     * Printing list of detected vehicles and related information.
     * @param counter last array element.
     */
    virtual void showInfo(short counter);

    /**
     * @brief
     * Update the last seen time of an indexed vehicle
     */
    virtual void updateLastSeenTime(short carId, short counter, simtime_t messageTime);

    /** Overloading handleSelfMsg to include vType */
    virtual void handleSelfMsg(cMessage* msg);
};

#endif
