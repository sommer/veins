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

#include "veins/modules/mac/aribt109/AribT109BaseStationMacLayer.h"
#include "veins/modules/utility/ConstsAribT109.h"
#include "veins/modules/messages/aribt109/AribT109IvcRvcPacket_m.h"
#include "veins/modules/messages/aribt109/AribT109TransmissionControlMessage_m.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/base/phyLayer/MacToPhyControlInfo.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "veins/modules/phy/DeciderResult80211.h"
#include "veins/base/phyLayer/PhyUtils.h"
#include <omnetpp.h>

using Veins::Radio;

Define_Module(AribT109BaseStationMacLayer);

void AribT109BaseStationMacLayer::initialize(int stage) {
    AribT109AbstractMacLayer::initialize(stage);

    if (stage == 0) {
        // initialize rvc period information

        double maxRsuTxLength = par("maxRsuTxLength").doubleValue();

        // e.g. "1000,3000,580,1456,792,1324,887,1542,0,120,3012,545,325,2565,10,0"
        const char *rvcPeriodDurationsStr =
                par("rvcPeriodDurations").stringValue();
        std::vector<int> rvcPeriodDurationsVector = cStringTokenizer(
                rvcPeriodDurationsStr, ",").asIntVector();
        if (rvcPeriodDurationsVector.capacity() != 16) {
            throw cRuntimeError("Invalid number of rvc period durations.");
        }

        // e.g. "0,1500,400,0,0,0,0,0,0,0,0,0,0,0,0,0"
        const char *baseStationTransmissionPeriodStartsStr = par(
                "baseStationTransmissionPeriodStarts").stringValue();
        std::vector<int> baseStationTransmissionPeriodStartsVector =
                cStringTokenizer(baseStationTransmissionPeriodStartsStr, ",").asIntVector();
        if (baseStationTransmissionPeriodStartsVector.capacity() != 16) {
            exceptionMessage.clear();
            exceptionMessage
                    << "Invalid number of base station transmission period duration starts at base station "
                    << this->getId() << ".";
            throw cRuntimeError(exceptionMessage.str().c_str());
        }

        // e.g. "500,1000,100,0,0,0,0,0,0,0,0,0,0,0,0,0"
        const char *baseStationTransmissionPeriodDurationsStr = par(
                "baseStationTransmissionPeriodDurations").stringValue();
        std::vector<int> baseStationTransmissionPeriodDurationsVector =
                cStringTokenizer(baseStationTransmissionPeriodDurationsStr, ",").asIntVector();
        if (baseStationTransmissionPeriodDurationsVector.capacity() != 16) {
            exceptionMessage.clear();
            exceptionMessage
                    << "Invalid number of base station transmission period durations at base station "
                    << this->getId() << ".";
            throw cRuntimeError(exceptionMessage.str().c_str());
        }

        // check whether all period information are correct
        int periodLength, transmissionStart, transmissionDuration;
        int completeTransmissionLength = 0;
        for (int i = 0; i < 16; i++) {
            periodLength = rvcPeriodDurationsVector[i];
            if (periodLength < 0 || periodLength > 3024) {
                exceptionMessage.clear();
                exceptionMessage << "Invalid period duration value for period "
                        << i << ". Must be between 0 and 3024, but is: "
                        << periodLength;
                throw cRuntimeError(exceptionMessage.str().c_str());
            }
            transmissionStart = baseStationTransmissionPeriodStartsVector[i];
            if (transmissionStart < 0 || transmissionStart > periodLength) {
                exceptionMessage.clear();
                exceptionMessage << "Invalid transmission start for period "
                        << i << ". Must be between 0 and period length ("
                        << periodLength << "), but is " << transmissionStart;
                throw cRuntimeError(exceptionMessage.str().c_str());
            }
            transmissionDuration =
                    baseStationTransmissionPeriodDurationsVector[i];
            if (transmissionDuration < 0
                    || transmissionDuration > periodLength) {
                exceptionMessage.clear();
                exceptionMessage << "Invalid transmission duration for period "
                        << i << ". Must be between 0 and period length ("
                        << periodLength << "), but is " << transmissionDuration;
                throw cRuntimeError(exceptionMessage.str().c_str());
            }
            // can't check for sums of durations of other base stations
            roadsidePeriodInformationArray[i].periodDuration = periodLength;
            transmissionControlArray[i].transmissionPeriodBegin =
                    transmissionStart;
            transmissionControlArray[i].transmissionPeriodDuration =
                    transmissionDuration;
            completeTransmissionLength += transmissionDuration;

            // is initialized with thrice because this is a base station so the period information are valid and can be transmitted thrice
            roadsidePeriodInformationArray[i].transmissionCount =
                    TRANSFER_THRICE;
        }

        if ((completeTransmissionLength / 1000) > maxRsuTxLength) {
            exceptionMessage.clear();
            exceptionMessage << "Invalid transmission duration of "
                    << completeTransmissionLength << " us."
                    << "Complete transmission duration in 100ms should be <= 10.5ms";
            throw cRuntimeError(exceptionMessage.str().c_str());
        }

        resetOneSecondTimer();
        emitRvcHash();
        emit(sigTimer, getOneSecondTimerCycleTimerValue());
        emitRvcTcHash();
    }
}

