/*
 *	copyright:   	(C) 2006 Computer Networks Group (CN) at
 *			University of Paderborn, Germany.
 *	
 *			This program is free software; you can redistribute it
 *			and/or modify it under the terms of the GNU General Public
 *			License as published by the Free Software Foundation; either
 *			version 2 of the License, or (at your option) any later
 *			version.
 *
 *			For further information see file COPYING
 *			in the top level directory.
 *
 *			Based on Mobility Framework 2.0p2 developed at 
 *			TKN (TU Berlin) and, ChSim 2.1 developed at CN 
 *			(Uni Paderborn).
 *
 *	file:		$RCSfile: SnrEvalOFDM.cc,v $
 *
 *      last modified:	$Date: 2007/03/26 12:44:45 $
 *      by:		$Author: tf $
 *
 *      informatin:	-
 *
 *	changelog:   	$Revision: 1.22 $
 *			$Log: SnrEvalOFDM.cc,v $
 *			Revision 1.22  2007/03/26 12:44:45  tf
 *			- incorporated changes for
 *			  - coding gain (ref: Proakis)
 *			  - time selective fading (channel coherence time estimation, ref: Proakis)
 *			
 *			Revision 1.21  2007/03/18 17:03:29  tf
 *			- implemented takeChannelSample
 *			- timer is scheduled in handleLowerMsgStart()
 *			- TODO: estimate channel coherence time
 *			
 *			Revision 1.20  2007/02/22 14:03:18  tf
 *			- really fixed noise message reception bug
 *			
 *			Revision 1.19  2007/02/22 09:56:43  tf
 *			- fixed noise message reception bug
 *			
 *			Revision 1.18  2007/02/13 12:51:53  tf
 *			- using parameter globalSpeed for setting speed of terminals
 *			
 *			Revision 1.17  2007/02/06 20:18:14  tf
 *			- added some debug output
 *			
 *			Revision 1.16  2007/01/31 17:29:04  tf
 *			*** empty log message ***
 *			
 *			Revision 1.15  2007/01/31 17:26:13  tf
 *			- minor bug fix: full-featured OFDMChannelSim used for eRTS
 *			
 *			Revision 1.14  2007/01/31 17:13:25  tf
 *			- added OFDMChannelSim instance for pathloss calculation
 *			
 *			Revision 1.13  2007/01/31 16:52:09  tf
 *			- SnrEval uses linear scale now
 *			
 *			Revision 1.12  2007/01/31 09:37:15  tf
 *			- using distance instead of square of distance
 *			
 *			Revision 1.11  2007/01/30 19:35:31  tf
 *			- added newControlDelay field in MAC frame
 *			- some minor bug fixes
 *			- Problem: sending of pending fragments in extended cycle with more than
 *			  one STA
 *			
 *			Revision 1.10  2007/01/29 22:50:32  tf
 *			- rcvdPowerMax is used for RSSI instead rcvdPowerMin
 *			- busy/carrier sensing bug fixed
 *			- carrier sensing is based on one subcarrier
 *			
 *			Revision 1.9  2007/01/29 18:10:18  tf
 *			- some more debug output
 *			
 *			Revision 1.8  2007/01/28 23:13:00  tf
 *			- fixed many bugs
 *			- simulation segfaults for networks with more than 2 hosts in sendDATAframe()
 *			
 *			Revision 1.7  2007/01/24 13:10:52  tf
 *			- added handling of OFDMA AirFrames
 *			  - only AirFrames for local NIC are stored
 *			  - collisions are calculated for:
 *			    - OFDMA AirFrame with standard 802.11a frame (or vice versa)
 *			    - OFDMA AirFrames with different magic numbers (possibly not disjunct
 *			      subband sets used)
 *			- collision detection works as follows
 *			  1. determine minRcvdPower from new frame
 *			  2. add minRcvdPower to noise level (optimistic)
 *			  3. add entry based on new noise level to SINR list
 *			  4. let decider work
 *			
 *			Revision 1.6  2007/01/17 17:01:47  tf
 *			- bugs in error calculation fixed
 *			
 *			Revision 1.5  2007/01/16 14:47:35  tf
 *			- fixed noise level addition bug ((-1)*(-1)=(+1))
 *			
 *			Revision 1.4  2007/01/15 16:29:15  tf
 *			- added many things for 802.11a OFDM conform channel state generation
 *			  - params for chsim readed from ini file
 *			  - creation and deletion of chsim object per node
 *			  - mobility still using existing mf mobility code
 *			  - using rcvdPowerMin and rcvdPowerMax for indication of some received power
 *			    values wherever a full array is not useful
 *			  - added finish() method
 *			  - added new sendUp() for passing OFDM subband aware received power indication
 *			
 */

