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

/////////////////////////////
//			   //
// TODOs for future rework //
//			   //
// * refactor TDMA slots   //
//			   //
/////////////////////////////

#include "veins/modules/mac/aribt109/AribT109AbstractMacLayer.h"
#include "veins/modules/utility/ConstsAribT109.h"
#include "veins/base/phyLayer/MacToPhyControlInfo.h"
#include "veins/modules/phy/Decider80211p.h"
#include "veins/base/phyLayer/PhyUtils.h"
#include "veins/base/phyLayer/BaseDecider.h"
#include <stdlib.h>

using Veins::Radio;

void AribT109AbstractMacLayer::initialize(int stage) {
    BaseMacLayer::initialize(stage);

    if (stage == 0) {
        phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(
                getParentModule());
        assert(phy11p);

        //this is required to circumvent double precision issues with constants from CONST80211p.h
        assert(simTime().getScaleExp() == SIMTIME_PS);

        txPower = par("txPower").doubleValue();
        if (txPower <= 0.0) {
            throw cRuntimeError("The transmission power is to small!");
        }
        //select 6mbs
        bitrate = BITRATES_ARIB[0];

        carrierFreq = par("carrierFrequency");
        phy11p->changeListeningFrequency(carrierFreq);
        phy11p->setCCAThreshold(par("ccaThreshold").doubleValue());

        myId = getParentModule()->getParentModule()->getFullPath();

        // "Start timer" and schedule reset
        resetHundredMsCycleTimerEvent = new cMessage("Reset the 100ms timer");
        resetOneSecondCycleTimerEvent = new cMessage("Reset the 1s timer");

        ivcPacketQueue = new cPacketQueue("IvcPacketQueue");

        sendIvcPacketEvent = new cMessage("SendIvcPacketEvent");
        sendMacPacketEvent = new cMessage("SendMacPacketEvent");
        sendMacPacketSequenceEvent = new cMessage("SendMacPacketSequeceEvent");

        // fill vector with 16 elements
        for (int i = 0; i < 16; i++) {
            roadsidePeriodInformationArray.push_back(
                    RoadsidePeriodInformation());
            transmissionControlArray.push_back(TransmissionControl());
        }
        roadsidePeriodBeginEvent = new cMessage("RoadsidePeriodBeginEvent");

        transmissionPeriodBeginEvent = new cMessage("TransmissionPeriodBegin");
        transmissionPeriodEndEvent = new cMessage("TransmissionPeriodEndEvent");
        currentPeriodIndex = 0;

        channelVirtuallyIdle = true;
        channelPhysicallyIdle = true;
        onGoingTransmission = false;

        /////////////////// signals & statistics ///////////////////

        // stuff

        lastBusy = simTime();
        lastIdle = simTime();

        statsTotalBusyTime = 0;

        // make sure to get the correct start time for evaluation of the mac busy time
        simtime_t n = simTime();
        simtime_t w = getSimulation()->getWarmupPeriod();
        if (n < w) {
            startTime = w;
        } else {
            startTime = n;
        }

        lastMacTime = simTime();

        // tell to anybody who is interested when the channel turns busy or idle
        sigChannelBusy = registerSignal("sigChannelBusy");
        // tell to anybody who is interested when a collision occurred
        sigCollision = registerSignal("sigCollision");

        sigSwitchToTxWhileBusy = registerSignal("sigSwitchToTxWhileBusy");

        // general numbers

        sigDataPacketToSend = registerSignal("sigDataPacketToSend");
        sigMacPacketSent = registerSignal("sigMacPacketSent");

        sigMacPacketReceived = registerSignal("sigMacPacketReceived");
        sigAddressedMacPacketReceived = registerSignal(
                "sigAddressedMacPacketReceived");

        sigDistributedSpace = registerSignal("sigDistributedSpace");

        // packet size stuff

        sigIvcDataBits = registerSignal("sigIvcDataBits");

        // discarded before transmission

        sigDataPacketDiscarded = registerSignal("sigDiscardedDataPackets");
        sigIvcPacketDiscarded = registerSignal("sigDiscardedIvcPackets");
        sigMacPacketDiscarded = registerSignal("sigDiscardedMacPackets");

        // discarded while transmission

        sigMacPacketDropped = registerSignal("sigMacPacketDropped");
        sigRecWhileSend = registerSignal("sigRecWhileSend");
        sigMacPacketReceivedWithError = registerSignal(
                "sigMacPacketReceivedWithError");

        // discarded after transmission

        sigReceivedMacPacketDiscarded = registerSignal(
                "sigDiscardedReceivedMacPackets");
        sigReceivedIvcPacketDiscarded = registerSignal(
                "sigDiscardedReceivedIvcPackets");

        // transmission times

        sigMacPacketTransmissionTime = registerSignal(
                "sigMacPacketTransmissionTime");
        sigIvcPacketTransmissionTime = registerSignal(
                "sigIvcPacketTransmissionTime");
        // basically, tdma delay
        sigIvcPacketQueueWaitingTime = registerSignal(
                "sigIvcPacketQueueWaitingTime");

        sigCsmaDelay = registerSignal("sigCsmaDelay");

        // communication control

        sigCurrentIvcPacketsInQueue = registerSignal(
                "sigCurrentIvcPacketsInQueue");
        sigReceivedWrongRvcInformation = registerSignal(
                "sigReceivedWrongRvcInformation");
        sigCurrentRvcInformation = registerSignal("sigCurrentRvcInformation");
        sigTimer = registerSignal("sigTimer");
        sigCurrentRvcTcInformation = registerSignal(
                "sigCurrentRvcTcInformation");

        // miscellaneous

    }
    rsu_period_start = 0;
    rsu_period_end = 0;
    vehicle_period_start = 0;
    vehicle_period_end = 0;
}

