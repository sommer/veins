/***************************************************************************
 * file:        Decider80211a.cc
 *
 * authors:     Thomas Freitag, David Raguin / Marc Loebbers
 *
 * extended by: Thomas Freitag
 * status:	finished, unchecked
 * remarks:	searches for minimum SINR value, computes if value is below threshold and 
 * 		returns a bit error, collisions are detected like in TKN implementation
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


#include "Mac80211Pkt_m.h"
#include "Decider80211a.h"
#include "Consts80211a.h"
//#include <FWMath.h>

Define_Module(Decider80211a);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicDecider.
 *
 * This decider also needs the bitrate and some 802.11a parameters are
 * initialized
 *
 * Extended for 802.11a OFDM PHY support // TF
 */
void Decider80211a::initialize(int stage)
{
    BasicLayer::initialize(stage);

    double subbands;
    double pilots;

    if (stage == 0) {
    	ccaThreshold = FWMath::dBm2mW(par("ccaThreshold"));
    	snirThreshold[0] = FWMath::dBm2mW(par("ThresholdMode0"));
    	snirThreshold[1] = FWMath::dBm2mW(par("ThresholdMode1"));
    	snirThreshold[2] = FWMath::dBm2mW(par("ThresholdMode2"));
    	snirThreshold[3] = FWMath::dBm2mW(par("ThresholdMode3"));
    	snirThreshold[4] = FWMath::dBm2mW(par("ThresholdMode4"));
    	snirThreshold[5] = FWMath::dBm2mW(par("ThresholdMode5"));
    	snirThreshold[6] = FWMath::dBm2mW(par("ThresholdMode6"));
    	snirThreshold[7] = FWMath::dBm2mW(par("ThresholdMode7"));
        // old // TF snirThreshold = FWMath::dBm2mW(par("snirThreshold"));
	WATCH(snirThreshold);
	WATCH(ccaThreshold);

        dot11aCodeRates[0] = 0.5; // BPSK 1/2
        dot11aCodeRates[1] = 0.75; // BPSK 3/4
        dot11aCodeRates[2] = 0.5; // QPSK 1/2
        dot11aCodeRates[3] = 0.75; // QPSK 3/4
        dot11aCodeRates[4] = 0.5; // 16-QAM 1/2
        dot11aCodeRates[5] = 0.75; // 16-QAM 3/4
        dot11aCodeRates[6] = 2./3.; // 64-QAM 2/3
        dot11aCodeRates[7] = 0.75; // 64-QM 3/4

	DF = double (par("df"));
	subbands = par("subbands");
	pilots = par("pilots");

        //transmissionPower = par("transmissionPower");
        transmissionPower = FWMath::dBm2mW(par("transmissionPower"));
        //transmissionPowerPerSubcarrier = transmissionPower - 10*log10(subbands+pilots);
        transmissionPowerPerSubcarrier = transmissionPower/(subbands+pilots);
        //whiteGaussianNoise = par("whiteGaussianNoise");
        whiteGaussianNoise = FWMath::dBm2mW(par("whiteGaussianNoise"));
        //whiteGaussianNoisePerSubcarrier = whiteGaussianNoise - 10*log10(subbands+pilots);
        whiteGaussianNoisePerSubcarrier = whiteGaussianNoise/(subbands+pilots);
        symbolTime = par("symbolTime");
        symbolRate = 1/symbolTime;
	myMacAddr = parentModule()->id();
    }
}

/**
 * Handle message from lower layer. The minimal snir is read in and
 * it is computed wether the packet has collided or has bit errors or
 * was received correctly. The corresponding kind is set and it is
 * handed on to the upper layer.
 *
 */