void AribT109BaseStationMacLayer::finish() {
    recordParametersAsScalars();

    recordScalar("totalBusyTime", statsTotalBusyTime);

    AribT109AbstractMacLayer::finish();
}

void AribT109BaseStationMacLayer::handleLowerMsg(cMessage* msg) {
    AribT109MacPacket* macPacket = check_and_cast<AribT109MacPacket *>(msg);

    long dest = macPacket->getDestAddr();

    EV << "Received frame name = " << macPacket->getName() << ", src = "
              << macPacket->getSrcAddr() << ", dst = " << dest << ", myAddr = "
              << myMacAddr << endl;

    emit(sigMacPacketReceived, true);

    // MAC layer stuff

    DeciderResult80211* macRes =
            dynamic_cast<DeciderResult80211 *>(PhyToMacControlInfo::getDeciderResult(
                    macPacket));
    ASSERT(macRes);
    DeciderResult80211* res = new DeciderResult80211(*macRes);
    if (res->isCollision()) {
        throw cRuntimeError(
                "Assumed that only correct packet will be received here but got a packet with collision!");
    }

    emit(sigMacPacketTransmissionTime,
            (simTime() - macPacket->getMacLayerTimeStamp()));

    // Actually, only broadcast is used in this standard.
    // But this makes it possible, to use addressed messages.
    if (dest == myMacAddr || dest == LAddress::L2BROADCAST()) {
        EV << "Received a data packet addressed to me." << endl;

        emit(sigAddressedMacPacketReceived, true);

        // a base station does not need to sync it's clock because in this model it is not unsynchronized
        // in the real world the base stations would sync it's clocks over GPS (see Description 2 in standard)

        // send MA-UNITDATA.indication
        AribT109IvcRvcPacket* ivcPacket = check_and_cast<AribT109IvcRvcPacket*>(
                macPacket->decapsulate());

        // IVC layer stuff

        // check whether the received ivc packet is big enough (i.e. at least the header size)
        if (ivcPacket->getByteLength() >= 22) {
            emit(sigIvcPacketTransmissionTime,
                    (simTime() - ivcPacket->getIvcLayerArivalTimeStamp()));

            // the received information should be at least greater than NO_TRANSFER
            RoadsidePeriodInformationVector r =
                    ivcPacket->getRoadsidePeriodInformationArray();

//            int no;
//            for (unsigned int i = 0; i < r.size(); i++)
//            {
//                if (r[i].transmissionCount > NO_TRANSFER)
//                {
//                    no++;
//                }
//            }

            // pass received frame to upper layer
            cPacket* dataPkt = check_and_cast<cPacket *>(
                    ivcPacket->decapsulate());

            dataPkt->setControlInfo(new PhyToMacControlInfo(res));

            sendUp(dataPkt);
        } else {
            EV << "Received IVC packet was to small, deleting..." << endl;
            emit(sigIvcPacketDiscarded, true);
            delete ivcPacket;
        }
    } else {
        EV << "Packet not for me, deleting..." << endl;
        emit(sigReceivedMacPacketDiscarded, true);
        delete macPacket;
    }
    delete msg;
}

