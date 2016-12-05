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

#include "veins/modules/mac/aribt109/AribT109MobileStationMacLayer.h"
#include "veins/modules/utility/ConstsAribT109.h"
#include "veins/modules/messages/aribt109/AribT109IvcRvcPacket_m.h"
#include "veins/modules/messages/aribt109/AribT109TransmissionControlMessage_m.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/base/phyLayer/MacToPhyControlInfo.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "veins/modules/phy/DeciderResult80211.h"

using Veins::Radio;

Define_Module(AribT109MobileStationMacLayer);

void AribT109MobileStationMacLayer::initialize(int stage) {
    AribT109AbstractMacLayer::initialize(stage);

    if (stage == 0) {
        // introduce a little asynchronization between radios
        syncOffset = dblrand() * par("syncOffset").doubleValue();

        scheduleAt(simTime() + syncOffset, resetOneSecondCycleTimerEvent);

        roadsidePeriodInformationValidTime = par(
                "roadsidePeriodInformationValidTime");
        roadsideSynchronizationStatusValidTimeElapsedEvent = new cMessage(
                "RoadsidePeriodInformationValidTimeElapsed");

        roadsideSynchronizationStatus = UNSYNCHRONIZED;
        updateRoadsideSynchronizationStatus();

        correctedTimerValue = 0;

        // initialize rvc period information

        // fill vector with 16 elements
        for (int i = 0; i < 16; i++) {
            int roadsidePeriodDuration = 0;
            roadsidePeriodInformationArray[i].periodDuration =
                    roadsidePeriodDuration;
            transmissionControlArray[i].transmissionPeriodBegin = 0
                    + roadsidePeriodDuration;
            // determine the duration of the current transmission period duration by subtracting the RVC period duration and another guard time at the end of the RVC period
            transmissionControlArray[i].transmissionPeriodDuration =
                    TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US
                            - roadsidePeriodDuration
                            - GUARD_TIME_ARIB.inUnit(SIMTIME_US);

            // initilaze with 0 because we do not have valid rvc information at start up
            roadsidePeriodInformationArray[i].transmissionCount = NO_TRANSFER;
        }

        channelPhysicallyIdle = true;

        sigSyncStatus = registerSignal("sigSyncStatus");
        sigCorrectTimerByValue = registerSignal("sigCorrectTimerByValue");
        sigInvalidControlFieldReceived = registerSignal(
                "sigInvalidControlFieldReceived");

        emit(sigSyncStatus, roadsideSynchronizationStatus);
        emitRvcHash();
        emit(sigTimer, getOneSecondTimerCycleTimerValue());
        emitRvcTcHash();

        statsRoadsideSynchronizationStatusElapseTimeResets = 0;
    }
}

void AribT109MobileStationMacLayer::finish() {

    if (sendIvcPacketEvent->isScheduled()) {
        cancelAndDelete(sendIvcPacketEvent);
    } else {
        delete sendIvcPacketEvent;
    }

    recordScalar("syncOffset", syncOffset);
    recordScalar("totalRoadsideSynchronizationStatusElapseTimeResets",
            statsRoadsideSynchronizationStatusElapseTimeResets);

    recordScalar("totalBusyTime", statsTotalBusyTime);

    recordParametersAsScalars();

    AribT109AbstractMacLayer::finish();
}

void AribT109MobileStationMacLayer::printMaintainedRvcInformation() {
    std::cout << "Current collected entries: " << endl;
    for (unsigned int i = 0;
            i < collectedRoadsidePeriodInformationEntries.size(); i++) {
        std::cout << "Period "
                << collectedRoadsidePeriodInformationEntries[i].periodNumber
                << ": duration = "
                << collectedRoadsidePeriodInformationEntries[i].periodInformation.periodDuration
                << ", tmc = "
                << collectedRoadsidePeriodInformationEntries[i].periodInformation.transmissionCount
                << " from sender "
                << collectedRoadsidePeriodInformationEntries[i].senderMacAddress
                << endl;
    }
    std::cout << "Current RVC Information: " << endl;
    for (int i = 0; i < 16; i++) {
        std::cout << "Period " << (i + 1) << ": duration = "
                << roadsidePeriodInformationArray[i].periodDuration
                << ", tmc = "
                << roadsidePeriodInformationArray[i].transmissionCount << endl;
    }
}