void AribT109AbstractMacLayer::finish() {
    if (sendMacPacketEvent->isScheduled()) {
        cancelAndDelete(sendMacPacketEvent);
    } else {
        delete sendMacPacketEvent;
    }

    recordScalar("lifeTime", simTime() - startTime);
    recordParametersAsScalars();
}

void AribT109AbstractMacLayer::resetHundredMsTimer() {
    hundredMsCycleTimerStart = simTime();
    scheduleAt((hundredMsCycleTimerStart + SimTime(100, SIMTIME_MS)),
            resetHundredMsCycleTimerEvent);

    EV
              << "Reseted the 100ms timer and canceled the roadside period begin event.";
    // when the 100ms timer is set to 0 the first roadside period begins implicitly
    currentPeriodIndex = 0;
}

void AribT109AbstractMacLayer::resetOneSecondTimer() {
    oneSecondCycleTimerStart = simTime();
    scheduleAt((oneSecondCycleTimerStart + SimTime(1, SIMTIME_S)),
            resetOneSecondCycleTimerEvent);

    cancelEvent(resetHundredMsCycleTimerEvent);

    resetHundredMsTimer();
}

const simtime_t AribT109AbstractMacLayer::getHundredMsCycleTimerValue() const {
    simtime_t result = (simTime() - hundredMsCycleTimerStart);
    if (result != getHundredMsCycleTimerValueAt(simTime())) {
        throw cRuntimeError("Error in 100ms timer value calculation!");
    }
    return result;
}

const simtime_t AribT109AbstractMacLayer::getHundredMsCycleTimerValueAt(
        simtime_t t) const {
    return (t - hundredMsCycleTimerStart);
}

const simtime_t AribT109AbstractMacLayer::getOneSecondTimerCycleTimerValue() const {
    simtime_t result = (simTime() - oneSecondCycleTimerStart);
    simtime_t st = getOneSecondTimerCycleTimerValueAt(simTime());
    if (result >= SimTime(1, SIMTIME_S)) {
        result = st;
    }

    if (result != st) {
        std::cerr << endl << simTime() << " - " << oneSecondCycleTimerStart
                << " = " << result.inUnit(SIMTIME_US) << " != "
                << st.inUnit(SIMTIME_US) << endl;

        throw cRuntimeError("Error in 1s timer value calculation!");
    }
    return result;
}