#include "SnrEvalOFDM.h"
#include "Bitrate.h"
#include <FWMath.h>
#include "NicControlType.h"
#include <AirFrame80211_m.h>	// for channel sim object selection // TF
#include <Mac80211Pkt_m.h>	// for channel sim object selection // TF
#include <Consts80211a.h>

Define_Module(SnrEvalOFDM);

/**
 * All values not present in the ned file will be read from the
 * ChannelControl module or assigned default values.
 **/
void SnrEvalOFDM::initialize(int stage)
{
    BasicSnrEval::initialize(stage);

    double subbands;
    double pilots;
  
    if(stage==0){
        
        useTorus = false;
        if(cc->hasPar("useTorus")) useTorus = cc->par("useTorus").boolValue();

        playground.x = cc->par("playgroundSizeX");
        playground.y = cc->par("playgroundSizeY");

        hasPar("publishRSSIAlways") ?
            publishRSSIAlways = par("publishRSSIAlways").boolValue() :
            publishRSSIAlways = false;
	subbands = par("subbands");
	pilots = par("pilots");
	globalSpeed = par("globalSpeed");

	ignoreFrame=-1;
	ignoreCount=0;

        //transmissionPower = par("transmissionPower");
        transmissionPower = FWMath::dBm2mW(par("transmissionPower"));
	EV<<"transmissionPower is "<<transmissionPower<<endl;
        //transmissionPowerPerSubcarrier = transmissionPower - 10*log10(subbands+pilots);
        transmissionPowerPerSubcarrier = transmissionPower/(subbands+pilots);
	EV<<"transmissionPowerPerSubcarrier is "<<transmissionPowerPerSubcarrier<<endl;
        //whiteGaussianNoise = par("whiteGaussianNoise");
        whiteGaussianNoise = FWMath::dBm2mW(par("whiteGaussianNoise"));
	EV<<"whiteGaussianNoise is "<<whiteGaussianNoise<<endl;
        //whiteGaussianNoisePerSubcarrier = whiteGaussianNoise - 10*log10(subbands+pilots);
        whiteGaussianNoisePerSubcarrier = whiteGaussianNoise/(subbands+pilots);
	EV<<"whiteGaussianNoisePerSubcarrier is "<<whiteGaussianNoisePerSubcarrier<<endl;
	WATCH(whiteGaussianNoise);
	WATCH(whiteGaussianNoisePerSubcarrier);
        
        radioState = RadioState::RECV;
        rssi.setRSSI(whiteGaussianNoisePerSubcarrier);

        // subscribe for information about the radio
        RadioState cs;
        Bitrate br;
        
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catRSSI = bb->getCategory(&rssi);
        catBitrate = bb->subscribe(this, &br, parentModule()->id());        

	// parameters for channel sim
	LIGHTSPEED = par("lightspeed");
	TENLOGK = par("tenlogk");
	ALPHA = par("alpha");
	MEAN = par("mean");
	STD_DEV = par("std_dev");   // 250 = 1 DL, 250*500=125000 = 1 second
	FADING_PATHS = par("fadingPaths");
	CENTER_FREQUENCY = par("center_frequency");
	DELAY_RMS = par("delay_rms");
	FREQUENCY_SPACING = par("freq_spacing");
	SUBBANDS = par("subbands");
	CALCULATE_PATH_LOSS = par("calculatePathLoss");
	CALCULATE_SHADOWING = par("calculateShadowing");
	CALCULATE_FADING = par("calculateFading");
	CORRELATED_SUBBANDS = par("correlated_subbands");

        symbolTime = par("symbolTime");
        logarithmicSymbolTime = 10*log10(symbolTime); // not used
        symbolRate = 1/symbolTime;
	logarithmicSymbolRate = 10*log10(symbolRate); // not used

	currentMagicNumber = -1;

	pathLossChannelSim = new OFDMChannelSim(LIGHTSPEED, TENLOGK, ALPHA, MEAN, STD_DEV, DELAY_RMS, FREQUENCY_SPACING, FADING_PATHS, CENTER_FREQUENCY, SUBBANDS, CALCULATE_PATH_LOSS, 0, 0, CORRELATED_SUBBANDS);;


    }
    else if(stage == 1) {
        // initialize the pointer of the snrInfo with NULL to indicate
        // that currently no message is received
        snrInfo.ptr = 0;	          
        snrInfo.sList = OFDMSnrList();
        
        // initialize  the receive time with NULL 
        recvTime = 0.0;
        nicModuleId = parentModule()->id();
        noiseLevel = whiteGaussianNoisePerSubcarrier;
        
        bb->publishBBItem(catRSSI, &rssi, nicModuleId);

	myMacAddr = parentModule()->id(); // get MAC addr of my nic // cross layer 
	sensitivity = FWMath::dBm2mW(par("ccaThreshold"));

        EV << " publishRSSIAlways: " << publishRSSIAlways
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
void SnrEvalOFDM::handleLowerMsgStart(AirFrame *frame)
{
    EV<<"handleLowerMsgStart() called"<<endl;
    // simulation abstraction:	// TF
    // 	if message is not for me, calculate only path loss
    // 	otherwise use more sophisticated mechanism provided by chsim
   
    AirFrame80211* rcvdAirFrame = static_cast<AirFrame80211*>(frame);
    Mac80211Pkt*  rcvdMacFrame = static_cast<Mac80211Pkt*>(frame->encapsulatedMsg());

    OFDMChannelSim* curChannelSim;
    ChannelState chState;

    if (channels.count(rcvdMacFrame->getSrcAddr())>0) // we have already a channel sim object for channel from this host
    {
    	// use the existing channel sim object (has state information for autocorrelation...)
    	curChannelSim = channels[rcvdMacFrame->getSrcAddr()];
	EV << "using existing OFDMChannelSim object for channel from "<<rcvdMacFrame->getSrcAddr()<<endl;
    }
    else
    {
    	// create new channel sim object
        curChannelSim = new OFDMChannelSim(LIGHTSPEED, TENLOGK, ALPHA, MEAN, STD_DEV, DELAY_RMS, FREQUENCY_SPACING, FADING_PATHS, CENTER_FREQUENCY, SUBBANDS, CALCULATE_PATH_LOSS, CALCULATE_SHADOWING, CALCULATE_FADING, CORRELATED_SUBBANDS);
	// store chsim object in map
	channels[rcvdMacFrame->getSrcAddr()] = curChannelSim;
	EV << "created new OFDMChannelSim object for channel from "<<rcvdMacFrame->getSrcAddr()<<endl;
    }

    // calculation of distances
    Coord myPos(hostMove.startPos);
    HostMove rHm(frame->getHostMove());
    Coord framePos(rHm.startPos);
    double time;
    double sqrdistance;
    
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
        EV<<"Speed is: "<<rHm.speed<<endl;
        EV<<"GlobalSpeed is: "<<globalSpeed<<endl;
    }
    else {
        EV<<"Speed is: "<<rHm.speed<<endl;
        EV<<"GlobalSpeed is: "<<globalSpeed<<endl;
    }

    //calculate distance and receive power
    if(useTorus) {
        sqrdistance = myPos.sqrTorusDist(framePos, playground);
    } else {
        sqrdistance = myPos.sqrdist(framePos);
 
 
    EV <<"receiving frame start pos x: "<<framePos.x
       <<" y: "<<framePos.y
       <<" myPos x: "<<myPos.x <<" y: "<< myPos.y
       << " distance: "<< sqrt(sqrdistance) <<" sqrdistance: " <<sqrdistance << " Torus: "<<useTorus<< endl;
    }

   
    if ((rcvdMacFrame->getDestAddr()==myMacAddr) || (rcvdMacFrame->kind()==ERTS)) {
    	EV<<"Using full-featured OFDMChannelSim..."<<endl;
	if (sqrdistance>1.0) {
    	    chState = curChannelSim->calculateLoss(sqrt(sqrdistance), globalSpeed);
    	    chState.speed = globalSpeed;
    	    chState.distance = sqrt(sqrdistance);

	    // TODO estimate channel coherence time
	    // disabled //channelCoherenceTime = frame->getDuration()+0.1;
	    channelCoherenceTime = 1/(sqrt(2)*M_PI*(CENTER_FREQUENCY*chState.speed/LIGHTSPEED));
	    EV<<"channel coherence time "<<channelCoherenceTime<<"="<<1<<"/("<<sqrt(2)<<"*"<<M_PI<<"*("<<CENTER_FREQUENCY<<"*"<<chState.speed<<"/"<<LIGHTSPEED<<"))"<<endl;
	    // schedule channel sample message if necessary
	    if (channelCoherenceTime<frame->getDuration()) {
	    	EV<<"channel coherence time "<<channelCoherenceTime<<" ("<<frame->getDuration()<<"), taking additional samples"<<endl;
	    	sampleTimer->setTimeLeft(frame->getDuration()-channelCoherenceTime);
		sampleTimer->setSenderMacAddr(rcvdMacFrame->getSrcAddr());
		sampleTimer->setDistance(chState.distance);
		sampleTimer->setSpeed(chState.speed);
		sampleTimer->setSampleInterval(channelCoherenceTime);
		scheduleAt(simTime()+channelCoherenceTime,sampleTimer);
	    }
	} 
	else {
	    EV<<"Corrected distance to 5"<<endl;
    	    chState = curChannelSim->calculateLoss(5, globalSpeed);
    	    chState.speed = rHm.speed;
    	    chState.distance = 5;
	}
    }
    else {
        EV<<"Using lite-featured OFDMChannelSim..."<<endl;
	if (sqrdistance>1.0) {
            chState = pathLossChannelSim->calculateLoss(sqrt(sqrdistance), globalSpeed);
	    chState.speed = rHm.speed;
	    chState.distance = sqrt(sqrdistance);
	}
	else {
	    EV<<"Corrected distance to 5"<<endl;
            chState = pathLossChannelSim->calculateLoss(5, globalSpeed);
	    chState.speed = rHm.speed;
	    chState.distance = 5;
	}
    }

    for (int i=0; i<48; i++) {
    	//EV<<"rcvdPower on subband "<<i<<" "<<transmissionPowerPerSubcarrier * chState.loss[i]<<"="<<transmissionPowerPerSubcarrier<<"*"<<chState.loss[i]<<endl;
    	chState.rcvdPower[i] = transmissionPowerPerSubcarrier * chState.loss[i];
	}
    
    double rcvdPowerMin = chState.rcvdPower[0];
    double rcvdPowerMax = chState.rcvdPower[0];
    for (int j=1; j<48; j++)
        {
       	      if (chState.rcvdPower[j] > rcvdPowerMax)
	      	  rcvdPowerMax = chState.rcvdPower[j];
	      if (chState.rcvdPower[j] < rcvdPowerMin)
	          rcvdPowerMin = chState.rcvdPower[j];
        }
    chState.rcvdPowerMin = rcvdPowerMin;
    chState.rcvdPowerMax = rcvdPowerMax;
    EV << "maximum received power "<<rcvdPowerMax<<endl;
    EV << "minimum received power "<<rcvdPowerMin<<endl;
    
    // store the receive power in the recvBuff
    recvBuff[frame] = chState;



    // if receive power is bigger than sensitivity
    // and currently not receiving another message

    // handle multiple frames with non-overlapping subbands
    
    /* 
     * AirFrame magic number identifies multiple frames sent on non-overlapping
     * subbands, only one magic number is valid (new magic number will lead to 
     * collision calculation mechanism)
     *
     * Frames with magic number set to 0 (standard value) are classic 802.11a 
     * frames transmitted over all OFDMA subbands
     */

    // set currentMagicNumber, if no one is set
    if (currentMagicNumber<0) {
    	currentMagicNumber=rcvdAirFrame->getMagicNumber(); 
	EV<<"magicNumber set to "<<currentMagicNumber<<endl;
    }
    else {
    	EV<<"magicNumber was set before (to "<<currentMagicNumber<<")"<<endl;
    }

    // standard 802.11a airframe received
    if (rcvdAirFrame->getMagicNumber()==0) 
    {
    EV<<"Receiving frame with magic number 0"<<endl;
    if (snrInfo.ptr == 0) {
        if(rcvdPowerMax >= sensitivity) {
            EV <<"with maximum rcvdPower: "<<rcvdPowerMax<<" sensitivity: "<<sensitivity<<endl;

           // Put frame and related SnrList in receive buffer
            snrInfo.ptr = frame;
            snrInfo.chState = chState;
            snrInfo.sList.clear();
    
        // add initial snr value
            EV <<"first frame arrived!\n";
	    EV <<"frame is from "<<rcvdMacFrame->getSrcAddr()<<endl;
            addNewSnr();
            rssi.setRSSI(noiseLevel + rcvdPowerMax); // RSSI based on one subband
            if(radioState == RadioState::RECV) bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
        else {
            //add receive power to the noise level
            EV <<"frame discarded -- just noise with rcvdPower: "<<rcvdPowerMin<<" sensitivity: "<<sensitivity<<endl;
            noiseLevel += rcvdPowerMax;
            //rssi.setRSSI(rssi.getRSSI() + noiseLevel);
            rssi.setRSSI(noiseLevel);
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
      }
    // if a message is beeing received add a new snr value
    else {
        // update snr info for currently beeing received message
	EV <<"started receiving additional frame, treated as noise"<<endl;
        EV <<"add new snr value to snr list of message beeing received\n";
	EV <<"frame is from "<<rcvdMacFrame->getSrcAddr()<<endl;
        noiseLevel += rcvdPowerMax;
        rssi.setRSSI(noiseLevel);
        addNewSnr();
        if(publishRSSIAlways && (radioState == RadioState::RECV)) {
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
      }
    } // OFDMA extended airframe received
    else
    {
    // check whether frame is for me, ignore if not
    // store frame for me, like standard frame
    // add to noise if frame has new magic number
    if ((snrInfo.ptr == 0) && (rcvdMacFrame->getDestAddr()==myMacAddr) ) { //&& (currentMagicNumber==rcvdAirFrame->getMagicNumber())) { // copied from above
        if(rcvdPowerMax >= sensitivity) {
	    EV <<"Multimode frame for me received"<<endl;
            EV <<"with maximum rcvdPower: "<<rcvdPowerMax<<" sensitivity: "<<sensitivity<<endl;

           // Put frame and related SnrList in receive buffer
            snrInfo.ptr = frame;
            snrInfo.chState = chState;
            snrInfo.sList.clear();
    
        // add initial snr value
            EV <<"first frame arrived!\n";
            addNewSnr();
            rssi.setRSSI(rssi.getRSSI() + rcvdPowerMax); // TODO: RSSI subband based?
            if(radioState == RadioState::RECV) bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
    /* 
     * frames with magic number > 0 but not for me are ignored
     * TODO argue why this is ok
     *
     * else {
     *      //add receive power to the noise level
     *      EV <<"frame discarded -- just noise with rcvdPower: "<<rcvdPowerMin<<" sensitivity: "<<sensitivity<<endl;
     *      noiseLevel += rcvdPowerMin;
     *      rssi.setRSSI(rssi.getRSSI() + noiseLevel);
     *      bb->publishBBItem(catRSSI, &rssi, nicModuleId);
     *  } 
     */
      }
    // if a message is beeing received add a new snr value
    else if (currentMagicNumber!=rcvdAirFrame->getMagicNumber()) {
        // update snr info for currently beeing received message
        EV <<"add new snr value to snr list of message beeing received\n";
        noiseLevel += rcvdPowerMax;
        rssi.setRSSI(noiseLevel);
        addNewSnr();
        if(publishRSSIAlways && (radioState == RadioState::RECV)) {
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
        }
	else {
	    EV<<"Frame with current magic number to another STA ignored"<<endl;
	    rssi.setRSSI(noiseLevel+rcvdPowerMax);
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
	}
      } // end copied from above
      else {
      	EV<<"Ignored multimode frame with currentMagicNumber but not for me"<<endl;
	// update RSSI to inform MAC if medium is busy
	rssi.setRSSI(noiseLevel+rcvdPowerMax);
        bb->publishBBItem(catRSSI, &rssi, nicModuleId);
	ignoreFrame=rcvdAirFrame->getMagicNumber();
	ignoreCount++;
      }
    }
    EV <<"with minimum rcvdPower: "<<rcvdPowerMin<<endl;
    EV<<" rssi: "<< rssi.getRSSI()<<endl;
    EV<<" noiseLevel: "<< noiseLevel<<endl;
    EV<<"frame finished at "<<simTime()+frame->getDuration()<<endl;
}

void SnrEvalOFDM::takeChannelSample() {
    ChannelState chState;
    // get new sample
    chState = channels[sampleTimer->getSenderMacAddr()]->calculateLoss(sampleTimer->getDistance(),sampleTimer->getSpeed());
    // save speed and distance
    chState.speed = sampleTimer->getSpeed();
    chState.distance = sampleTimer->getDistance();
    // calculate received power per subband
    for (int i=0; i<48; i++) {
    	chState.rcvdPower[i] = transmissionPowerPerSubcarrier * chState.loss[i];
	}
    double rcvdPowerMin = chState.rcvdPower[0];
    double rcvdPowerMax = chState.rcvdPower[0];
    for (int j=1; j<48; j++)
        {
       	      if (chState.rcvdPower[j] > rcvdPowerMax)
	      	  rcvdPowerMax = chState.rcvdPower[j];
	      if (chState.rcvdPower[j] < rcvdPowerMin)
	          rcvdPowerMin = chState.rcvdPower[j];
        }
    chState.rcvdPowerMin = rcvdPowerMin;
    chState.rcvdPowerMax = rcvdPowerMax;

    // add new SINR value
    snrInfo.chState = chState;
    addNewSnr();
    if (sampleTimer->getTimeLeft()>sampleTimer->getSampleInterval()) {
    	sampleTimer->setTimeLeft(sampleTimer->getTimeLeft()-sampleTimer->getSampleInterval());
    	scheduleAt(simTime()+sampleTimer->getSampleInterval(),sampleTimer);
	}
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
void SnrEvalOFDM::handleLowerMsgEnd(AirFrame *frame)
{
    AirFrame80211* rcvdAirFrame = static_cast<AirFrame80211*>(frame);
    EV<<"handleLowerMsgEnd() called"<<endl;
//    if (static_cast<AirFrame80211*>(frame)->getMagicNumber()==currentMagicNumber) {
    // TODO fix "reception of noise message" bug
    if (snrInfo.ptr == frame) {    
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
            currentMagicNumber=-1;
	    EV <<"reset magicNumber"<<endl;
            snrInfo.sList.clear();
            bb->publishBBItem(catRSSI, &rssi, nicModuleId);
            EV <<"packet sent to the decider\n";      
        }
        else {
            // delete the pointer to indicate that no message is currently
            // beeing received and clear the list
            snrInfo.ptr = NULL;
            currentMagicNumber=-1;
	    EV <<"reset magicNumber"<<endl;
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
    	if ((ignoreFrame!=rcvdAirFrame->getMagicNumber()) || (ignoreCount<=0)) {
        EV <<"reception of noise message over, removing recvdPower from noiseLevel....\n";
      
        // get the rcvdPower and subtract it from the noiseLevel
        cRecvBuff::iterator it = recvBuff.find(frame);
        noiseLevel -= it->second.rcvdPowerMax;
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
	    currentMagicNumber=-1;
	    EV <<"reset magicNumber"<<endl;
        }
	}
	else {
	    EV<<"ignoring previously ignored frame end"<<endl;
	    ignoreCount--;
	    if (ignoreCount<=0) {
                rssi.setRSSI(noiseLevel);
		ignoreFrame=-1;
                if(radioState == RadioState::RECV) {
                    bb->publishBBItem(catRSSI, &rssi, nicModuleId);
                }
	    }
	}
	// message should be deleted
        delete frame;
        EV <<"message deleted\n";
    }
    EV<<"noiseLevel "<<noiseLevel<<" rssi: "<<rssi.getRSSI()<<endl;
//    }
//    else {
//    	EV<<"frame with wrong magic number ignored"<<endl;
//	EV<<"current magic number is: "<<currentMagicNumber<<", magic number of frame is "<<static_cast<AirFrame80211*>(frame)->getMagicNumber()<<endl;
//	delete frame; // ???
//    }
}

/**
 * The SnrEval has subscreibed the NewRadioState and will be informed each time the
 * radio state changes. The time radio switched to RECV must be noted.
 **/

void SnrEvalOFDM::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
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
void SnrEvalOFDM::addNewSnr()
{
    //print("NoiseLevel: "<<noiseLevel<<" recvPower: "<<snrInfo.rcvdPower);

    snrInfo.sList.push_back(OFDMSnrListEntry());
    snrInfo.sList.back().time=simTime();
    EV<<"Calculating SINR for each subband..."<<endl;
    for (int i=0;i<48;i++)
        {
	    //EV<<"SINR for subband "<<i<<": "<<snrInfo.chState.rcvdPower[i]/noiseLevel<<"="<<snrInfo.chState.rcvdPower[i]<<"/"<<noiseLevel<<endl;
            snrInfo.sList.back().snr[i]=snrInfo.chState.rcvdPower[i]/noiseLevel;
	}
    ev<<"New Snr added: "<<snrInfo.sList.back().snr[0]
      <<" at time: "<<snrInfo.sList.back().time<<endl;
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
void SnrEvalOFDM::modifySnrList(OFDMSnrList& list)
{
    for(OFDMSnrList::iterator iter=list.begin(); iter!=list.end();iter++) {
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
double SnrEvalOFDM::calcPathloss(AirFrame* frame)
{
    double time;
    double sqrdistance;
    double pRecv;
    double pSend = frame->getPSend();
    
       
    pRecv = -1000; // TF // usage of calcPathLoss will result in frame loss!
    	    //(pSend*waveLength*waveLength /
            // (16.0*M_PI*M_PI*pow(sqrdistance,pathLossAlphaHalf)));
    error("calcPathloss is outdated!");
    return (pRecv < pSend) ? pRecv : pSend;
}

void SnrEvalOFDM::finish()
{
    // delete channel sim objects
    ChannelMap::const_iterator it;
    for (it = channels.begin(); it != channels.end(); ++it)
    	delete it->second;
    delete pathLossChannelSim;
    BasicSnrEval::finish();
}

void SnrEvalOFDM::sendUp(AirFrame *msg, const OFDMSnrList& list)
{
    // create ControlInfo
    OFDMSnrControlInfo *cInfo = new OFDMSnrControlInfo;
    // attach the list to cInfo
    cInfo->setSnrList(list);
    // attach the cInfo to the AirFrame
    msg->setControlInfo(cInfo);
    send(static_cast<cMessage *>(msg), uppergateOut);
}


//const double SnrEvalOFDM::speedOfLight = 300000000.0;


