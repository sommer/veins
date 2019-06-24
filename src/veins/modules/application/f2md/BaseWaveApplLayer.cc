//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#include <veins/modules/application/f2md/BaseWaveApplLayer.h>

using namespace veins;

void BaseWaveApplLayer::initialize(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage == 0) {

        // initialize pointers to other modules
        if (FindModule<TraCIMobility*>::findSubModule(getParentModule())) {
            mobility = TraCIMobilityAccess().get(getParentModule());
            traci = mobility->getCommandInterface();
            traciVehicle = mobility->getVehicleCommandInterface();
        } else {
            traci = nullptr;
            mobility = nullptr;
            traciVehicle = nullptr;
        }

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        mac = FindModule<DemoBaseApplLayerToMac1609_4Interface*>::findSubModule(
                getParentModule());
        assert(mac);

        // read parameters
        headerLength = par("headerLength");
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits");
        beaconUserPriority = par("beaconUserPriority");
        beaconInterval = par("beaconInterval");

        dataLengthBits = par("dataLengthBits");
        dataOnSch = par("dataOnSch").boolValue();
        dataUserPriority = par("dataUserPriority");

        wsaInterval = par("wsaInterval").doubleValue();
        currentOfferedServiceId = -1;

        isParked = false;

        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        findHost()->subscribe(TraCIMobility::parkingStateChangedSignal, this);

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendWSAEvt = new cMessage("wsa evt", SEND_WSA_EVT);

        generatedBSMs = 0;
        generatedWSAs = 0;
        generatedWSMs = 0;
        receivedBSMs = 0;
        receivedWSAs = 0;
        receivedWSMs = 0;


        // F2MD
        curPositionConfidenceOrig = Coord(0, 0, 0);
        curSpeedConfidenceOrig = Coord(0, 0, 0);
        curHeadingConfidenceOrig = Coord(0, 0, 0);
        curAccelConfidenceOrig = Coord(0, 0, 0);

        curPositionConfidence = Coord(0, 0, 0);
        curSpeedConfidence = Coord(0, 0, 0);
        curHeadingConfidence = Coord(0, 0, 0);
        curAccelConfidence = Coord(0, 0, 0);
        myWidth = 0;
        myLength = 0;
        curHeading = Coord(0, 0, 0);
        curAccel = Coord(0, 0, 0);
        myMdType = mbTypes::Genuine;
        myAttackType = attackTypes::Attacks::Genuine;
        //attackBsm.setSenderAddress(0);
        //nextAttackBsm.setSenderAddress(0);
    } else if (stage == 1) {

        // store MAC address for quick access
        myId = mac->getMACAddress();

        // simulate asynchronous channel access

        if (dataOnSch == true && !mac->isChannelSwitchingActive()) {
            dataOnSch = false;
            EV_ERROR
                            << "App wants to send data on SCH but MAC doesn't use any SCH. Sending all data on CCH"
                            << std::endl;
        }
        simtime_t firstBeacon = simTime();

        if (par("avoidBeaconSynchronization").boolValue() == true) {

            simtime_t randomOffset = dblrand() * beaconInterval;
            firstBeacon = simTime() + randomOffset;

            if (mac->isChannelSwitchingActive() == true) {
                if (beaconInterval.raw()
                        % (mac->getSwitchingInterval().raw() * 2)) {
                    EV_ERROR << "The beacon interval (" << beaconInterval
                                    << ") is smaller than or not a multiple of  one synchronization interval ("
                                    << 2 * mac->getSwitchingInterval()
                                    << "). This means that beacons are generated during SCH intervals"
                                    << std::endl;
                }
                firstBeacon = computeAsynchronousSendingTime(beaconInterval,
                        ChannelType::control);
            }

            if (sendBeacons) {
                scheduleAt(firstBeacon, sendBeaconEvt);
            }
        }
    }
}

