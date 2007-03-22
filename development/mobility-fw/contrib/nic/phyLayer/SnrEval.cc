/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval.cc
 *
 * author:      Jamarin Phongcharoen, Andreas Koepke, Jochen Adamek
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
        
        hasPar("thermalNoise") ? thermalNoise=FWMath::dBm2mW(par("thermalNoise").doubleValue()) :
            thermalNoise=FWMath::dBm2mW(-100);
        
        useTorus = false;
        if(cc->hasPar("useTorus")) useTorus = cc->par("useTorus").boolValue();

        playground.x = cc->par("playgroundSizeX");
        playground.y = cc->par("playgroundSizeY");

        hasPar("publishRSSIAlways") ?
            publishRSSIAlways = par("publishRSSIAlways").boolValue() :
            publishRSSIAlways = false;
        
        radioState = RadioState::RECV;
        defaultChannel.rssi.setRSSI(thermalNoise);

        defaultChannel.indication.setState(MediumIndication::IDLE);
        
        // subscribe for information about the radio
        RadioState cs;
        Bitrate br;
        
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catRSSI = bb->getCategory(&defaultChannel.rssi);
        catBitrate = bb->subscribe(this, &br, parentModule()->id());        
        catIndication = bb->getCategory(&defaultChannel.indication);

    }
    else if(stage==1) {
        waveLength = speedOfLight/carrierFrequency;
        pathLossAlphaHalf = alpha/2.0;

        // initialize the pointer of the snrInfo with NULL to indicate
        // that currently no message is received

        snrInfo.ptr = 0;	          
        snrInfo.rcvdPower = 0;
        snrInfo.sList = SnrList();
        
        // initialize  the receive time with NULL 
        defaultChannel.recvTime = 0.0;
        nicModuleId = parentModule()->id();
        defaultChannel.noiseLevel = thermalNoise;
        
        bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
        bb->publishBBItem(catIndication, &defaultChannel.indication, nicModuleId);

        EV << "carrierFrequency: " << carrierFrequency
           << " waveLength: " << waveLength
           << " sensitivity: "<<sensitivity
           << " pathLossAlpha: " << alpha
           << " pathLossAlphaHalf: " << pathLossAlphaHalf
           << " publishRSSIAlways: " << publishRSSIAlways
           << " noiseLevel: " << defaultChannel.noiseLevel << endl;
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
void SnrEval::handleLowerMsgStart(AirFrame *frame){
   
    double rcvdPower = calcRcvdPower(frame);
    defaultChannel.rcvdPower = rcvdPower;
    unsigned channelID = frame->getChannelId();
    
    
    while(channelID >= usedChannel.size()){
                          EV << "vector size must be changed";
                          usedChannel.push_back(defaultChannel);
                           }

    // store the receive power in the recvBuff
    recvBuff[frame] = defaultChannel.rcvdPower;

    // if receive power is bigger than sensitivity
    // and currently not receiving another message

    if(snrInfo.ptr == 0) {
        if(defaultChannel.rcvdPower >= sensitivity) {
            EV <<"with rcvdPower: "<<defaultChannel.rcvdPower<<" sensitivity: "<<sensitivity<<endl;

           // Put frame and related SnrList in receive buffer
            snrInfo.ptr = frame;
            snrInfo.rcvdPower = defaultChannel.rcvdPower;
            snrInfo.sList.clear();
    
          // add initial snr value
            EV <<"first frame arrived!\n";
            addNewSnr();
            defaultChannel.rssi.setRSSI(defaultChannel.rssi.getRSSI() + defaultChannel.rcvdPower);
            defaultChannel.indication.setState(MediumIndication::BUSY);
            if(radioState == RadioState::RECV) {
                bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
                bb->publishBBItem(catIndication, &defaultChannel.indication, nicModuleId);
            }
        }
        else {
            //add receive power to the noise level
            EV <<"frame discared -- just noise with rcvdPower: "<<defaultChannel.rcvdPower<<" sensitivity: "<<sensitivity<<endl;
            defaultChannel.noiseLevel += defaultChannel.rcvdPower;
            defaultChannel.rssi.setRSSI(defaultChannel.rssi.getRSSI() + defaultChannel.noiseLevel);
            bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
        }
    }
    // if a message is beeing received add a new snr value
    else {
        // update snr info for currently beeing received message
        EV <<"add new snr value to snr list of message beeing received\n";
        defaultChannel.noiseLevel += defaultChannel.rcvdPower;
        defaultChannel.rssi.setRSSI(defaultChannel.rssi.getRSSI() + defaultChannel.noiseLevel);
        addNewSnr();
        if(publishRSSIAlways && (radioState == RadioState::RECV)) {
            bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
        }
    }
    EV <<"with rcvdPower: "<<defaultChannel.rcvdPower<<endl;
    EV<<" rssi: "<< defaultChannel.rssi.getRSSI()<<endl;
    EV<<" noiseLevel: "<< defaultChannel.noiseLevel<<endl;
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
        defaultChannel.rssi.setRSSI(defaultChannel.noiseLevel);
        defaultChannel.indication.setState(MediumIndication::IDLE);
        if(radioState == RadioState::RECV) {
            EV <<"receiving of frame is over, foward the frame to the decider\n";
            
            // foward the packet to an Deceider with the possibly new snr information
            if(snrInfo.sList.front().time < defaultChannel.recvTime) {
                modifySnrList(snrInfo.sList);
                EV<<"radio switched to RX after frame transmission started: " <<snrInfo.sList.front().time<<endl;
            }
            recvBuff.erase(frame);
            sendUp(frame, snrInfo.sList);
            // delete the pointer to indicate that no message is currently
            // beeing received and clear the list
            snrInfo.ptr = NULL;
            snrInfo.sList.clear();
            bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
            bb->publishBBItem(catIndication, &defaultChannel.indication, nicModuleId);
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
        defaultChannel.noiseLevel -= it->second;
        defaultChannel.rssi.setRSSI(defaultChannel.noiseLevel);
        // delete message from the recvBuff
        recvBuff.erase(it);

        // update snr info for message currently being received if any
        if(snrInfo.ptr != NULL){
            addNewSnr();
            EV << " noise frame ended during a reception "
               << " radiostate = " << radioState << endl;
            
            if(publishRSSIAlways && (radioState == RadioState::RECV)) {
                bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
            }
        }
        else {
            EV << " end message, radio state: " << radioState << endl;
            if(radioState == RadioState::RECV) bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);            
        }
        
        //channelUsed[i].clear();
        
        /**for(size_t=usedChannel.size-1;i>1;i--){
                 channelUsed[i].clear();
                 
        EV <<"channel deleted\n";**/                                      
        // message should be deleted
        delete frame;
        EV <<"message deleted\n";
    }
    EV<<"noiseLevel "<<defaultChannel.noiseLevel<<" rssi: "<<defaultChannel.rssi.getRSSI()<<endl;
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
            defaultChannel.recvTime = simTime();
            EV <<"Radio switched to RECV at T= "<<defaultChannel.recvTime<<endl;
            bb->publishBBItem(catRSSI, &defaultChannel.rssi, nicModuleId);
            bb->publishBBItem(catIndication, &defaultChannel.indication, nicModuleId);
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
    snrInfo.sList.back().snr=snrInfo.rcvdPower/defaultChannel.noiseLevel;
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
    SnrList::iterator iter;
    for(iter = list.begin(); iter != list.end(); iter++) {
        EV<<" MessageSnr_time: "<<iter->time<<endl;
        EV<<" recvtime: "<<defaultChannel.recvTime<<endl;
        if(iter->time >= defaultChannel.recvTime) {
            list.erase(list.begin(), iter);
            iter->time=defaultChannel.recvTime;
            break;
        }
    }
    if(iter == list.end()) {
        list.erase(list.begin(), --iter);
        list.begin()->time = defaultChannel.recvTime;
    }
}