void AribT109MobileStationMacLayer::handleLowerMsg(cMessage* msg) {
    AribT109MacPacket* macPacket = check_and_cast<AribT109MacPacket *>(msg);

    // MAC layer stuff

    long dest = macPacket->getDestAddr();

    EV << "Received frame name = " << macPacket->getName() << ", src = "
              << macPacket->getSrcAddr() << ", dst = " << dest << ", myAddr = "
              << myMacAddr << endl;

//    cModule *sender = getSimulation()->getModule(macPacket->getSrcAddr())->getParentModule();
//    std::cout << "From " << sender->getFullPath() << " (" << macPacket->getSrcAddr() << ") >> " << getParentModule()->getParentModule()->getFullPath() << " (" << myMacAddr << ")" << endl;

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
        EV << "Received a mac packet addressed to me." << endl;

        emit(sigAddressedMacPacketReceived, true);

        // "set the frame reception time to the rxtime parameter"
        // "the frame reception time shall be the value of the one second cycle timer when the preamble arrives at the antenna
        // which is the same as simTime() because the phy delay is 0
        simtime_t rxtime = getOneSecondTimerCycleTimerValueAt(simTime());;

        // send MA-UNITDATA.indication
        AribT109IvcRvcPacket* ivcPacket = check_and_cast<AribT109IvcRvcPacket*>(
                macPacket->decapsulate());

        emit(sigIvcPacketTransmissionTime,
                (simTime() - ivcPacket->getIvcLayerArivalTimeStamp()));

        // IVC layer stuff

        // check whether the received ivc packet is big enough (i.e. at least the header size)
        if (ivcPacket->getByteLength() >= 22) {

            emit(sigSyncStatus, roadsideSynchronizationStatus);
            emitRvcHash();
            emit(sigTimer, getOneSecondTimerCycleTimerValue());
            emitRvcTcHash();

            //////// Procedures of updating all period information (ORT) ////////
            /*
             * First of all, we will update the synchronization information (ORT.SYN)
             * Then, we will synchronize the local clock.
             * Afterwards, we will update the collected roadside period information entries (ORT.ENT) and
             * corresponding to this, we will update the actual roadside period information (OTI)
             * In conclusion, we will update the transmission control information (Inverse ONC).
             */

            //  check whether the ivc control field is valid (p. 65)
            bool controlFieldValid = true;

            // INVALID if:
            // 1) one value in control field is out of range (see ch. 4.4.3.1.2)
            // nothing to be done here
            controlFieldValid &= true;

            // 2) the synchronization information is set to three times or unsynchronized
            if (ivcPacket->getMobileStationSynchronizationStatus()
                    == SYNCHRONIZED_THRICE || !ivcPacket->getSynchronized()) {
                controlFieldValid = false;
            }

            // 3) the value of the rvc period duration in the rvc period information is set to 0
            RoadsidePeriodInformationVector v =
                    ivcPacket->getRoadsidePeriodInformationArray();

            bool allPeriodsInvalid = true;
            for (int i = 0; i < 16; i++) {
                if (v[i].periodDuration == 0) {
//                    controlFieldValid &= false;
                    allPeriodsInvalid &= true;
//                    std::cerr << "The control field was not valid because the duration of period " << (i + 1) << " was 0." << endl;
//                    break;
                } else {
                    allPeriodsInvalid &= false;
                }
            }

//            controlFieldValid &= !allPeriodsInvalid;
//
//            if (allPeriodsInvalid) {
//                controlFieldValid = false;
//                controlFieldValid &= false;
//            } else {
//                controlFieldValid = true;
//                controlFieldValid &= true;
//            }

            controlFieldValid &= true;

            bool updatedOrt = false;
            // update ORT (SYN & ENT) if the ivc control field is valid
            if (controlFieldValid) {
                EV << "The received IR control field is valid." << endl;

                bool updatedLocalSyncStatus = false;
                //// Update the synchronization information ////
                if (ivcPacket->getIsFromBaseStation()) {
                    roadsideSynchronizationStatus = SYNCHRONIZED_DIRECT;
                    updatedLocalSyncStatus = true;
                } else {
                    SynchronizationStatus senderSyncStatus =
                            ivcPacket->getMobileStationSynchronizationStatus();
                    // check whether the sender is not synchronized
                    // this check is just for convenience reasons
                    // in fact, the sync status could just be incemented
                    // because incrementation of unsynchronized gives unsynchronized, too
                    if (roadsideSynchronizationStatus == UNSYNCHRONIZED) {
                        roadsideSynchronizationStatus = incrementStatus(
                                senderSyncStatus);
                        updatedLocalSyncStatus = true;
                    } else {
                        // compare received with stored value
                        if (roadsideSynchronizationStatus > senderSyncStatus) {
                            roadsideSynchronizationStatus = incrementStatus(
                                    senderSyncStatus);
                            updatedLocalSyncStatus = true;
                        }
                    }
                }

                updatedOrt |= updatedLocalSyncStatus;

                if (updatedLocalSyncStatus) {
                    // update synchronization information update elapsed time
                    onRoadsideSynchronizationStatusUpdate();

                    /// synchronize the local clock (p. 67, (5)) ///

                    /*
                     * by compensating the timer in l2
                     * which shall be carried out by
                     * calculating the difference between
                     * the timestamp of the received ivc control field and the rxtime parameter (see above)
                     * and setting the difference to the TC.
                     *
                     * The difference afterwards shall be less than 4 us
                     */

                    simtime_t diff = ivcPacket->getAribTimeStamp() - rxtime;
                    correctedTimerValue = diff.inUnit(SIMTIME_US);

                    if (correctedTimerValue < 0) {
                        // "put back the timer"
                        // subtract value from the timer
                        // parameter is negative
                        correctOneSecondTimerBy(diff);
                    } else if (correctedTimerValue > 0) {
                        // "put forward the timer"
                        // add value to the timer
                        // parameters is positive
                        correctOneSecondTimerBy(diff);
                    } else {
                        // do nothing
                    }
                }

                //// Update the collected rvc period information entries (ORT.ENT) p. 66 (b) ////
//                std::vector<RoadsidePeriodInformation> v = ivcPacket->getRoadsidePeriodInformationArray();

                bool updatedOrtEnt = false;
                // check and compare received entries
//                RoadsidePeriodInformationVector v = ivcPacket->getRoadsidePeriodInformationArray();
                RoadsidePeriodInformation currentNewEntry;
                // for each period compare the information
                for (int i = 0; i < 16; i++) {
                    // for each information with duration != 0
                    currentNewEntry = v[i];
                    if (currentNewEntry.periodDuration != 0) {
                        // check if period number already exists in the collected entry table

                        bool found = false;
                        std::vector<RoadsidePeriodInformationEntry>::iterator itOld =
                                collectedRoadsidePeriodInformationEntries.begin();
                        while (itOld
                                != collectedRoadsidePeriodInformationEntries.end()
                                && !found) {
                            RoadsidePeriodInformationEntry entry =
                                    (RoadsidePeriodInformationEntry) *itOld;
                            if ((i + 1) == entry.periodNumber
                                    && macPacket->getSrcAddr()
                                            == entry.senderMacAddress) {
                                // the current collected entry has the same period number as the currently considered new period information
                                // the current collected entry also has the same mac address as the sender
                                // this means we already received information about this period from the same sender
                                // however, they still can differ
                                found = true;

                                // update the entry
                                if (currentNewEntry.transmissionCount
                                        > entry.periodInformation.transmissionCount) {
                                    // the new transmission count was higher
                                    // use the new information

                                    entry.periodInformation.transmissionCount =
                                            currentNewEntry.transmissionCount;

                                    // NEW FEATURE
                                    // also use the new duration
                                    entry.periodInformation.periodDuration =
                                            currentNewEntry.periodDuration;

                                    entry.storingTimeStamp = simTime();

//                                    std::cout << "Updated RVC Period Information Entry for period "
//                                            << entry.periodNumber << ": " << "duration = "
//                                            << entry.periodInformation.periodDuration << ", tmc = "
//                                            << entry.periodInformation.transmissionCount << " from sender "
//                                            << entry.senderMacAddress << endl;

                                    updatedOrtEnt = true;
                                } else if (currentNewEntry.transmissionCount
                                        == entry.periodInformation.transmissionCount) {
                                    // NEW FEATURE
                                    if (currentNewEntry.periodDuration
                                            >= entry.periodInformation.periodDuration) {
                                        // the transmission count was the same and the duration was larger
                                        // so we choose the new duration

                                        entry.storingTimeStamp = simTime();

//                                        std::cout << "Added new RVC Period Information Entry for period "
//                                                << entry.periodNumber << ": " << "duration = "
//                                                << entry.periodInformation.periodDuration << ", tmc = "
//                                                << entry.periodInformation.transmissionCount << " from sender "
//                                                << entry.senderMacAddress << endl;

                                        updatedOrtEnt = true;
                                    } else {
                                        // the transmission count was the same but the duration was smaller
                                        // so we use the largest duration to be save
                                    }
                                } else {
                                    // the transmission count of the current entry is smaller than the old entry
                                    // do nothing
                                }
                                // we found our matching entry, so we can return from the loop
                            } else {
                                // not the same period number or the same mac address
                            }
                            itOld++;
                        }

                        if (!found) {
                            // just add the information about the current period
                            RoadsidePeriodInformationEntry r =
                                    RoadsidePeriodInformationEntry();
                            r.periodNumber = (i + 1);
                            r.periodInformation = currentNewEntry;
                            r.senderMacAddress = macPacket->getSrcAddr();
                            r.storingTimeStamp = simTime();
                            collectedRoadsidePeriodInformationEntries.push_back(
                                    r);

//                            std::cout << "Added new RVC Period Information Entry for period " << r.periodNumber << ": "
//                                    << "duration = " << r.periodInformation.periodDuration << ", tmc = "
//                                    << r.periodInformation.transmissionCount << " from sender " << r.senderMacAddress
//                                    << endl;

                            updatedOrtEnt = true;
                        }
                    } else {
                        // the information about the current period was not usable because it's duration was 0
                        debugEV
                                       << "The information about the current period was not usable because the duration was 0."
                                       << endl;
                    }
                }

                updatedOrt |= updatedOrtEnt;

                /// update also the roadside period information (OTI) (p. 67 (6)) ///
                if (updatedOrt) {
                    // the best fitting entries out of all roadside period information have to be taken and used for the actual roadside period information table

                    for (int i = 0; i < 16; i++) {
                        // compare current transmission count to all stored entries with the same period number
                        // choose the largest transmission count of all these
                        TransmissionCount largestTransmissionCount = NO_TRANSFER;
                        RoadsidePeriodInformationEntry largestTransmissionCountEntry =
                                RoadsidePeriodInformationEntry();
                        largestTransmissionCountEntry.periodInformation.transmissionCount =
                                NO_TRANSFER;
                        bool set = false;
                        bool found = false;
                        // search for max transmission count among all entries with the same period number as the currently sonsidered period
                        for (unsigned int j = 0;
                                j
                                        < collectedRoadsidePeriodInformationEntries.size();
                                j++) {
                            RoadsidePeriodInformationEntry current =
                                    collectedRoadsidePeriodInformationEntries[j];
                            if ((i + 1) == current.periodNumber) {
                                // the current collected entry has the same period number as the currently considered new period information
                                found = true;
                                if (set) {
                                    // the first match is set to the largest transmission count
                                    // we can compare now
                                    if (current.periodInformation.transmissionCount
                                            > largestTransmissionCount) {
                                        // we found a larger transmission count
                                        // simply update the current largest
                                        largestTransmissionCount =
                                                current.periodInformation.transmissionCount;
                                        largestTransmissionCountEntry = current;
                                    } else if (current.periodInformation.transmissionCount
                                            == largestTransmissionCount) {
                                        // the transmission count was the same
                                        // compare durations
                                        if (current.periodInformation.periodDuration
                                                > largestTransmissionCountEntry.periodInformation.periodDuration) {
                                            // the duration was bigger, update
                                            largestTransmissionCountEntry =
                                                    current;
                                        }
                                    } else {
                                        // the transmission count was smaller
                                    }
                                } else {
                                    // we do a minimum search here
                                    // the largest transmission count will be set to the first transmission count with the correct period number
                                    largestTransmissionCount =
                                            current.periodInformation.transmissionCount;
                                    largestTransmissionCountEntry = current;
                                    set = true;
                                }
                            } else {
                                // the current period number does not match the currently considered period
                            }
                        } // end inner for

                        if (found) {
                            // we found the entry with the largest transmission count
                            // now update the period information
//                            if (sizeof(largestTransmissionCountEntry) >= sizeof(largestTransmissionCount))
//                            {
                            if (largestTransmissionCountEntry.periodInformation.transmissionCount
                                    != largestTransmissionCount) {
                                exceptionMessage.clear();
                                exceptionMessage
                                        << "We ran into a consistency problem while searching for the largest transmission count!"
                                        << "Largest transmission count: "
                                        << largestTransmissionCount
                                        << ", largest transmission count from entry: "
                                        << largestTransmissionCountEntry.periodInformation.transmissionCount
                                        << ".";
                                throw cRuntimeError(
                                        exceptionMessage.str().c_str());
                            }
//                            }
//                            else
//                            {
                            // the largest entry is still NULL which means there was no larger entry
//                            }

                            if (largestTransmissionCount >= TRANSFER_ONCE) {
                                roadsidePeriodInformationArray[i].transmissionCount =
                                        decrementTransmissionCount(
                                                largestTransmissionCount);
                                roadsidePeriodInformationArray[i].periodDuration =
                                        largestTransmissionCountEntry.periodInformation.periodDuration;
//
//                                std::cout << "Updated RVC Period Information for period " << i << ": " << " duration = "
//                                        << roadsidePeriodInformationArray[i].periodDuration << ", tmc = "
//                                        << roadsidePeriodInformationArray[i].transmissionCount << endl;
                            } else if (largestTransmissionCount
                                    == NO_TRANSFER) {
                                roadsidePeriodInformationArray[i].transmissionCount =
                                        NO_TRANSFER;
                                roadsidePeriodInformationArray[i].periodDuration =
                                        0;

//                                std::cout << "Updated RVC Period Information for period " << i << ": " << " duration = "
//                                        << "0" << ", tmc = " << NO_TRANSFER << endl;
                            } else {
                                // this case must not be reached
                                cRuntimeError(
                                        "We ran into an illegal state while checking the transfer count.");
                            }
                        } else {
                            // there was no entry for the current period
                            roadsidePeriodInformationArray[i].transmissionCount =
                                    NO_TRANSFER;
                            roadsidePeriodInformationArray[i].periodDuration =
                                    0;

//                            std::cout << "Updated RVC Period Information for period " << i << ": " << " duration = "
//                                    << "0" << ", tmc = " << NO_TRANSFER << endl;
                        }

                        // guard time at the beginning of the rvc period is already handled while sending a roadside period event
                    }

                    /// update also the transmission control information (ONC) (p. 67 (7)) ///

                    for (int i = 0; i < 16; i++) {
/* the following is defined in the standard but I am not using this...

                    // search for the largest period duration in the entries which match the current period
                    int largestDuration = NO_TRANSFER;
                    RoadsidePeriodInformationEntry largestDurationEntry;
                    bool found = false;
                    while (std::vector<RoadsidePeriodInformationEntry>::iterator it =
                            collectedRoadsidePeriodInformationEntries.begin();
                            it
                                    != collectedRoadsidePeriodInformationEntries.end();
                            ) {
                        RoadsidePeriodInformationEntry current =
                                (RoadsidePeriodInformationEntry) *it;
                        if ((i + 1) == current.periodNumber) {
                            found = true;
                            if (current.periodInformation.periodDuration
                                    > largestDuration) {
                                // we found a larger duration
                                // simply update the current largest
                                largestDuration =
                                        current.periodInformation.periodDuration;
                                largestDurationEntry = current;
                            } else {
                                // the duration was smaller
                            }
                        } else {
                            // the current period number does not match the currently considered period
                            break;
                        }
                        it++;
                    }

		    if (found) {
                        // we found the entry with the largest duration
                        // now update the transmission information

                        int PPDU_DURATION = 0;
                        // inhibition start (NST)
                        int nst = (i + 1)
                                * TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US
                                - GUARD_TIME_ARIB.dbl() - PPDU_DURATION;
                        if (nst < 0) {
                            nst += 6250;
                        }

                        // inhibition duration (NVP)
                        int nvp =
                                PPDU_DURATION
                                        + 3
                                                * collectedRoadsidePeriodInformationEntries[i].periodInformation.periodDuration
                                        + 2 * GUARD_TIME_ARIB.dbl();
                        if (nvp > 6250) {
                            nvp = 6250;
                        }

                        // ...
                    } else {
                        // there was no entry for the current period
                    }
*/

/* instead, I am using the following for updating the transmission control */

                        // getting the current RVC period duration
                        int roadsidePeriodDuration =
                                roadsidePeriodInformationArray[i].periodDuration;
                        // determine the transmission period begin as the end of the current RVC period (this already includes guard time at the end)
                        transmissionControlArray[i].transmissionPeriodBegin = 0
                                + roadsidePeriodDuration;
                        // determine the duration of the current transmission period duration by subtracting the RVC period duration and another guard time at the end of the RVC period
                        transmissionControlArray[i].transmissionPeriodDuration =
                                TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US
                                        - roadsidePeriodDuration
                                        - GUARD_TIME_ARIB.inUnit(SIMTIME_US);
                    }
                }

                ///// END OF ALL IVC UPDATING STUFF /////

                emit(sigSyncStatus, roadsideSynchronizationStatus);
                emitRvcHash();
                emit(sigTimer, getOneSecondTimerCycleTimerValue());
                emitRvcTcHash();

//                printMaintainedRvcInformation();
            } else {
                EV
                          << "The IVC control field was not valid. Still the data is passed to the upper layer."
                          << endl;
                emit(sigInvalidControlFieldReceived, true);
            }

            // pass received frame to upper layer
            cPacket* dataPkt = check_and_cast<cPacket *>(
                    ivcPacket->decapsulate());

            dataPkt->setControlInfo(new PhyToMacControlInfo(res));

            sendUp(dataPkt);
        } else {
            EV << "Received IVC packet was to small, deleting..." << endl;
            emit(sigIvcPacketDiscarded, true);
            // statsDeletedReceivedFrames++;
            delete ivcPacket;
        }
    } else {
        EV << "Packet not for me, deleting..." << endl;
        emit(sigReceivedMacPacketDiscarded, true);
        delete macPacket;
    }
    delete msg;
}

