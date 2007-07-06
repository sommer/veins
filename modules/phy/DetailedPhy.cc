/***************************************************************************
 * file:        DetailedPhy.cc
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/


#include "DetailedPhy.h"
#include <assert.h>
#include "FindModule.h"

Define_Module(DetailedPhy);

/**
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 *
 * If you want to use your own AirFrames you have to redefine createCapsulePkt
 * function.
 */
void DetailedPhy::initialize(int stage)
{
	BasePhyLayer::initialize(stage);
	if (stage ==1){ // 0 has to be there for others (i.e. BasePropagation) to do coreDebug
        headerLength = par("headerLength");

		hasPar("transmitterPower") ? transmitterPower=par("transmitterPower") :
            transmitterPower = static_cast<double>(pm->par("pMax"));
        hasPar("sensitivity") ? sensitivity=FWMath::dBm2mW(par("sensitivity")) :
            sensitivity = FWMath::dBm2mW(static_cast<double>(pm->par("sat")));
        hasPar("carrierFrequency") ? carrierFrequency=par("carrierFrequency") :
            carrierFrequency = static_cast<double>(pm->par("carrierFrequency"));
        hasPar("alpha") ? alpha=par("alpha") :
            alpha = static_cast<double>(pm->par("alpha"));

		bitrate = par("bitrate");
    }
    else if (stage == 2){
        if(alpha < static_cast<double>(pm->par("alpha")))
            error("SnrEval::initialize() alpha can't be smaller than in \
                   ChannelControl. Please adjust your omnetpp.ini file accordingly");
        if(transmitterPower > static_cast<double>(pm->par("pMax")))
            error("SnrEval::initialize() tranmitterPower can't be bigger than \
                   pMax in ChannelControl! Please adjust your omnetpp.ini file accordingly");
        if(sensitivity < FWMath::dBm2mW(static_cast<double>(pm->par("sat"))))
            error("SnrEval::initialize() sensitivity can't be smaller than the signal attentuation threshold (sat) in \
                   ChannelControl. Please adjust your omnetpp.ini file accordingly");
        if(carrierFrequency < static_cast<double>(pm->par("carrierFrequency")))
            error("SnrEval::initialize() carrierFrequency can't be smaller than in \
                   ChannelControl. Please adjust your omnetpp.ini file accordingly");
	}
}

AirFrame *DetailedPhy::encapsMsg(cMessage *msg)
{
	AirFrame *frame = new AirFrame(msg->name(), msg->kind());
	frame->setLength(headerLength);
	frame->setPSend(transmitterPower);
	frame->setChannelId(channel.getActiveChannel());
	frame->setDuration(calcDuration(frame));
	frame->setMove(move);
	frame->encapsulate(msg);
	return frame;
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right after a message is received,
 * i.e. right before it is buffered for 'transmission time'.
 *
 * Here you should decide whether the message is "really" received or
 * whether it's receive power is so low that it is just treated as
 * noise.
 *
 * If the energy of the message is high enough to really receive it
 * you should create an snr list (@ref SnrList) to be able to store
 * sn(i)r information for that message. Every time a new message
 * arrives you can add a new snr value together with a timestamp to
 * that list. Make sure to store a pointer to the mesage together with
 * the snr information to be able to retrieve it later.
 *
 * In this function also an initial SNR value can be calculated for
 * this message.
 *
 * Please take a look at SnrEval to see a "real" example.
 *
 * @sa SnrList, SnrEval
 **/
void DetailedPhy::handleLowerMsgStart(cMessage *msg)
{
    //AirFrame *frame = static_cast<AirFrame *>(msg);

    coreEV <<"in handleLowerMsgStart"<<endl;

    //calculate the receive power

    // calculate snr information, like snr=pSend/noise or whatever....

    // if receive power is actually high enough to be able to read the
    // message and no other message is currently beeing received, store
    // the snr information for the message someweher where you can find
    // it in handleLowerMsgEnd
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right before a packet is handed on to the
 * upper layer, i.e. right after unbufferMsg. Again you can caluculate
 * some more SNR information if you want.
 *
 * You have to copy / create the SnrList related to the message and
 * pass it to sendUp() if you want to pass the message to the decider.
 *
 * Do not forget to send the message to the upper layer with sendUp()
 *
 * For a "real" implementaion take a look at SnrEval
 *
 * @sa SnrList, SnrEval
 */
void DetailedPhy::handleLowerMsgEnd(cMessage *msg)
{
    coreEV << "in handleLowerMsgEnd\n";

    // We need to create a "dummy" snr list that we can pass together
    // with the message to the decider module so that also the
    // DetailedPhy is able to work.
    SnrList snrList;

    // However you can take this as a reference how to create your own
    // snr entries.

    // Everytime you want to add something to the snr information list
    // it has to look like this:
    // 1. create a list entry and fill the fields
    SnrListEntry listEntry;
    listEntry.time = simTime();
    listEntry.snr = 3;          //just a senseless example

    // 2. add an entry to the SnrList
    snrList.push_back(listEntry);

    // 3. pass the message together with the list to the decider
    sendUp( decapsMsg(static_cast<AirFrame *>(msg)) );
}

/*void DetailedPhy::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    ChannelAccess::receiveBBItem(category, details, scopeModuleId);
    if(category == catActiveChannel) {
        channel = *(static_cast<const ActiveChannel *>(details));
    }
}*/
