//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2.h"
#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"

#include "veins/modules/application/ieee1609dot2/ContentUnsecuredData_m.h"

#include "veins/modules/application/ieee1609dot2/ContentChoiceType_m.h"


using namespace veins;

Define_Module(veins::Ieee1609Dot2);

void Ieee1609Dot2::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    EV << "Ieee1609Dot2 initialized " << stage << "\n";
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
    }
}

void Ieee1609Dot2::processSPDU(Ieee1609Dot2Message* spdu)
{
    if(spdu->getData().getProtocolVersion() != 3){
        delete(spdu);
    } else {
        //EV << "Content: " << spdu->getData().getContent() << "\n";

        findHost()->getDisplayString().setTagArg("i", 1, "green");

        int checkType = spdu->getData().getContent().getContentType();

        switch (checkType) {
        case ContentChoiceType::UNSECURE_DATA:
            ContentUnsecuredData unsecuredData = spdu->getData().getContent().getUnsecuredData();
            if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(unsecuredData.getUnsecuredData(), 9999);
                if (!sentMessage) {
                    sentMessage = true;
                    // repeat the received traffic update once in 2 seconds plus some random delay
                    spdu->setSenderAddress(myId);
                    scheduleAt(simTime() + 2 + uniform(0.01, 0.2), spdu->dup());
                }
            break;
        }
    }
}

Ieee1609Dot2Message* Ieee1609Dot2::createSPDU(int type, const char * msg)
{

    Ieee1609Dot2Message* message = new Ieee1609Dot2Message();
    populateWSM(message);

    Ieee1609Dot2Data* data = new Ieee1609Dot2Data();
    data->setProtocolVersion(3);

    Ieee1609Dot2Content* content = new Ieee1609Dot2Content();

    switch(type){
    case ContentChoiceType::UNSECURE_DATA:
        ContentUnsecuredData* contentUnsecuredData = new ContentUnsecuredData();
        contentUnsecuredData->setUnsecuredData(msg);
        content->setUnsecuredData(*contentUnsecuredData);
        break;
    }

    data->setContent(*content);
    message->setData(*data);

    return message;
}



void Ieee1609Dot2::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void Ieee1609Dot2::onWSM(BaseFrame1609_4* frame)
{
    processSPDU(check_and_cast<Ieee1609Dot2Message*>(frame));
}

void Ieee1609Dot2::handleSelfMsg(cMessage* msg)
{
    if (Ieee1609Dot2Message* wsm = dynamic_cast<Ieee1609Dot2Message*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendDown(wsm->dup());
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void Ieee1609Dot2::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {

            EV << "Creating new SPUD\n";
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            Ieee1609Dot2Message *wsm = createSPDU(ContentChoiceType::UNSECURE_DATA, mobility->getRoadId().c_str());

            // host is standing still due to crash
            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                // started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            }
            else {
                // send right away on CCH, because channel switching is disabled*
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
}