void AribT109MobileStationMacLayer::handleSelfMsg(cMessage* msg) {
    EV << "Received message " << msg->getFullName() << endl;

    if (msg == resetHundredMsCycleTimerEvent) {
        resetHundredMsTimer();

        cancelEvent(roadsidePeriodBeginEvent);

        // determine the beginning of the next RVC period by using a maximal period duration and guard time at the beginning
        // --> guard still has to be added at the end of a RVC period
        // is done when receiving new information
        simtime_t nextRoadsidePeriodBegin = simTime() + GUARD_TIME_ARIB
                + SimTime(TOTAL_PERIOD_DURATION_UNTIL_NEXT_RVC_PERIOD_US,
                        SIMTIME_US);

        scheduleAt(nextRoadsidePeriodBegin, roadsidePeriodBeginEvent);
        EV << "Scheduled event for next roadside period begin at "
                  << nextRoadsidePeriodBegin << endl;

        //// Update the rvc period information entries (ORT.ENT)

        /*
         * Update each entry as following:
         *
         * If the elapsed time exceeds the threshold and the transmission count is 1 or larger,
         * the transmission count shall be decremented by 1 and the elapsed time shall be set to 0
         *
         * If the transmission count is 0, the entry shall be deleted
         */

        // removes all entries with transmission count of 0
        // which leaves all entries with a transmission count of 1 at least
        collectedRoadsidePeriodInformationEntries.erase(
                std::remove_if(
                        collectedRoadsidePeriodInformationEntries.begin(),
                        collectedRoadsidePeriodInformationEntries.end(),
                        isNoTransfer),
                collectedRoadsidePeriodInformationEntries.end());

        for (unsigned int i = 0;
                i < collectedRoadsidePeriodInformationEntries.size(); i++) {
            RoadsidePeriodInformationEntry currentEntry =
                    (RoadsidePeriodInformationEntry) collectedRoadsidePeriodInformationEntries[i];
            simtime_t elapsedTime = simTime() - currentEntry.storingTimeStamp;
            if (elapsedTime > roadsidePeriodInformationValidTime) {
                // decrement entry
                currentEntry.periodInformation.transmissionCount =
                        decrementTransmissionCount(
                                currentEntry.periodInformation.transmissionCount);
            }
        }
    } else if (msg == transmissionPeriodBeginEvent) {
        // the channel is judged IDLE by the virtual carrier sense function (TDMA)
        // But we still have to do CSMA / CA

        channelVirtuallyIdle = true;

        // we came to the point in time where we are allowed to transmit packets from our queue
        // we schedule an event for sending an ivc packet
        // Therefore, we use the current time because we are now allowed to transmit a packet
        // But we still have to do CSMA / CA
        cancelEvent(sendIvcPacketEvent);
        scheduleAt(simTime(), sendIvcPacketEvent);
        EV << "Scheduled event for transmission of a mac packet." << endl;
    } else if (msg == sendIvcPacketEvent) {
        if (channelVirtuallyIdle) {
            // We came to the point in time where we can transmit a packet (the virtual carrier sense function i.e. TDMA judges the channel as IDLE)
            // but we still have to do CSMA / CA for the channel access

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
                simtime_t currentTransmissionPeriodDuration =
                        SimTime(
                                transmissionControlArray[(currentPeriodIndex - 1)].transmissionPeriodDuration,
                                SIMTIME_US);
                EV << "The current transmission period duration is "
                          << currentTransmissionPeriodDuration << endl;

                simtime_t randomWaitingPeriod = generateRandomWaitingPeriod();

                simtime_t completePacketDuration = DISTRIBUTED_SPACE
                        + randomWaitingPeriod + currentFrameDuration;

                // fix to check whether enough time is left int the slot for the packet to be sent!
		// TODO consider in future rework
                bool we_can_send = (simTime() + completePacketDuration < vehicle_period_end) ? true : false;

                // check whether current packet can be send in current transmission period
                // Therefore, the medium has to be idle at least for the time of the distributed space + a random time + the current frame duration
                if (completePacketDuration
                        <= currentTransmissionPeriodDuration && we_can_send) {
                    // packet is small enough to be send in the current transmission period
                    // actually "send" the packet
                    EV
                              << "The packet to transmit is small enough to be sent in the current transmission period."
                              << endl;

					// emit TDMA waiting time
                    emit(sigIvcPacketQueueWaitingTime,
                            simTime() - ivcPacket->getIvcLayerArivalTimeStamp());

                    /////////////////////////////// CSMA/CA ///////////////////////////////

                    if (!onGoingTransmission && channelPhysicallyIdle) {
                        // the channel is currently idle so there is a good chance that it is still idle after the period of distributed space and the random waiting period
                        // hence, we schedule a packet transmission after time of distributed space and random waiting period
                        simtime_t minimumIdleTime = DISTRIBUTED_SPACE
                                + randomWaitingPeriod;

                        // add time stamp for eval the csma delay
                        ivcPacket->setCsmaStartTimeStamp(simTime());

                        /**
                         * To get better performance and more accurate results, I wanted to switch the radio just before the packet is scheduled.
                         * This approach is bad because if the channel gets busy in the time it takes to switch to TX,
                         * the channel was not idle for the whole time of distributed space and random waiting period
                         * and the radio cannot notice that because it is switching or has already switched to TX.
                         *
                         * So simply wait for the switching until the sendMacPacketEvent.
                         */

                        // we schedule the actual packet
                        cancelEvent(sendMacPacketEvent);
                        scheduleAt(simTime() + minimumIdleTime,
                                sendMacPacketEvent);

                        // schedule an event for sending the next ivc packet
                        // if there is enough time left
                        // consider at least the time passed by sending the current packet
                        // the rest is handled by the transmission end event
                        if (currentTransmissionPeriodDuration
                                - completePacketDuration
                                > getFrameDuration(288)) {
                            // there is enough time left for another packet
                            scheduleAt(simTime() + completePacketDuration,
                                    sendIvcPacketEvent);
                        } else {
                            // the next packet has to wait until the next transmission period
                        }
                    } else {
                        // the channel is currently busy
                        // so we can't schedule a SendMacPacketEvent right now
                        // the channel has to be idle at least for the time of the distributed space
                        // but we can check again after a small amount of time

                        // do nothing because a ivc packet will be scheduled in case we are idle
                    }
                } else {
                    // packet was to big to be sent in this transmission period
                    // wait until next transmission period (is done implicitly by the transmission periods)
                    EV << "The current ivc packet (" << ivcPacket->getFullName()
                              << ", id=" << ivcPacket->getId()
                              << ") is to big to be transmitted in the current transmission period"
                              << endl;
                }
            } else {
                // queue is empty
                // do nothing
                EV
                          << "Received event for sending a IVC packet but there is no data in the queue."
                          << endl;
            }
        } else {
            // the channel is not virtually idle anymore
            // do nothing
        }
    } else if (msg == sendMacPacketEvent) {
        // the channel was physically idle
        // now we waited for the the distributed time, a random waiting period and a frame duration
        // now we have to check, whether the channel is idle
        // if this ist the case, we are allowed to transmit
        // otherwise we have to start over

        EV << "Received an event for sending a MAC packet." << endl;

        // fix to send only if pkg fits in to remaining sending time slot of the vehicle
	// TODO consider in future rework
        simtime_t currFrameDuration = getFrameDuration(
                                (ivcPacketQueue->front()->getBitLength() + 288));
        simtime_t pkg_duration = RADIODELAY_11P + currFrameDuration + SIFS_ARIB;
        bool we_can_send = ((simTime() + pkg_duration) < vehicle_period_end)?true:false;

        // check the current channel state
        if (!onGoingTransmission && channelPhysicallyIdle && we_can_send) {
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

            AribT109IvcRvcPacket* ivcPacket = check_and_cast<
                    AribT109IvcRvcPacket *>(ivcPacketQueue->pop());

            // csma waiting time
            emit(sigCsmaDelay,
                    simTime() - ivcPacket->getCsmaStartTimeStamp());
            emit(sigCurrentIvcPacketsInQueue, ivcPacketQueue->getLength());

            AribT109TransmissionControlMessage* ivcControlInfo = check_and_cast<
                    AribT109TransmissionControlMessage *>(
                    ivcPacket->getControlInfo());

            AribT109MacPacket *macPacket = new AribT109MacPacket(
                    ivcPacket->getName(), ivcPacket->getKind());

            macPacket->setMacLayerTimeStamp(simTime());

            // set length of MAC control field (24B) + LLC control field (8B) + FCS (4B)
            macPacket->setByteLength(36);

            // set destination to value from upper layer
            macPacket->setDestAddr(ivcControlInfo->getLinkAddress());
            macPacket->setSrcAddr(myMacAddr);

            transmissionCounter++;
            macPacket->setTransmissionCount(transmissionCounter);

            macPacket->setWirelessCallNumber(
                    "Some string as wireless call number");

            // set value of 1s timer when the preamble arrives at the antenna
            // corresponding to the delay from below
            // correct according standards document
            ivcPacket->setAribTimeStamp(
                    getOneSecondTimerCycleTimerValueAt(
                            simTime() + RADIODELAY_11P + SIFS_ARIB));

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
            // Just consider the case that we already are in TX but still wait RADIODELAY_11P for sending later on
            //
            // see comment in case of receiving a sendIvcPacketEvent
            phy->setRadioState(Radio::TX);

            attachSignal(macPacket, simTime() + RADIODELAY_11P, carrierFreq,
                    bitrate, txPower_mW);

            EV << "Attached signal to the mac packet." << endl;

            sendDelayed(macPacket, RADIODELAY_11P, lowerLayerOut);

            simtime_t distri = simTime() + RADIODELAY_11P - lastMacTime;
            emit(sigDistributedSpace, distri.inUnit(SIMTIME_US));
            lastMacTime = simTime();

            emit(sigMacPacketSent, macPacket);

            EV << "Actually sent the packet to the PHY!" << endl;
        } else {
            // the channel is busy
            // so we have start over
            // the channel has to be idle at least for the time of the distributed space
            // but we can check again after a small amount of time

            // do nothing
            // we do not have to put the current ivc packet back into the queue because we just got the front element of the queue
            // but did not deleted it from the queue
        }

    } else if (msg == roadsideSynchronizationStatusValidTimeElapsedEvent) {
        // update the synchronization status & reset timer
        updateRoadsideSynchronizationStatus();
    } else {
        AribT109AbstractMacLayer::handleSelfMsg(msg);
    }
}