simtime_t BaseWaveApplLayer::computeAsynchronousSendingTime(simtime_t interval,
        ChannelType chan) {

    /*
     * avoid that periodic messages for one channel type are scheduled in the other channel interval
     * when alternate access is enabled in the MAC
     */

    simtime_t randomOffset = dblrand() * beaconInterval;
    simtime_t firstEvent;
    simtime_t switchingInterval = mac->getSwitchingInterval(); // usually 0.050s
    simtime_t nextCCH;

    /*
     * start event earliest in next CCH (or SCH) interval. For alignment, first find the next CCH interval
     * To find out next CCH, go back to start of current interval and add two or one intervals
     * depending on type of current interval
     */

    if (mac->isCurrentChannelCCH()) {
        nextCCH = simTime()
                - SimTime().setRaw(simTime().raw() % switchingInterval.raw())
                + switchingInterval * 2;
    } else {
        nextCCH = simTime()
                - SimTime().setRaw(simTime().raw() % switchingInterval.raw())
                + switchingInterval;
    }

    firstEvent = nextCCH + randomOffset;

    // check if firstEvent lies within the correct interval and, if not, move to previous interval

    if (firstEvent.raw() % (2 * switchingInterval.raw())
            > switchingInterval.raw()) {
        // firstEvent is within a sch interval
        if (chan == ChannelType::control)
            firstEvent -= switchingInterval;
    } else {
        // firstEvent is within a cch interval, so adjust for SCH messages
        if (chan == ChannelType::service)
            firstEvent += switchingInterval;
    }

    return firstEvent;
}

//F2MD
void BaseWaveApplLayer::addMyBsm(BasicSafetyMessage bsm) {
    if (myBsmNum < MYBSM_SIZE) {
        myBsmNum++;
    }
    for (int var = myBsmNum - 1; var > 0; --var) {
        myBsm[var] = myBsm[var - 1];
    }

    myBsm[0] = bsm;
}