void AribT109BaseStationMacLayer::handleSelfMsg(cMessage* msg) {
    EV << "Received message " << msg->getFullName() << endl;

    if (msg == resetHundredMsCycleTimerEvent) {
        resetHundredMsTimer();

        cancelEvent(roadsidePeriodBeginEvent);

        simtime_t nextRoadsidePeriodBegin = simTime()
                + SimTime(TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US,
                        SIMTIME_US);
        scheduleAt(nextRoadsidePeriodBegin, roadsidePeriodBeginEvent);
        EV << "Scheduled event for next roadside period begin at "
                  << nextRoadsidePeriodBegin << endl;

        emitRvcHash();
        emit(sigTimer, getOneSecondTimerCycleTimerValue());
        emitRvcTcHash();
    } else if (msg == transmissionPeriodBeginEvent) {
        channelVirtuallyIdle = true;

        // we came to the point in time where we are allowed to transmit packets from our queue
        // we schedule an event for sending a mac packet
        // Therefore, we use the current time because we are now allowed to transmit a packet
        cancelEvent(sendMacPacketEvent);
        scheduleAt(simTime(), sendMacPacketEvent);
        EV << "Scheduled event for transmission of a mac packet." << endl;
        // maybe cancel mac pkt event
    } else if (msg == sendMacPacketEvent) {
        EV << "Received an event for sending a MAC packet." << endl;

        if (channelVirtuallyIdle) {

            // We are at the point in time where we can actually transmit a packet right now
            // check whether we have some packets
            if (ivcPacketQueue->getLength() > 0) {
                EV
                          << "There is at least one packet in the queue which we can send."
                          << endl;

                // get packet and check whether it can be send
                AribT109IvcRvcPacket* ivcPacket = check_and_cast<
                        AribT109IvcRvcPacket *>(ivcPacketQueue->front());

                // duration of SIFS + IVC packet + MAC overhead
                simtime_t currentFrameDuration = getFrameDuration(
                        (ivcPacket->getBitLength() + 288));
                // We calculate the duration of the current transmission period to determine whether we can send the current packet in the current transmission period
                // Therefore, we use the index for the current roadside period (and subtract 1 for addressing in the vector)
		TransmissionControl currentPeriod = transmissionControlArray[(currentPeriodIndex - 1)];
                simtime_t currentTransmissionPeriodDuration =
                        SimTime(currentPeriod.transmissionPeriodDuration, SIMTIME_US);
                EV << "The current transmission period duration is " << currentTransmissionPeriodDuration << endl;

                simtime_t completePacketDuration = currentFrameDuration
                        + SIFS_ARIB;

		// in case we want to send another packet in this transmission period
		// or there is a delay between the beginning of the period and the actual sending
		// we have to check the actual time which is left in this period
		// for the first packet in a period this time should be the same as the complete period duration

		// sum up the time of all complete rvc periods before the current one
		// beginn of the current rvc period
		// TODO fix calculation in future rework
		simtime_t currentRvcPeriodStart = (currentPeriodIndex) * SimTime(TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US, SIMTIME_US);
		simtime_t currentTransmissionPeriodStart = SimTime(currentPeriod.transmissionPeriodBegin, SIMTIME_US) + currentRvcPeriodStart; // since it is an absolute value (e.g. 1500)
		simtime_t currentTransmissionPeriodEnd = currentTransmissionPeriodStart + SimTime(currentPeriod.transmissionPeriodDuration, SIMTIME_US);
		simtime_t currentTimerValue = getHundredMsCycleTimerValue();
		if (currentTimerValue < currentTransmissionPeriodStart || currentTimerValue > currentTransmissionPeriodEnd) {
			exceptionMessage.clear();
		        exceptionMessage
			<< "The current value of the 100.000 us timer"
			<< " (" << currentTimerValue << ") "
			<< "is out of the range of the current transmission period"
			<< " (#" << currentPeriodIndex << ": " << currentTransmissionPeriodStart << ", " << currentTransmissionPeriodEnd << ")."
			<< " Something is wrong here!";
			//throw cRuntimeError(exceptionMessage.str().c_str());
			//std::cout << exceptionMessage.str() << endl;
			return;
		}
		simtime_t actualTimeLeftInTransmissionPeriod = currentTransmissionPeriodEnd - currentTimerValue;
		if (actualTimeLeftInTransmissionPeriod > currentTransmissionPeriodDuration) {
			throw cRuntimeError("The calculated time left in the current transmission period is bigger then the time this transmission period actually has!");
		}

                // check whether current packet can be send in current transmission period
                if (completePacketDuration <= actualTimeLeftInTransmissionPeriod) {
                    // packet is small enough to be send in the current transmission period
                    // actually "send" the packet

                    EV << "The packet to transmit is small enough to be sent in the current transmission period." << endl;

                    // check the current channel state
                    if (!onGoingTransmission && channelPhysicallyIdle) {
                        if (onGoingTransmission) {
                            throw cRuntimeError(
                                    "channel is said to be idle but a transmission is ongoing.");
                        }
                        if (!channelPhysicallyIdle) {
                            throw cRuntimeError(
                                    "channel is said to be idle but we receive packets");
                        }

                        // the channel was idle for the period of distributed space and a random waiting period (otherwise this mac event would have been canceled)
                        // we are allowed to send now
                        channelBusySelf();

                        ivcPacket = check_and_cast<AribT109IvcRvcPacket *>(
                                ivcPacketQueue->pop());

                        // csma waiting time
                        emit(sigIvcPacketQueueWaitingTime, simTime() - ivcPacket->getIvcLayerArivalTimeStamp());
                        emit(sigCurrentIvcPacketsInQueue,
                                ivcPacketQueue->getLength());

                        AribT109MacPacket* macPacket = new AribT109MacPacket(
                                ivcPacket->getName(), ivcPacket->getKind());

                        macPacket->setMacLayerTimeStamp(simTime());

                        // set length of MAC control field (24B) + LLC control field (8B) + FCS (4B)
                        macPacket->setByteLength(36);

                        AribT109TransmissionControlMessage* ivcControlInfo =
                                check_and_cast<
                                        AribT109TransmissionControlMessage *>(
                                        ivcPacket->getControlInfo());

                        // set destination to value from upper layer
                        macPacket->setDestAddr(
                                ivcControlInfo->getLinkAddress());
                        macPacket->setSrcAddr(myMacAddr);

                        transmissionCounter++;
                        macPacket->setTransmissionCount(transmissionCounter);

                        macPacket->setWirelessCallNumber(
                                "Some string as wireless call number");

                        EV
                                  << "The sending duration of the current packet will be"
                                  << currentFrameDuration << endl;

                        // set value of 1s timer when the preamble arrives at the antenna send
                        // corresponding to the delay from below
                        ivcPacket->setAribTimeStamp(
                                getOneSecondTimerCycleTimerValueAt(
                                        simTime() + RADIODELAY_11P));

                        if (debug) {
                            debugEV
                                           << "RoadsidePeriodInformation of current packet will follow:"
                                           << endl;
                            for (int i = 0; i < 16; i++) {
                                debugEV << "Period " << i << ": "
                                               << ivcPacket->getRoadsidePeriodInformationArray()[i].periodDuration
                                               << " - "
                                               << ivcPacket->getRoadsidePeriodInformationArray()[i].transmissionCount
                                               << endl;
                            }
                        }
                        macPacket->encapsulate(ivcPacket);

                        // set according to received value from upper layer (parameter control info)
                        // get phy control information from data packet
                        PhyControlMessage *controlInfo =
                                dynamic_cast<PhyControlMessage *>(ivcPacket->getEncapsulatedPacket()->getControlInfo());

                        double txPower_mW = txPower;

                        if (controlInfo) {
                            txPower_mW = controlInfo->getTxPower_mW();
                            if (txPower_mW < 0) {
                                txPower_mW = txPower;
                            }
                        }

                        // switch to TX
                        // the time this takes ranges from 0 to RADIODELAY_11P
                        // Hopefully, we get correct results here
                        // Just consider the case that we already are in TX but still wait RADIODELAY_11p for sending later on
                        phy->setRadioState(Radio::TX);

                        attachSignal(macPacket,
                                simTime() + RADIODELAY_11P + SIFS_ARIB,
                                carrierFreq, bitrate, txPower_mW);

                        EV << "Attached signal to the mac packet." << endl;

                        // Time for switching the antenna to transmission
                        // NOTE: That will generate a bit inaccurate results because the module is now allowed to send
                        // but maybe not more after waiting for the radio to switch
                        sendDelayed(macPacket, RADIODELAY_11P + SIFS_ARIB,
                                lowerLayerOut);

                        emit(sigDistributedSpace,
                                (simTime() - lastMacTime).inUnit(SIMTIME_US));
                        lastMacTime = simTime();

                        emit(sigMacPacketSent, macPacket);

                        EV << "Actually sent the packet to the PHY!" << endl;

                        // schedule an event for sending the next ivc packet
                        // if there is enough time left
                        // consider at least the time passed by sending the current packet
                        // the rest is handled by the transmission end event
                        if (actualTimeLeftInTransmissionPeriod - (completePacketDuration + RADIODELAY_11P) >= getFrameDuration(288)) {
                            // there still is enough time in this period to send another packet
                            // send another packet
                            EV << "There is enough time left in the current transmission period to transmit another packet, so another mac event will be scheduled." << endl;
                            scheduleAt(simTime() + completePacketDuration + RADIODELAY_11P, sendMacPacketEvent);
                        } else {
                            // the next packet has to wait until the next transmission period
                        }

                    } else {
                        /// the channel is busy
                        // so we have start over
                        // the channel has to be idle at least for the time of the distributed space
                        // but we can check again after a small amount of time

                        // do nothing
                        // we do not have to put the current ivc packet back into the queue because we just got the front element of the queue
                        // but did not deleted it from the queue
                    }
                } else {
                    // packet was to big to be sent in this transmission period
                    // wait until next transmission period
                    EV << "The current packet ivc packet ("
                              << ivcPacket->getEncapsulatedPacket()->getFullName()
                              << ", id=" << ivcPacket->getId()
                              << ") is to big to be transmitted in the current transmission period ("
                              << currentPeriodIndex << ", "
                              << currentFrameDuration << " > "
                              << currentTransmissionPeriodDuration << ")"
                              << endl;
                }

            } else {
                // queue is empty
                // do nothing
                EV
                          << "Received event for sending a MAC packet but there is no data in the queue."
                          << endl;
            }
        } else {
            // the channel is not virtually idle anymore
            // do nothing
        }
    } else {
        AribT109AbstractMacLayer::handleSelfMsg(msg);
    }
}