void AribT109MobileStationMacLayer::handleUpperMsg(cMessage* msg) {
    cPacket* dataPkt = check_and_cast<cPacket*>(msg);

    EV << "Received a message from upper layer " << dataPkt->getName() << endl;

    emit(sigDataPacketToSend, dataPkt);

    // check whether the packet is small enough to be sent (see standard)
    if (dataPkt->getByteLength() <= 1500) {

        EV << "Data size is small enough to be sent." << endl;

        // extract SequenceNumber, ControlInformation, LinkAddress from data (APDU)
        int packetNumber = dataPkt->getId();

        // do IVC packet stuff: generate IPDU

        AribT109IvcRvcPacket* ivcPacket = new AribT109IvcRvcPacket(
                dataPkt->getName(), dataPkt->getKind());

        ivcPacket->setIvcLayerArivalTimeStamp(simTime());

        // set length of IR control field (header)
        ivcPacket->setByteLength(22);

        ivcPacket->setIsFromBaseStation(false);
        ivcPacket->setSynchronized(
                roadsideSynchronizationStatus == UNSYNCHRONIZED ? false : true);
        ivcPacket->setMobileStationSynchronizationStatus(
                roadsideSynchronizationStatus);

        // set RVC period information by getting the registered values
        RoadsidePeriodInformationVector *v =
                new RoadsidePeriodInformationVector();

        for (int i = 0; i < 16; i++) {
            if (roadsidePeriodInformationArray[i].transmissionCount
                    > NO_TRANSFER) {
                v->push_back(roadsidePeriodInformationArray[i]);
            } else {
                RoadsidePeriodInformation inf = RoadsidePeriodInformation();
                inf.periodDuration = 0;
                inf.transmissionCount = NO_TRANSFER;
                v->push_back(inf);
            }
        }
        ivcPacket->setRoadsidePeriodInformationArray(*v);

        // give data (packet itself) & [control info:] LinkAddress (destination address), SequenceNumber (see app layer 4.5.2.1.4) and ControlInformation (data rate) to mac layer

        ivcPacket->encapsulate(dataPkt);
        AribT109TransmissionControlMessage *ivcControl =
                new AribT109TransmissionControlMessage();

        ivcControl->setLinkAddress(LAddress::L2BROADCAST());
        ivcControl->setPacketNumber(packetNumber);

        ivcPacket->setControlInfo(ivcControl);

        emit(sigIvcDataBits, ivcPacket->getBitLength());

        // ready to send from perspective of IVC layer

        // LLC sub-layer is left out --> MA-UNITDATA.request SequenceNumber (see above), LinkAddress (see above), data & control field (see above), ControlInformation (see above)

        // MAC layer

        // check whether the transmission time for the MSDU (IVC packet) with length of MAC control field (24B) + LLC control field (8B) + FCS (4B) exceeds 300 us
        // if this is the case, we don't need to enqueue the packet

        if (AribT109AbstractMacLayer::getFrameDuration(
                ivcPacket->getBitLength() + 288) <= SimTime(300, SIMTIME_US)) {
            // store MSDU (IVC packet)
            ivcPacketQueue->insert(ivcPacket);
            emit(sigCurrentIvcPacketsInQueue, ivcPacketQueue->getLength());
            EV << "Added message " << ivcPacket->getName() << " to queue"
                      << endl;
        } else {
            // discard message
	    delete ivcPacket;
	    delete v;
	    delete ivcControl;
            delete msg;
            std::cerr << "Data was to big, Message was deleted." << endl;
            emit(sigDataPacketDiscarded, true);
        }
    } else {
        delete msg;
        std::cerr << "Data was to big, Message was deleted." << endl;
        emit(sigDataPacketDiscarded, true);
    }
}