void BaseWaveApplLayer::populateWSM(BaseFrame1609_4* wsm,
        LAddress::L2Type rcvId, int serial) {
    wsm->setRecipientAddress(rcvId);
    wsm->setBitLength(headerLength);

    if (BasicSafetyMessage* bsm = dynamic_cast<BasicSafetyMessage*>(wsm)) {
        //F2MD
        bsm->setSenderPseudonym(myPseudonym);
        bsm->setSenderMbType(myMdType);
        bsm->setSenderAttackType(myAttackType);
        // Genuine
        bsm->setSenderPos(curPosition);
        bsm->setSenderPosConfidence(curPositionConfidence);

        std::pair<double, double> currLonLat = traci->getLonLat(curPosition);
        bsm->setSenderGpsCoordinates(
                Coord(currLonLat.first, currLonLat.second));

        bsm->setSenderSpeed(curSpeed);
        bsm->setSenderSpeedConfidence(curSpeedConfidence);

        bsm->setSenderHeading(curHeading);
        bsm->setSenderHeadingConfidence(curHeadingConfidence);

        bsm->setSenderAccel(curAccel);
        bsm->setSenderAccelConfidence(curAccelConfidence);

        bsm->setSenderWidth(myWidth);
        bsm->setSenderLength(myLength);

        bsm->setSenderRealId(myId);
        addMyBsm(*bsm);
        // Genuine

        if (myMdType == mbTypes::LocalAttacker) {
            if (attackBsm.getSenderPseudonym() != 0) {
                bsm->setSenderPseudonym(attackBsm.getSenderPseudonym());

                bsm->setSenderPos(attackBsm.getSenderPos());
                bsm->setSenderPosConfidence(attackBsm.getSenderPosConfidence());

                std::pair<double, double> currLonLat = traci->getLonLat(
                        attackBsm.getSenderPos());
                bsm->setSenderGpsCoordinates(
                        Coord(currLonLat.first, currLonLat.second));

                bsm->setSenderSpeed(attackBsm.getSenderSpeed());
                bsm->setSenderSpeedConfidence(
                        attackBsm.getSenderSpeedConfidence());

                bsm->setSenderHeading(attackBsm.getSenderHeading());
                bsm->setSenderHeadingConfidence(
                        attackBsm.getSenderHeadingConfidence());

                bsm->setSenderAccel(attackBsm.getSenderAccel());
                bsm->setSenderAccelConfidence(
                        attackBsm.getSenderAccelConfidence());

                bsm->setSenderWidth(attackBsm.getSenderWidth());
                bsm->setSenderLength(attackBsm.getSenderLength());
            } else {
                bsm->setSenderPos(curPosition);
                bsm->setSenderPosConfidence(curPositionConfidence);

                std::pair<double, double> currLonLat = traci->getLonLat(
                        curPosition);
                bsm->setSenderGpsCoordinates(
                        Coord(currLonLat.first, currLonLat.second));

                bsm->setSenderSpeed(curSpeed);
                bsm->setSenderSpeedConfidence(curSpeedConfidence);

                bsm->setSenderHeading(curHeading);
                bsm->setSenderHeadingConfidence(curHeadingConfidence);

                bsm->setSenderAccel(curAccel);
                bsm->setSenderAccelConfidence(curAccelConfidence);

                bsm->setSenderWidth(myWidth);
                bsm->setSenderLength(myLength);

                bsm->setSenderMbType(mbTypes::Genuine);
                bsm->setSenderAttackType(attackTypes::Genuine);
            }
        }

//        GeneralLib genLib = GeneralLib();
//        Coord posConf = bsm->getSenderPosConfidence();
//        posConf = Coord(posConf.x + genLib.RandomDouble(0, 0.1*posConf.x) ,posConf.y + genLib.RandomDouble(0, 0.1*posConf.y),posConf.z);
//        bsm->setSenderPosConfidence(posConf);
//
//        Coord spdConf = bsm->getSenderSpeedConfidence();
//        spdConf = Coord(spdConf.x + genLib.RandomDouble(0, 0.1*spdConf.x) ,spdConf.y + genLib.RandomDouble(0, 0.1*spdConf.y),spdConf.z);
//        bsm->setSenderSpeedConfidence(spdConf);
//
//        Coord accConf = bsm->getSenderAccelConfidence();
//        accConf = Coord(accConf.x + genLib.RandomDouble(0, 0.1*accConf.x) ,accConf.y + genLib.RandomDouble(0, 0.1*accConf.y),accConf.z);
//        bsm->setSenderAccelConfidence(accConf);
//
//        Coord headConf = bsm->getSenderHeadingConfidence();
//        headConf = Coord(headConf.x + genLib.RandomDouble(0, 0.1*headConf.x) ,headConf.y + genLib.RandomDouble(0, 0.1*headConf.y),headConf.z);
//        bsm->setSenderHeadingConfidence(headConf);

        bsm->setPsid(-1);
        bsm->setChannelNumber(static_cast<int>(Channel::cch));
        bsm->addBitLength(beaconLengthBits);

        wsm->setUserPriority(beaconUserPriority);
    } else if (DemoServiceAdvertisment* wsa =
            dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        wsa->setChannelNumber(static_cast<int>(Channel::cch));
        wsa->setTargetChannel(static_cast<int>(currentServiceChannel));
        wsa->setPsid(currentOfferedServiceId);
        wsa->setServiceDescription(currentServiceDescription.c_str());
    } else {
        if (dataOnSch)
            wsm->setChannelNumber(static_cast<int>(Channel::sch1)); // will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        else
            wsm->setChannelNumber(static_cast<int>(Channel::cch));
        wsm->addBitLength(dataLengthBits);
        wsm->setUserPriority(dataUserPriority);
    }
}

void BaseWaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID,
        cObject* obj, cObject* details) {
    Enter_Method_Silent();
    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == TraCIMobility::parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}

void BaseWaveApplLayer::handlePositionUpdate(cObject* obj) {
    ChannelMobilityPtrType const mobility = check_and_cast<
            ChannelMobilityPtrType>(obj);

    RelativeOffsetConf relativeOffsetConfidence = RelativeOffsetConf(&ConfPosMax,
            &ConfSpeedMax, &ConfHeadMax, &ConfAccelMax, &deltaConfPos,
            &deltaConfSpeed, &deltaConfHead, &deltaConfAccel);

    curPositionConfidence = relativeOffsetConfidence.OffsetPosConf(curPositionConfidenceOrig);
    curSpeedConfidence = relativeOffsetConfidence.OffsetSpeedConf(curSpeedConfidenceOrig);
    curHeadingConfidence = relativeOffsetConfidence.OffsetHeadingConf(curHeadingConfidenceOrig);
    curAccelConfidence = relativeOffsetConfidence.OffsetAccelConf(curAccelConfidenceOrig);

    RelativeOffset relativeOffset = RelativeOffset(&curPositionConfidence,
            &curSpeedConfidence, &curHeadingConfidence, &curAccelConfidence,
            &deltaRPosition, &deltaThetaPosition, &deltaSpeed, &deltaHeading,
            &deltaAccel);

//    GaussianRandom relativeOffset = GaussianRandom(&curPositionConfidence,
//            &curSpeedConfidence, &curHeadingConfidence);


    curPosition = relativeOffset.OffsetPosition(
            mobility->getPositionAt(simTime()));
    curSpeed = relativeOffset.OffsetSpeed(mobility->getCurrentSpeed());
    curHeading = relativeOffset.OffsetHeading(mobility->getCurrentDirection());
    curAccel = relativeOffset.OffsetAccel(mobility->getCurrentAccel());


}