/**
 * Add packet to transmission queue and schedule transmission.
 */
void AribT109BaseStationMacLayer::handleUpperMsg(cMessage* msg) {
    cPacket* dataPkt = check_and_cast<cPacket*>(msg);

    EV << "Received a message from upper layer " << dataPkt->getName() << endl;

    emit(sigDataPacketToSend, dataPkt);

    if (dataPkt->getByteLength() <= 1500) {
        EV << "Data length is small enough to be sent." << endl;

        // extract SequenceNumber (not used), ControlInformation, LinkAddress from data (APDU)
        int packetNumber = dataPkt->getId();

        // do IVC packet stuff: generate IPDU

        AribT109IvcRvcPacket* ivcPacket = new AribT109IvcRvcPacket(
                dataPkt->getName(), dataPkt->getKind());

        ivcPacket->setIvcLayerArivalTimeStamp(simTime());

        // set length of IR control field (header)
        ivcPacket->setByteLength(22);

        ivcPacket->setIsFromBaseStation(true);
        ivcPacket->setSynchronized(true);
        ivcPacket->setMobileStationSynchronizationStatus(SYNCHRONIZED_DIRECT);

        // set RVC period information by getting the registered values
        ivcPacket->setRoadsidePeriodInformationArray(
                roadsidePeriodInformationArray);

        debugEV << "RoadsidePeriodInformation will follow:" << endl;
        for (int i = 0; i < 16; i++) {
            debugEV << "Period " << i << ": "
                           << ivcPacket->getRoadsidePeriodInformationArray()[i].periodDuration
                           << " - "
                           << ivcPacket->getRoadsidePeriodInformationArray()[i].transmissionCount
                           << endl;

        }

        // give data (packet itself) & [control info:] LinkAddress (destination address), SequenceNumber (see app layer 4.5.2.1.4) and ControlInformation (data rate) to mac layer

        ivcPacket->encapsulate(dataPkt);

        AribT109TransmissionControlMessage* ivcControl =
                new AribT109TransmissionControlMessage();

        ivcControl->setLinkAddress(LAddress::L2BROADCAST());
        ivcControl->setPacketNumber(packetNumber);

        ivcPacket->setControlInfo(ivcControl);

        emit(sigIvcDataBits, ivcPacket->getBitLength());

        // ready to send from perspective of IVC layer

        // LLC sub-layer is left out --> MA-UNITDATA.request SequenceNumber (see above), LinkAddress (see above), data & control field (see above), ControlInformation (see above)

        // MAC layer

        // store MSDU (IVC packet)
        ivcPacketQueue->insert(ivcPacket);
        emit(sigCurrentIvcPacketsInQueue, ivcPacketQueue->getLength());
    } else {
        delete msg;
        EV << "Data was to big, Message was deleted." << endl;
        emit(sigDataPacketDiscarded, true);
    }
}