void AribT109MobileStationMacLayer::channelBusy() {
    // the channel turned busy because we hear someone, who is not us, sending

    channelPhysicallyIdle = false;

    EV << "Channel turned busy because of an external sender" << endl;
    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        lastBusy = simTime();
    }

    // cancel ongoing random waiting period
    cancelEvent(sendMacPacketEvent);
    emit(sigChannelBusy, true);
}

void AribT109MobileStationMacLayer::channelBusySelf() {
    // we started a transmission

    onGoingTransmission = true;
    EV << "Transmission started" << endl;

    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        lastBusy = simTime();
    }

    cancelEvent(sendMacPacketEvent);
    emit(sigChannelBusy, true);
}

void AribT109MobileStationMacLayer::channelIdle() {
    // the channel turned idle after someone, who was not us, has sent something

    channelPhysicallyIdle = true;
    EV << "Channel turned idle after external transmission" << endl;

    if (!onGoingTransmission) {
        // channel is idle now

        if (simTime() >= getSimulation()->getWarmupPeriod()) {
            lastIdle = simTime();
            statsTotalBusyTime += simTime() - lastBusy;
        }

        // check whether the channel is virtual idle
        // if this is the case. we can schedule an ivc packet
        if (channelVirtuallyIdle) {
            cancelEvent(sendIvcPacketEvent);
            scheduleAt(simTime(), sendIvcPacketEvent);
        } else {
            cancelEvent(sendIvcPacketEvent);
        }
    }
}