const simtime_t AribT109AbstractMacLayer::getOneSecondTimerCycleTimerValueAt(
        const simtime_t t) const {
    // assume that oneSecondCycleTimerStart <= simTime()
    // otherwise the timer start would be in the future which is not allowed

    if (oneSecondCycleTimerStart <= t) {
        // everything is fine, just calculate the difference
        simtime_t diff = t - oneSecondCycleTimerStart;
        // now we have to check the 1s overflow
        // if the given time is later than the timer is reseted because it reached 1s

        if (diff >= SimTime(1, SIMTIME_S)) {
            // we have to take case of the overflow
            double rest = diff.inUnit(SIMTIME_US)
                    % (SimTime(1, SIMTIME_S).inUnit(SIMTIME_US));
            return SimTime(rest, SIMTIME_US);
        } else {
            return diff; // maybe in micro seconds
        }
    } else {
        // now we have a problem
        throw cRuntimeError("Given timestamp was before beginning of timer!");
    }
}

void AribT109AbstractMacLayer::setCCAThreshold(double ccaThreshold_dBm) {
    phy11p->setCCAThreshold(ccaThreshold_dBm);
}

/**
 * Generate random waiting period according to standard
 */
simtime_t AribT109AbstractMacLayer::generateRandomWaitingPeriod() {
    return (uniform(0, 64) * SLOT_TIME_ARIB);
}

simtime_t AribT109AbstractMacLayer::getFrameDuration(
        int payloadLengthBits) const {
    // calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
    simtime_t duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION
            + T_SYM_80211P * ceil((16 + payloadLengthBits + 6) / 96);

    return duration;
}

void AribT109AbstractMacLayer::attachSignal(AribT109MacPacket* macPacket,
        simtime_t startTime, double frequency, uint64_t datarate,
        double txPower_mW) {
    simtime_t duration = getFrameDuration(macPacket->getBitLength());

    Signal* s = createSignal(startTime, duration, txPower_mW, datarate,
            frequency);
    MacToPhyControlInfo* cinfo = new MacToPhyControlInfo(s);

    macPacket->setControlInfo(cinfo);
}

Signal* AribT109AbstractMacLayer::createSignal(simtime_t start,
        simtime_t length, double power, uint64_t bitrate, double frequency) {
    simtime_t end = start + length;
    //create signal with start at current simtime and passed length
    Signal* s = new Signal(start, length);

    //create and set tx power mapping
    ConstMapping* txPowerMapping = createSingleFrequencyMapping(start, end,
            frequency, BW_OFDM_10_MHZ, power);
    s->setTransmissionPower(txPowerMapping);

    Mapping* bitrateMapping = MappingUtils::createMapping(
            DimensionSet::timeDomain(), Mapping::STEPS);

    Argument pos(start);
    bitrateMapping->setValue(pos, bitrate);

    pos.setTime(phyHeaderLength / bitrate);
    bitrateMapping->setValue(pos, bitrate);

    s->setBitrate(bitrateMapping);

    return s;
}