void Decider80211a::handleLowerMsg(cMessage* m)
{
    double snirMin;
    double rxDuration;
    int	mode;	// we need the actual mode to decide if frame is received correctly // TF
    bool allOk = false;
    
    AirFrame80211 *af = static_cast<AirFrame80211 *>(m);
    OFDMSnrControlInfo *cInfo = static_cast<OFDMSnrControlInfo*>(af->removeControlInfo());
    OFDMSnrListEntry pEntry;
    PhyControlInfo *p;
    const OFDMSnrList& rList = cInfo->getSnrList();
    OFDMSnrList::const_iterator iter = rList.begin();
    Mac80211Pkt *mac;
    
    rxDuration = simTime() - iter->time;
    mode = af->getMode();

    snirMin = iter->snr[0];
    pEntry = *iter;
    for(; iter != rList.end(); iter++) {
    	int i=0;
    	for (;i<48;i++) {
        	if (iter->snr[i] < snirMin) snirMin = iter->snr[i];
	}
    }
    
    EV << " snrMin: " << snirMin << ", ccaThreshold: " << ccaThreshold << endl;
    
    //if snir is big enough so that packet can be recognized at all
    if((snirMin > ccaThreshold) && (rxDuration > af->getDuration() - RED_PHY_HEADER_DURATION))
    {	
	if (af->getMagicNumber() == 0) {
            if (packetOk(snirMin, af->getMode())) {
                EV << "packet was received correctly, it is now handed to upper layer...\n";
                mac = static_cast<Mac80211Pkt *>(af->decapsulate());
	        p = new PhyControlInfo(af->getBitrate(), snirMin);
	        p->setOFDMState(pEntry);
                mac->setControlInfo(p);
                sendUp(mac);
                delete af;
             }
             else {
                 EV << "Packet has BIT ERRORS! It is lost! Mode was: " <<af->getMode()<<endl;
                 af->setName("ERROR");
                 af->setKind(BITERROR);
                 sendControlUp(af);
             }
	}
	else {
	    allOk = true;
            pEntry = *iter;
            for(; iter != rList.end(); iter++) {
    	        for (int i=0;i<48;i++) {
		    if (af->getAssignment().entry[i].dest==myMacAddr);
        	        if (!packetOk(iter->snr[i],af->getAssignment().entry[i].mode)) 
		     	    allOk=false;
         
	        }
            }
	   if (allOk) {
               EV << "Multimode frame was received correctly, it is now handed to upper layer...\n";
               mac = static_cast<Mac80211Pkt *>(af->decapsulate());
	       p = new PhyControlInfo(af->getBitrate(), snirMin);
	       p->setOFDMState(pEntry);
               mac->setControlInfo(p);
               sendUp(mac);
               delete af;
	   }
	   else {
               EV << "Frame has BIT ERRORS! It is lost! It was a multimode frame!"<<endl; 
               af->setName("ERROR");
               af->setKind(BITERROR);
               sendControlUp(af);
	   }
	}
    }
    else {
        ev << "COLLISION! Packet got lost: rxDuration " << rxDuration << ", frame duration " << af->getDuration() - RED_PHY_HEADER_DURATION << ", minimum SINR: " << snirMin << ", ccaThreshold: " << ccaThreshold << endl;
        af->setName("COLLISION");
        af->setKind(COLLISION);
        sendControlUp(af);
    }
    delete cInfo;
}


bool Decider80211a::packetOk(double snirMin, int dot11aMode)
{
	EV << "packetOk? snirMin: "<<snirMin<<", coding gain: "<< calcCodingGain(dot11aCodeRates[dot11aMode]) << " threshold: "<< getThreshold(dot11aMode)<< " snrMin*Ts: "<<snirMin*symbolTime<< " Ts: "<<symbolTime<<endl;
	EV<<snirMin<<"*"<<symbolTime<<"*"<<calcCodingGain(dot11aCodeRates[dot11aMode])<<"="<<snirMin*symbolTime*calcCodingGain(dot11aCodeRates[dot11aMode])<<">"<<getThreshold(dot11aMode)<<endl;
	return (snirMin*symbolTime*calcCodingGain(dot11aCodeRates[dot11aMode])>getThreshold(dot11aMode));	
}

double Decider80211a::calcCodingGain(double codeRate)
{
    //return (10.0*log10(codeRate*DF)); // this results in maximum coding gain
    //return codeRate*DF; // this results in maximum coding gain
    //
    // using coding gains from Proakis
    if (codeRate<2./3.)
    	return 5.1; // code rate 1/2
    else if (codeRate<0.75)
    	return 4.6; // code rate 2/3
    else
    	return 4.2; // code rate 3/4
}

/*!
 * Returns threshold of a 802.11a modes (0-7)
 * thresholds provided by Stefan Valentin
 */
double Decider80211a::getThreshold(int mode)
{
	if ((0 <= mode) && (mode <8)) {
		//return 10*log10(snirThreshold[mode]);
		return snirThreshold[mode];
	} else { 
		return 9.9999e+20; // return unreachable threshold, if incorrect mode is given
	}
		
}