void AribT109MobileStationMacLayer::channelIdleSelf() {
    // our transmission is over

    onGoingTransmission = false;
    EV << "Transmission is over" << endl;

    if (channelPhysicallyIdle) {
        // channel is idle now

        if (simTime() >= getSimulation()->getWarmupPeriod()) {
            lastIdle = simTime();
            statsTotalBusyTime += simTime() - lastBusy;
        }

        // check whether the channel is virtual idle
        // if this is the case. we can schedule an ivc packet
        if (channelVirtuallyIdle) {
            cancelEvent(sendIvcPacketEvent);
            scheduleAt(simTime(), sendIvcPacketEvent);
        } else {
            cancelEvent(sendIvcPacketEvent);
        }
    }
}

/**
 * Modifies the 1s cycle timer with the correctedTimerValue
 *
 * If you want to increase the timer, use a positive value.
 * If you want to decrease the timer, use a negative value.
 *
 * e.g. if the timer is 50 now, it has to be 60 afterwards if the corrected value is 10
 * startTime = startTime - 10
 * --> subtract the value
 *
 * e.g. if the timer is 50 now, it has to be 40 afterwards if the corrected value was -10
 * startTime = startTime + 10 = startTime - (-10)
 * --> subtract the value
 */
void AribT109MobileStationMacLayer::correctOneSecondTimerBy(
        const simtime_t correctedTimerValue) {

    if (correctedTimerValue > SimTime(1, SIMTIME_MS)) {
        debugEV << "Want to correct the timer by " << correctedTimerValue
                       << "s (" << correctedTimerValue.inUnit(SIMTIME_MS)
                       << "ms)" << endl;
    }

    emit(sigCorrectTimerByValue, correctedTimerValue);

    debugEV << "Timer value: " << getOneSecondTimerCycleTimerValue();

    oneSecondCycleTimerStart -= correctedTimerValue;

    debugEV << " --> " << getOneSecondTimerCycleTimerValue() << endl;

    // we still have to modify all events
    onTimerValueModified(correctedTimerValue);
}