void BaseWaveApplLayer::handleParkingUpdate(cObject* obj) {
    isParked = mobility->getParkingState();
}

void BaseWaveApplLayer::handleLowerMsg(cMessage* msg) {

    BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);
    ASSERT(wsm);

    if (BasicSafetyMessage* bsm = dynamic_cast<BasicSafetyMessage*>(wsm)) {
        receivedBSMs++;
        onBSM(bsm);
    } else if (DemoServiceAdvertisment* wsa =
            dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        receivedWSAs++;
        onWSA(wsa);
    } else {
        receivedWSMs++;
        onWSM(wsm);
    }

    delete (msg);
}

void BaseWaveApplLayer::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
    case SEND_BEACON_EVT: {
        BasicSafetyMessage* bsm = new BasicSafetyMessage();
        populateWSM(bsm);
        if (bsm->getSenderPseudonym() != 1) {
            sendDown(bsm);
        }
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
        break;
    }
    case SEND_WSA_EVT: {
        DemoServiceAdvertisment* wsa = new DemoServiceAdvertisment();
        populateWSM(wsa);
        sendDown(wsa);
        scheduleAt(simTime() + wsaInterval, sendWSAEvt);
        break;
    }
    default: {
        if (msg)
            EV_WARN << "APP: Error: Got Self Message of unknown kind! Name: "
                           << msg->getName() << endl;
        break;
    }
    }
}

void BaseWaveApplLayer::finish() {
    recordScalar("generatedWSMs", generatedWSMs);
    recordScalar("receivedWSMs", receivedWSMs);

    recordScalar("generatedBSMs", generatedBSMs);
    recordScalar("receivedBSMs", receivedBSMs);

    recordScalar("generatedWSAs", generatedWSAs);
    recordScalar("receivedWSAs", receivedWSAs);
}

BaseWaveApplLayer::~BaseWaveApplLayer() {
    cancelAndDelete(sendBeaconEvt);
    cancelAndDelete(sendWSAEvt);
    findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);
}

void BaseWaveApplLayer::startService(Channel channel, int serviceId,
        std::string serviceDescription) {
    if (sendWSAEvt->isScheduled()) {
        error("Starting service although another service was already started");
    }

    mac->changeServiceChannel(channel);
    currentOfferedServiceId = serviceId;
    currentServiceChannel = channel;
    currentServiceDescription = serviceDescription;

    simtime_t wsaTime = computeAsynchronousSendingTime(wsaInterval,
            ChannelType::control);
    scheduleAt(wsaTime, sendWSAEvt);
}

void BaseWaveApplLayer::stopService() {
    cancelEvent(sendWSAEvt);
    currentOfferedServiceId = -1;
}

void BaseWaveApplLayer::sendDown(cMessage* msg) {
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDown(msg);
}

void BaseWaveApplLayer::sendDelayedDown(cMessage* msg, simtime_t delay) {
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDelayedDown(msg, delay);
}

void BaseWaveApplLayer::checkAndTrackPacket(cMessage* msg) {
    if (dynamic_cast<BasicSafetyMessage*>(msg)) {
        EV_TRACE << "sending down a BSM" << std::endl;
        generatedBSMs++;
    } else if (dynamic_cast<DemoServiceAdvertisment*>(msg)) {
        EV_TRACE << "sending down a WSA" << std::endl;
        generatedWSAs++;
    } else if (dynamic_cast<BaseFrame1609_4*>(msg)) {
        EV_TRACE << "sending down a wsm" << std::endl;
        generatedWSMs++;
    }
}
