/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval.cc
 *
 * author:      Jamarin Phongcharoen, Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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
 * description: - SnrEval class
 *              - mains tasks are to determine the SNR for a message and
 *                to simulate a transmission delay
 *
 ***************************************************************************/

#include "SnrEval.h"
#include "Bitrate.h"
#include <FWMath.h>
#include "NicControlType.h"

#include <ChannelControl.h>

Define_Module(SnrEval);

/**
 * All values not present in the ned file will be read from the
 * ChannelControl module or assigned default values.
 **/
void SnrEval::initialize(int stage)
{
    BasicSnrEval::initialize(stage);
  
    if(stage==0){
        
        hasPar("thermalNoise") ? thermalNoise=FWMath::dBm2mW(par("thermalNoise")) :
            thermalNoise=FWMath::dBm2mW(-100);
        
        useTorus = false;
        if(cc->hasPar("useTorus")) useTorus = cc->par("useTorus").boolValue();

        playground.x = cc->par("playgroundSizeX");
        playground.y = cc->par("playgroundSizeY");

        hasPar("publishRSSIAlways") ?
            publishRSSIAlways = par("publishRSSIAlways").boolValue() :
            publishRSSIAlways = false;
        
        radioState = RadioState::RECV;
        rssi.setRSSI(thermalNoise);

        // subscribe for information about the radio
        RadioState cs;
        Bitrate br;
        
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catRSSI = bb->getCategory(&rssi);

        catBitrate = bb->subscribe(this, &br, parentModule()->id());        
    }
    else if(stage == 1) {
        waveLength = speedOfLight/carrierFrequency;
        pathLossAlphaHalf = alpha/2.0;

        // initialize the pointer of the snrInfo with NULL to indicate
        // that currently no message is received

        snrInfo.ptr = 0;	          
        snrInfo.rcvdPower = 0;
        snrInfo.sList = SnrList();
        
        // initialize  the receive time with NULL 
        recvTime = 0.0;
        nicModuleId = parentModule()->id();
        noiseLevel = thermalNoise;
        
        bb->publishBBItem(catRSSI, &rssi, nicModuleId);

        EV << "carrierFrequency: " << carrierFrequency
           << " waveLength: " << waveLength
           << " sensitivity: "<<sensitivity
           << " pathLossAlpha: " << alpha
           << " pathLossAlphaHalf: " << pathLossAlphaHalf
           << " publishRSSIAlways: " << publishRSSIAlways
           << " noiseLevel: " << noiseLevel << endl;
    }
}

/**
 * This function is called right after a packet arrived, i.e. right
 * before it is buffered for 'transmission time'.
 *
 *
 * First must to decide whether the message is "really" received or
 * whether it's receive power is so low that it is just treated as noise.
 * If the energy of the message is high enough to really receive it
 * an snr list (@ref SnrList) should be created to be able to store
 * sn(i)r information for that message. Every time a new message
 * a new snr value with a timestamp is added to that list.
 * If the receive power of the frame is below the receiver sensibility, the
 * frame is added to the accumulated noise.
 *
 **/