void AribT109BaseStationMacLayer::channelBusy() {
    // the channel turned busy because we hear someone, who is not us, sending

    channelPhysicallyIdle = false;

    EV << "Channel turned busy because of an external sender" << endl;
    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        lastBusy = simTime();
    }

    cancelEvent(sendMacPacketEvent);
    emit(sigChannelBusy, true);
}

void AribT109BaseStationMacLayer::channelBusySelf() {
    // we started a transmission

    onGoingTransmission = true;
    EV << "Transmission started" << endl;

    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        lastBusy = simTime();
    }

    cancelEvent(sendMacPacketEvent);
    emit(sigChannelBusy, true);
}

void AribT109BaseStationMacLayer::channelIdle() {
    // the channel turned idle after someone, who was not us, has sent something

    channelPhysicallyIdle = true;
    EV << "Channel turned idle after external transmission" << endl;

    if (!onGoingTransmission) {
        // channel is idle now

        if (simTime() >= getSimulation()->getWarmupPeriod()) {
            lastIdle = simTime();
            statsTotalBusyTime += simTime() - lastBusy;
        }

        // schedule new mac event
        cancelEvent(sendMacPacketEvent);
        scheduleAt(simTime(), sendMacPacketEvent);
    }
}

void AribT109BaseStationMacLayer::channelIdleSelf() {
    // our transmission is over

    onGoingTransmission = false;
    EV << "Transmission is over" << endl;

    if (channelPhysicallyIdle) {
        // channel is idle now
        if (simTime() >= getSimulation()->getWarmupPeriod()) {
            lastIdle = simTime();
            statsTotalBusyTime += simTime() - lastBusy;
        }

        // schedule new mac event
        cancelEvent(sendMacPacketEvent);
        scheduleAt(simTime(), sendMacPacketEvent);
    }
}