/**
 * This function simply calculates with how much power the signal
 * arrives "here". If a different way of computing the path loss is
 * required this function can be redefined.
 **/


 const double SnrEval::speedOfLight = 300000000.0;

 double SnrEval::calcPathloss(AirFrame *frame)
                                 {
        
 double time;
 double attenuation=0.0;
 //double pRecv;
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
    
    //pRecv = (pSend*waveLength*waveLength /
    //         (16.0*M_PI*M_PI*pow(calcSqrdistance(myPos, framePos),pathLossAlphaHalf)));
                      
    if(pSend==0){
                 //cerr << pSend "pSend is 0\n";
 
     }else if(calcSqrdistance(myPos, framePos) > 1.0)
     { attenuation = (16.0*M_PI*M_PI*pow(calcSqrdistance(myPos, framePos),pathLossAlphaHalf));
     }
     //transmission delay in BasicSnrEval
     else{
          attenuation = 1.0;
          }
     
     return attenuation;
     
}

double SnrEval::calcSqrdistance(const Coord &myPos,const Coord &framePos){
       double sqrdistance = 0.0;
       
       //calculate distance
    if(useTorus) {
        sqrdistance = myPos.sqrTorusDist(framePos, playground);
    } else {
        sqrdistance = myPos.sqrdist(framePos);
    }
       
       
    EV << " sqrdistance: "<< sqrt(sqrdistance) << " Torus: "<<useTorus<< endl;
    
    return sqrdistance;
    
}


void SnrEval::finish(){
}
     
     
