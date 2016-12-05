//
// Copyright (C) 2016 Julian Heinovski <julian.heinovski@ccs-labs.org>
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
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef MOBILESTATIONARIBT109MAC_H_
#define MOBILESTATIONARIBT109MAC_H_

#include "veins/modules/mac/aribt109/AribT109AbstractMacLayer.h"
#include "veins/modules/mac/aribt109/SynchronizationStatus.h"

class AribT109MobileStationMacLayer: public AribT109AbstractMacLayer {
public:
    AribT109MobileStationMacLayer() {
    }
    ;

    // OMNeT functions

    /** @brief Initialize the module and variables.*/
    void initialize(int stage);
    /** @brief Delete all dynamically allocated objects of the module.*/
    void finish();

    /** @brief Handle messages from lower layer. Is called from handleMessage of super class. */
    void handleLowerMsg(cMessage* msg);
    /** @brief Handle messages from upper layer. Is called from handleMessage of super class.*/
    void handleUpperMsg(cMessage* msg);
    /** @brief Handle self messages such as timers. Is called from handleMessage of super class.*/
    void handleSelfMsg(cMessage* msg);

protected:
    void channelIdle();
    void channelBusy();
    void channelBusySelf();
    void channelIdleSelf();

private:

    // IVC parameters

    /////////////////// Synchronization Information Variable (ORT.SYN) ///////////////////

    /**
     * Roadside-to-Vehicle period information table - Synchronization information - status
     * Represents the roadside period synchronization status of this particular mobile station
     * ORT.SYN.STA
     */
    SynchronizationStatus roadsideSynchronizationStatus;

    /**
     * Roadside-to-Vehicle period information table - Synchronization information - elapsed time
     * Represents the time elapsed since updating the synchronization status
     * ORT.SYN.ELT
     */
    simtime_t roadsideSynchronizationTimeStamp;

    /**
     * Message variable for scheduling the time elapsed event
     */
    cMessage* roadsideSynchronizationStatusValidTimeElapsedEvent;

    /////////////////// RVC Period Information Valid Time Variable (ORV) ///////////////////

    /**
     * Roadside-to-Vehicle period information valid time
     * ORV
     * Is used for the overall synchronization status and each period information entry
     * The value is in us.
     * The received roadside period information shall be updated every time this field elapses
     * Values 300..65536
     */
    simtime_t roadsidePeriodInformationValidTime;

    /**
     * Message variable for scheduling the time elapsed event
     */
    cMessage* roadsidePeriodInformationValidTimeElapsedEvent;

    /////////////////// Collected RVC Information ///////////////////

    /**
     * Roadside-to-Vehicle period information entries
     * Multiple different values for the period information are stored here.
     * These values are received from different other participants in the network.
     *
     * Is used for maintaining the own rvc period information
     *
     * ORT.ENT[r]
     */
    std::vector<RoadsidePeriodInformationEntry> collectedRoadsidePeriodInformationEntries;

    /**
     * @brief Holds the corrected value of the 100ms cycle timer
     * TC
     */
    int correctedTimerValue;

    /**
     * Adds the given value to the 1s cycle timer
     */
    void correctOneSecondTimerBy(const simtime_t correctedTimerValue);

    void updateRoadsideSynchronizationStatus();
    void onRoadsideSynchronizationStatusUpdate();

    void onTimerValueModified(const simtime_t correctedTimerValue);
    void printMaintainedRvcInformation();

    simtime_t syncOffset;

    // stats
    long statsRoadsideSynchronizationStatusElapseTimeResets;

    simsignal_t sigSyncStatus;
    simsignal_t sigCorrectTimerByValue;
    simsignal_t sigInvalidControlFieldReceived;
};

#endif /* MOBILESTATIONARIBT109MAC_H_ */