void SnrEval::handleLowerMsgStart(AirFrame *frame)
{
    double rcvdPower = calcRcvdPower(frame);

    // store the receive power in the recvBuff
    recvBuff[frame] = rcvdPower;

    // if receive power is bigger than sensitivity
    // and currently not receiving another message

    if(snrInfo.ptr == 0) {
        if(rcvdPower >= sensitivity) {
            EV <<"with rcvdPower: "<<rcvdPower<<" sensitivity: "<<sensitivity<<endl;

           // Put frame and related SnrList in receive buffer
            snrInfo.ptr = frame;
            snrInfo.rcvdPower = rcvdPower;
            snrInfo.sList.clear();
    
        // add initial snr value
            EV <<"first frame arrived!\n";
            addNewSnr();
            rssi.setRSSI(rssi.getRSSI() + rcvdPower);
            if(radioState == RadioState::RECV) bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
        else {
            //add receive power to the noise level
            EV <<"frame discared -- just noise with rcvdPower: "<<rcvdPower<<" sensitivity: "<<sensitivity<<endl;
            noiseLevel += rcvdPower;
            rssi.setRSSI(rssi.getRSSI() + noiseLevel);
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
    }
    // if a message is beeing received add a new snr value
    else {
        // update snr info for currently beeing received message
        EV <<"add new snr value to snr list of message beeing received\n";
        noiseLevel += rcvdPower;
        rssi.setRSSI(rssi.getRSSI() + noiseLevel);
        addNewSnr();
        if(publishRSSIAlways && (radioState == RadioState::RECV)) {
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
    }
    EV <<"with rcvdPower: "<<rcvdPower<<endl;
    EV<<" rssi: "<< rssi.getRSSI()<<endl;
    EV<<" noiseLevel: "<< noiseLevel<<endl;
    EV<<"frame finished at"<<simTime()+frame->getDuration()<<endl;
}


/**
 * This function is called right after the transmission is over,
 * i.e. right after unbuffering.
 *
 * First check the current radio state. The radio must not be switched from RECV
 * state before the end of message is received. Otherwise drop the message.
 * Additionally the snr information of the currently being received message (if any)
 * has to be updated with the receivetime as timestamp and a new snr value.
 * The new SnrList and the AirFrame are sent to the decider.
 *
 **/
void SnrEval::handleLowerMsgEnd(AirFrame *frame)
{
    if(snrInfo.ptr == frame){    
        EV <<"first frame finished " << endl;
        rssi.setRSSI(noiseLevel);
        
        if(radioState == RadioState::RECV) {
            EV <<"receiving of frame is over, foward the frame to the decider\n";
            
            // foward the packet to an Deceider with the possibly new snr information
            if(snrInfo.sList.front().time > recvTime) {	
                EV<<" MessageSnr_time: "<<snrInfo.sList.front().time<<endl;
                EV<<" recvtime: "<<recvTime<<endl;
            } else {
                EV<<"radio erst nach Frameanfang in RECV umgeschaltet!\n";
                modifySnrList(snrInfo.sList);
                EV<<snrInfo.sList.front().time<<endl;
            }	         

            recvBuff.erase(frame);
            sendUp(frame, snrInfo.sList);
            // delete the pointer to indicate that no message is currently
            // beeing received and clear the list
            snrInfo.ptr = NULL;
            snrInfo.sList.clear();
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
            EV <<"packet sent to the decider\n";      
        }
        else{
            // delete the pointer to indicate that no message is currently
            // beeing received and clear the list
            snrInfo.ptr = NULL;
            snrInfo.sList.clear();  
      
            // delete the frame from the recvBuff
            recvBuff.erase(frame);     
            // message should be deleted
            delete frame;
            EV <<"message deleted radio state: "<<radioState<<endl;
        } 
    }
    // all other message are noise
    else {
        EV <<"reception of noise message over, removing recvdPower from noiseLevel....\n";
      
        // get the rcvdPower and subtract it from the noiseLevel
        cRecvBuff::iterator it = recvBuff.find(frame);
        noiseLevel -= it->second;
        rssi.setRSSI(noiseLevel);
        // delete message from the recvBuff
        recvBuff.erase(it);

        // update snr info for message currently being received if any
        if(snrInfo.ptr != NULL){
            addNewSnr();
            EV << " noise frame ended during a reception "
               << " radiostate = " << radioState << endl;
            
            if(publishRSSIAlways && (radioState == RadioState::RECV)) {
                bb->publishBBItem(catRSSI, &rssi, nicModuleId);
            }
        }
        else {
            EV << " end message, radio state: " << radioState << endl;
            if(radioState == RadioState::RECV) bb->publishBBItem(catRSSI, &rssi, nicModuleId);            
        }
        // message should be deleted
        delete frame;
        EV <<"message deleted\n";
    }
    EV<<"noiseLevel "<<noiseLevel<<" rssi: "<<rssi.getRSSI()<<endl;
}

/**
 * The SnrEval has subscreibed the NewRadioState and will be informed each time the
 * radio state changes. The time radio switched to RECV must be noted.
 **/

void SnrEval::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    BasicSnrEval::receiveBBItem(category, details, scopeModuleId);    
    if(category == catRadioState) {
        radioState = static_cast<const RadioState *>(details)->getState();
        if(radioState == RadioState::RECV) {
            recvTime = simTime();
            EV <<"Radio switched to RECV at T= "<<recvTime<<endl;
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
    }
    else if(category == catBitrate) {
        bitrate = static_cast<const Bitrate *>(details)->getBitrate();
    }
}

/**
 * The Snr information of the buffered message is updated....
 **/
void SnrEval::addNewSnr()
{
    //print("NoiseLevel: "<<noiseLevel<<" recvPower: "<<snrInfo.rcvdPower);

    snrInfo.sList.push_back(SnrListEntry());
    snrInfo.sList.back().time=simTime();
    snrInfo.sList.back().snr=snrInfo.rcvdPower/noiseLevel;
    EV<<"New Snr added: "<<snrInfo.sList.back().snr
      <<" at time:"<<snrInfo.sList.back().time<<endl;
}

/**
 * The Snr information of the buffered message is updated....
 *
 * If the frame arrived before the radio was ready to received, the snir 
 * information must be new calculated.
 * The new snir information is a SnrList that lists all different snr
 * levels together with the point of time starting from which the radio 
 * is switched in the received mode.
 *
 **/
void SnrEval::modifySnrList(SnrList& list)
{
    for(SnrList::iterator iter=list.begin(); iter!=list.end();iter++) {
        EV<<" MessageSnr_time: "<<iter->time<<endl;
        EV<<" recvtime: "<<recvTime<<endl;
        if(iter->time>=recvTime) {
            list.erase(list.begin(), iter);
            iter->time=recvTime;
            break;
        }
    }
}

/**
 * This function simply calculates with how much power the signal
 * arrives "here". If a different way of computing the path loss is
 * required this function can be redefined.
 **/
double SnrEval::calcPathloss(AirFrame* frame)
{
    double time;
    double sqrdistance;
    double pRecv;
    double pSend = frame->getPSend();
    
    Coord myPos(hostMove.startPos);
    HostMove rHm(frame->getHostMove());
    Coord framePos(rHm.startPos);
    
    // Calculate the receive power of the message
    // get my position
    time = simTime() - hostMove.startTime;
    if(hostMove.speed != 0) {
        myPos.x += time*hostMove.speed*hostMove.direction.x;
        myPos.y += time*hostMove.speed*hostMove.direction.y;
    }
    
    //get Position of the sending node
    time = simTime() - rHm.startTime;
    
    if(rHm.speed != 0) {
        framePos.x += time*rHm.speed*rHm.direction.x;
        framePos.y += time*rHm.speed*rHm.direction.y;
    }

    //calculate distance and receive power
    if(useTorus) {
        sqrdistance = myPos.sqrTorusDist(framePos, playground);
    } else {
        sqrdistance = myPos.sqrdist(framePos);
    }

    EV <<"receiving frame start pos x: "<<framePos.x
       <<" y: "<<framePos.y
       <<" myPos x: "<<myPos.x <<" y: "<< myPos.y
       << " distance: "<< sqrt(sqrdistance) << " Torus: "<<useTorus<< endl;
        
    pRecv = (pSend*waveLength*waveLength /
             (16.0*M_PI*M_PI*pow(sqrdistance,pathLossAlphaHalf)));
    return (pRecv < pSend) ? pRecv : pSend;
}

const double SnrEval::speedOfLight = 300000000.0;