/**
 * - is used for everything to the value has to be either positive for subtracting or negative for adding
 */
void AribT109MobileStationMacLayer::onTimerValueModified(
        const simtime_t correctedTimerValue) {
    // we still have to modify all events

    simtime_t newTime = SIMTIME_ZERO;
    simtime_t now = simTime();
    debugEV << "Now: " << now << endl;

    // reset one 1s cycle timer event
    newTime = resetOneSecondCycleTimerEvent->getArrivalTime()
            - correctedTimerValue;
    debugEV << "1s timer: " << resetOneSecondCycleTimerEvent->getArrivalTime()
                   << " --> " << newTime << endl;
    cancelEvent(resetOneSecondCycleTimerEvent);
    scheduleAt(newTime, resetOneSecondCycleTimerEvent);

    // reset 100ms cycle timer event
    newTime = resetHundredMsCycleTimerEvent->getArrivalTime()
            - correctedTimerValue;
    if (newTime < now) {
        debugEV << "100ms timer: "
                       << resetHundredMsCycleTimerEvent->getArrivalTime()
                       << " --> " << newTime << endl;
        simtime_t alreadyPassedTime = now - newTime;
        debugEV << "Already passed: " << alreadyPassedTime << endl;
        newTime += SimTime(100, SIMTIME_MS);
        if (newTime - now > SimTime(100, SIMTIME_MS)) {
            newTime -= SimTime(100, SIMTIME_MS);
            std::cerr << "Something bad happened while correcting the timer."
                    << endl;
        }
    }
    debugEV << "100ms timer: "
                   << resetHundredMsCycleTimerEvent->getArrivalTime() << " --> "
                   << newTime << endl;
    cancelEvent(resetHundredMsCycleTimerEvent);
    scheduleAt(newTime, resetHundredMsCycleTimerEvent);

    // roadside synchronization status valid time elapsed event
    newTime =
            roadsideSynchronizationStatusValidTimeElapsedEvent->getArrivalTime()
                    - correctedTimerValue;
    debugEV << "syncStatus: "
                   << roadsideSynchronizationStatusValidTimeElapsedEvent->getArrivalTime()
                   << " --> " << newTime << endl;
    cancelEvent(roadsideSynchronizationStatusValidTimeElapsedEvent);
    scheduleAt(newTime, roadsideSynchronizationStatusValidTimeElapsedEvent);
}

void AribT109MobileStationMacLayer::updateRoadsideSynchronizationStatus() {
    if (roadsideSynchronizationStatus == SYNCHRONIZED_THRICE) {
        // delete all entries in ORT
        collectedRoadsidePeriodInformationEntries.clear();
    }
    roadsideSynchronizationStatus = incrementStatus(
            roadsideSynchronizationStatus);
    onRoadsideSynchronizationStatusUpdate();
}
void AribT109MobileStationMacLayer::onRoadsideSynchronizationStatusUpdate() {
    roadsideSynchronizationTimeStamp = simTime();
    cancelEvent(roadsideSynchronizationStatusValidTimeElapsedEvent);
    scheduleAt((simTime() + roadsidePeriodInformationValidTime),
            roadsideSynchronizationStatusValidTimeElapsedEvent);
    statsRoadsideSynchronizationStatusElapseTimeResets++;
}
