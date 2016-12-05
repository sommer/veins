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

#ifndef ABSTRACTARIBT109MAC_H_
#define ABSTRACTARIBT109MAC_H_

#include "veins/base/modules/BaseMacLayer.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"
#include "veins/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h"
#include "veins/modules/messages/aribt109/AribT109MacPacket_m.h"
#include "veins/modules/mac/aribt109/RoadsidePeriodInformation.h"
#include "veins/modules/mac/aribt109/TransmissionControl.h"
#include "veins/modules/mac/aribt109/RoadsidePeriodInformationEntry.h"
#include "veins/modules/utility/ConstsPhy.h"
#include "veins/modules/utility/ConstsAribT109.h"
#include "veins/base/utils/FindModule.h"
#include <omnetpp.h>

/**
 * @brief
 *
 * Manages virtual and physical carrier sense function for medium access.
 * Does not manage logical links.
 *
 */
class AribT109AbstractMacLayer : public BaseMacLayer,
	 public WaveAppToMac1609_4Interface {

public:
    AribT109AbstractMacLayer() {
    }
    ;

    // OMNeT functions

    /** @brief Initialize the module and variables.*/
    virtual void initialize(int stage);
    /** @brief Delete all dynamically allocated objects of the module.*/
    virtual void finish();

    /** @brief Handle messages from lower layer. Is called from handleMessage of super class. */
    virtual void handleLowerMsg(cMessage* msg) = 0;
    /** @brief Handle messages from upper layer. Is called from handleMessage of super class.*/
    virtual void handleUpperMsg(cMessage* msg) = 0;
    /** @brief Handle self messages such as timers. Is called from handleMessage of super class.*/
    virtual void handleSelfMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer.*/
    virtual void handleLowerControl(cMessage* msg);

    /**
     * @brief Change the phy layer carrier sense threshold.
     *
     * @param ccaThreshold_dBm the cca threshold in dBm
     */
    void setCCAThreshold(double ccaThreshold_dBm);

    void attachSignal(AribT109MacPacket* macPacket, simtime_t startTime,
            double frequency, uint64_t datarate, double txPower_mW);
    Signal* createSignal(simtime_t start, simtime_t length, double power,
            uint64_t bitrate, double frequency);

    /**
     * Only for compliance with current Veins version
     */
    bool isChannelSwitchingActive() {return false;}
    simtime_t getSwitchingInterval() {return SimTime(0.050);}
    bool isCurrentChannelCCH() {return true;}
    void changeServiceChannel(int channelNumber) {}

protected:

    virtual void channelIdle() = 0;
    virtual void channelBusy() = 0;
    virtual void channelBusySelf() = 0;
    virtual void channelIdleSelf() = 0;

    /** @brief The power (in mW) to transmit with.*/
    double txPower;

    /** @brief the bit rate at which we transmit */
    uint64_t bitrate;

    double carrierFreq;

    /** @brief N_DBPS, derived from bitrate, for frame length calculation */
    double n_dbps;

    /** @brief Id for debug messages */
    std::string myId;

    // MAC parameters

    // determines whether we are currently transmitting a packet
    bool onGoingTransmission;
    // determines whether we are currently receiving packets from other nodes
    bool channelPhysicallyIdle;

    cMessage* sendMacPacketEvent;

    /**
     * @brief Event for actually sending a sequence of packets.
     */
    cMessage* sendMacPacketSequenceEvent;

    /**
     * Reference tp PHY
     */
    Mac80211pToPhy11pInterface* phy11p;

    int transmissionCounter;

    // IVC parameters

    bool channelVirtuallyIdle;

    /**
     * @brief This queue holds the packets which arrived from the APP layer and shall be sent.
     */
    cPacketQueue* ivcPacketQueue;

    cMessage* sendIvcPacketEvent;

    /**
     * @brief This parameter holds the value of the 1s cycle time which is used for IVC synchronization.
     */
    simtime_t oneSecondCycleTimerStart;

    cMessage* resetOneSecondCycleTimerEvent;

    /**
     * @brief This parameter holds the value of the 100ms cycle timer which is used as an control cycle.
     */
    simtime_t hundredMsCycleTimerStart;

    cMessage* resetHundredMsCycleTimerEvent;

    /////////////////// Period information (for base & mobile stations) ///////////////////

    /**
     * Roadside-to-Vehicle period information field
     *
     * Represents the information of the Roadside-to-Vehicle periods (RRC for base stations or OTI[1.16] for mobile stations)
     * Will be transferred to mobile stations for synchronization
     *
     * Array shall hold max 16 entries.
     */
    std::vector<RoadsidePeriodInformation> roadsidePeriodInformationArray;

    cMessage* roadsidePeriodBeginEvent;

    cMessage* transmissionPeriodEndEvent;

    int currentPeriodIndex;

    /////////////////// Transmission Control (for base & mobile stations ///////////////////

    /*
     * Transmission period control in base stations:
     * Used for transmission control to determine at which times a particular base station actually transmits (RTC).
     *
     * Transmission inhibition period control in mobile stations (ONC[1..16]):
     * Represents the transmission inhibition times for a mobile station
     * This values are inverse to the roadside period information.
     */
    std::vector<TransmissionControl> transmissionControlArray;

    cMessage* transmissionPeriodBeginEvent;

    std::stringstream exceptionMessage;

    /**
     * Returns the value of the 100ms cycle timer (in ms)
     */
    const simtime_t getHundredMsCycleTimerValue() const;

    /**
     * Returns the value of the 100ms cycle timer at time t (in ms)
     */
    const simtime_t getHundredMsCycleTimerValueAt(const simtime_t t) const;

    /**
     * Returns the value of the 1s cycle timer (in ms)
     */
    const simtime_t getOneSecondTimerCycleTimerValue() const;

    /**
     * Returns the value of the 1s cycle timer at time t (in ms)
     */
    const simtime_t getOneSecondTimerCycleTimerValueAt(const simtime_t t) const;

    /**
     *  Generates a random waiting period in SimTime.
     */
    simtime_t generateRandomWaitingPeriod();

    void resetHundredMsTimer();
    void resetOneSecondTimer();

    simtime_t getFrameDuration(int payloadLengthBits) const;

    /////////////////// signals & statistics ///////////////////

    // stuff

    simtime_t lastBusy;
    simtime_t lastIdle;

    simtime_t lastMacTime;

    simtime_t startTime;

    // tell to anybody which is interested when the channel turns busy or idle
    simsignal_t sigChannelBusy;
    // tell to anybody which is interested when a collision occurred
    simsignal_t sigCollision;

    simsignal_t sigSwitchToTxWhileBusy;

    // general numbers

    simsignal_t sigDataPacketToSend;
    simsignal_t sigMacPacketSent;

    simsignal_t sigMacPacketReceived;
    simsignal_t sigAddressedMacPacketReceived;

    // logging the distributed space between to mac packets
    simsignal_t sigDistributedSpace;

    // packet size stuff

    simsignal_t sigIvcDataBits;

    // discarded before transmission
    simsignal_t sigDataPacketDiscarded;
    simsignal_t sigIvcPacketDiscarded;
    simsignal_t sigMacPacketDiscarded;

    // discarded while transmission

    simsignal_t sigMacPacketDropped;
    simsignal_t sigRecWhileSend;
    simsignal_t sigMacPacketReceivedWithError;

    // discarded after transmission

    simsignal_t sigReceivedMacPacketDiscarded;
    simsignal_t sigReceivedIvcPacketDiscarded;

    // transmission times

    simsignal_t sigMacPacketTransmissionTime;
    simsignal_t sigIvcPacketTransmissionTime;
    // basically, tdma delay
    simsignal_t sigIvcPacketQueueWaitingTime;

    simsignal_t sigCsmaDelay;

    // communication control

    simsignal_t sigCurrentIvcPacketsInQueue;
    simsignal_t sigReceivedWrongRvcInformation;

    simsignal_t sigCurrentRvcInformation;
    simsignal_t sigTimer;
    simsignal_t sigCurrentRvcTcInformation;

    void emitRvcHash();
    void emitRvcTcHash();

    // miscellaneous

    simtime_t statsTotalBusyTime;
    simtime_t rsu_period_start;
    simtime_t rsu_period_end;
    simtime_t vehicle_period_start;
    simtime_t vehicle_period_end;
};

#endif /* ABSTRACTARIBT109MAC_H_ */