void AribT109AbstractMacLayer::handleSelfMsg(cMessage* msg) {
    if (msg == resetOneSecondCycleTimerEvent) {
        resetOneSecondTimer();
    } else if (msg == roadsidePeriodBeginEvent) {
        // We got an event for a begin of a new roadside period
        // Hence, our current period index is incremented
        currentPeriodIndex++;

        // We schedule an event for the next roadside period after the maximum length of the current period
        // we have to do it here again for the next period within in the 100ms control cycle
        simtime_t nextRoadsidePeriodBegin = simTime()
                + SimTime(TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US,
                        SIMTIME_US);
        scheduleAt(nextRoadsidePeriodBegin, roadsidePeriodBeginEvent);
        EV << "Scheduled event for next roadside period begin at "
                  << nextRoadsidePeriodBegin << endl;

        // We schedule an event for the begin of the transmission period of the current roadside period
        // Therefore, we use the index for the current roadside period (and subtract 1 for addressing in the vector)
        int begin =
                transmissionControlArray[(currentPeriodIndex - 1)].transmissionPeriodBegin;
        simtime_t transmissionPeriodBegin = simTime()
                + SimTime(begin, SIMTIME_US);

        if (transmissionPeriodBegin < simTime()) {
            std::cerr << transmissionPeriodBegin << " < " << simTime() << endl;
            transmissionPeriodBegin = simTime();
        }

        cancelEvent(transmissionPeriodBeginEvent);
        scheduleAt(transmissionPeriodBegin, transmissionPeriodBeginEvent);
        EV
                  << "Scheduled event for transmission period begin for current roadside period at "
                  << transmissionPeriodBegin << endl;

        // Schedule an event for the end of the transmission period
        // i.e. the end with some guard time
        // In fact, it makes sense to also subtract the minimum packet duration from this time
        // So we don not try to send if our packet is bigger then an a packet with no data
        // Hence, we do not run into collisions
        int duration =
                transmissionControlArray[(currentPeriodIndex - 1)].transmissionPeriodDuration;
	simtime_t calculatedPeriodEnd = transmissionPeriodBegin + SimTime(duration, SIMTIME_US);

        // subtract the length of an empty packet (there is not even enough time for sending an empty packet)
	// calculatedPeriodEnd -= getFrameDuration(288); // TODO
        if (calculatedPeriodEnd < simTime()) {
            std::cerr << calculatedPeriodEnd << " < " << simTime() << endl;
            calculatedPeriodEnd = simTime();
        }

        cancelEvent(transmissionPeriodEndEvent);
        scheduleAt(calculatedPeriodEnd, transmissionPeriodEndEvent);
        simtime_t rsu_period_duration = SimTime(roadsidePeriodInformationArray[currentPeriodIndex - 1].periodDuration, SIMTIME_US);
        simtime_t vehicle_period_duration = SimTime(TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US, SIMTIME_US) - rsu_period_duration;
        rsu_period_start = simTime();
        rsu_period_end = rsu_period_start + rsu_period_duration;
        vehicle_period_start = rsu_period_end;
        vehicle_period_end = vehicle_period_start + vehicle_period_duration;
    } else if (msg == transmissionPeriodEndEvent) {
        channelVirtuallyIdle = false;
        cancelEvent(sendMacPacketEvent);
        cancelEvent(sendIvcPacketEvent);
    } else {
        std::cerr << "Received unknown message: " << msg->getFullName() << endl;
	delete msg;
    }
}

void AribT109AbstractMacLayer::handleLowerControl(cMessage* msg) {
    switch (msg->getKind()) {
    case Mac80211pToPhy11pInterface::CHANNEL_BUSY:
        channelBusy();
        break;
    case Mac80211pToPhy11pInterface::CHANNEL_IDLE:
        channelIdle();
        break;
    case MacToPhyInterface::TX_OVER:
        EV << "Successfully transmitted a packet." << endl;
        phy->setRadioState(Radio::RX);
        channelIdleSelf();
        break;
    case Decider80211p::BITERROR:
        // this case is reached if the sender was too far away from the receiver
        // simply ignore this case in evaluation
        emit(sigMacPacketReceivedWithError, true);
        EV << this->getClassName() << " " << this->getId() << " " << simTime()
                  << ": A packet was not received due to a bit error" << endl;
        break;
    case Decider80211p::COLLISION:
        emit(sigCollision, true);
        EV << this->getClassName() << " " << this->getId() << " " << simTime()
                  << ": A packet was not received due to a collision" << endl;
        break;
    case Decider80211p::RECWHILESEND:
        emit(sigRecWhileSend, true);
        EV << this->getClassName() << " " << this->getId() << " " << simTime()
                  << ": A packet was not received because we were receiving while sending"
                  << endl;
        break;
    case MacToPhyInterface::RADIO_SWITCHING_OVER:
        EV << "Phy layer said radio switching is done" << endl;
        break;
    case BaseDecider::PACKET_DROPPED:
        emit(sigMacPacketDropped, true);
        EV << this->getClassName() << " " << this->getId() << " " << simTime()
                  << ": Phy layer said packet was dropped" << endl;
        break;
    default:
        std::cerr << "Invalid control message type: " << msg->getName()
                << ", module src = " << msg->getSenderModule()->getFullPath()
                << ", Message = " << msg->getKind() << "." << endl;
        break;
    }

    delete msg;
}

// compute hash value and emit
void AribT109AbstractMacLayer::emitRvcHash() {
    unsigned int b = 0;
    for (int i = 0; i < 16; i++) {
        b += (i + 1) * roadsidePeriodInformationArray[i].periodDuration;
    }

    emit(sigCurrentRvcInformation, b);
}

void AribT109AbstractMacLayer::emitRvcTcHash() {
    unsigned int hash = 0;
    for (int i = 0; i < 16; i++) {
        hash += roadsidePeriodInformationArray[i].transmissionCount;
    }

    emit(sigCurrentRvcTcInformation, hash);
}
