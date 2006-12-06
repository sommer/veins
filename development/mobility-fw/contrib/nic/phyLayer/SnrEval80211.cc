// file:        SnrEval80211.cc
//
//  author:      Marc Löbbers
// copyright:   (c) by Tralafitty
//              Telecommunication Networks Group
//              TU-Berlin
// email:       loebbers@tkn.tu-berlin.de

// part of: framework implementation developed by tkn description: - a
// snrEval extension for the use with the other 802.11 modules


#include "SnrEval80211.h"
#include "Consts80211.h"
#include "AirFrame80211_m.h"
#include "PhyControlInfo.h"


Define_Module(SnrEval80211);

void SnrEval80211::initialize(int stage)
{
    SnrEval::initialize(stage);
    if (stage == 0) {
        EV << "initializing stage 0\n";
        headerLength = 192;     //has to be 192; this makes sure it is!        
    }
}


/**
 * The duration of the packet is computed, with respect to the
 * different bitrates of header and data. The header is sent with
 * 1Mbit/s and the rest with the bitrate encoded in the frame
 */
double SnrEval80211::calcDuration(cMessage *frame)
{
    double duration;
    AirFrame80211 *f = static_cast<AirFrame80211 *>(frame);
    
    EV << "bits without header: " << frame->length() - headerLength
       << ", bits header: " << headerLength << endl;
    duration = ((frame->length() - headerLength) / f->getBitrate() + headerLength / BITRATE_HEADER);
    return duration;
}

/**
 * Call the right processing functions
 */
void SnrEval80211::handleMessage(cMessage *msg)
{
    if (msg->arrivalGateId() == uppergateIn){
        AirFrame80211 *frame = encapsMsg(msg);
        handleUpperMsg(static_cast<AirFrame *>(frame));
    }
    else {
        SnrEval::handleMessage(msg);
    }
}

/**
 * This function encapsulates messages from the upper layer into an
 * AirFrame, copies the type and channel fields, adds the
 * headerLength, sets the pSend (transmitterPower) and returns the
 * AirFrame.
 */
AirFrame80211 *SnrEval80211::encapsMsg(cMessage *msg)
{
    AirFrame80211 *frame = new AirFrame80211(msg->name(), msg->kind());
    PhyControlInfo *pco = static_cast<PhyControlInfo *>(msg->removeControlInfo());
    frame->setBitrate(pco->getBitrate());
    frame->setPSend(transmitterPower);
    frame->setLength(headerLength);
    frame->setHostMove(hostMove);
    frame->setChannelId(channel.getActiveChannel());
    frame->encapsulate(msg);
    frame->setDuration(calcDuration(frame));
    frame->setHostMove(hostMove);
    EV << "SnrEval80211::encapsMsg duration: " <<  frame->getDuration() << "\n";
    return frame;
}
