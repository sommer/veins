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
 *	file:		$RCSfile: SnrEvalOFDM.h,v $
 *
 *      last modified:	$Date: 2007/03/18 17:03:29 $
 *      by:		$Author: tf $
 *
 *      informatin:	-
 *
 *	changelog:   	$Revision: 1.9 $
 *			$Log: SnrEvalOFDM.h,v $
 *			Revision 1.9  2007/03/18 17:03:29  tf
 *			- implemented takeChannelSample
 *			- timer is scheduled in handleLowerMsgStart()
 *			- TODO: estimate channel coherence time
 *			
 *			Revision 1.8  2007/02/22 09:56:43  tf
 *			- fixed noise message reception bug
 *			
 *			Revision 1.7  2007/02/13 12:51:53  tf
 *			- using parameter globalSpeed for setting speed of terminals
 *			
 *			Revision 1.6  2007/01/31 17:13:25  tf
 *			- added OFDMChannelSim instance for pathloss calculation
 *			
 *			Revision 1.5  2007/01/24 13:04:59  tf
 *			- added
 *				myMacAddr
 *				currentMagicNumber
 *			  for OFDMA AirFrame management
 *			
 *			Revision 1.4  2007/01/17 17:01:47  tf
 *			- bugs in error calculation fixed
 *			
 *			Revision 1.3  2007/01/15 16:22:37  tf
 *			- added channel sim parameters for initialization
 *			- fully support of 48 OFDM subbands for rssi and channel state now
 *			
 */

#ifndef SNR_EVAL_OFDM_H
#define SNR_EVAL_OFDM_H

#include <BasicSnrEval.h>
#include "RadioState.h"
#include "RSSI.h"

#include <OFDMChannelSim.h>
#include <ChannelControl.h>
#include <SampleChannelMsg_m.h>

#include <list>
#include <map>

/**
 * @brief Keeps track of the different snir levels when receiving a
 * packet
 *
 * This module keeps track of the noise level of the channel.
 *
 * When receiving a packet this module updates the noise level of the
 * channel. Based on the receive power of the packet it is processed
 * and handed to upper layers or just treated as noise.
 *
 * After the packet is completely received the snir information is
 * attached and it is handed to the deceider module. The snir 
 * information may be manipulated if the module received the 
 * frame before the radio was in received mode. 
 * The snir information is a SnrList that lists all different snr
 * levels together with the point of time starting from which the radio 
 * is switched in the received mode. 
 *
 * On top of that this module subscribs the RadioState on the
 * Blackboard. The radio state gives information about whether this
 * module is sending a packet, receiving a packet or idle. This
 * information can be accessed via the Blackboard by other modules,
 * e.g. a CSMAMacLayer.
 *
 * @author Marc Loebbers, Jamarin Phongcharoen, Andreas Koepke
 * @ingroup snrEval
 **/

class SnrEvalOFDM : public BasicSnrEval
{
    Module_Class_Members( SnrEvalOFDM, BasicSnrEval, 0 );

public:
    /** @brief Initialize variables and publish the radio status*/
    virtual void initialize(int);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
    
    /** @brief alternative sendUp for OFDMSnrList cInfo attachments */
    virtual void sendUp(AirFrame *msg, const OFDMSnrList& list);

protected:
    
    /** @brief Buffer the frame and update noise levels and snr information...*/
    virtual void handleLowerMsgStart(AirFrame*);

    /** @brief Unbuffer the frame and update noise levels and snr information*/
    virtual void handleLowerMsgEnd(AirFrame*);

    /** get additional channel sample */
    virtual void takeChannelSample();
    
    /** @brief Calculates the power with which a packet is received.
     * Overwrite this function to call all required loss elements, by
     * default only pathloss is calculated. Fading, shadowing, antenna
     * pattern are not evaluated in this class.
     */
    virtual double calcRcvdPower(AirFrame* frame) {
        return calcPathloss(frame);
    }

    /** @brief Calculate the path loss.
     */
    double calcPathloss(AirFrame* frame);

    /** @brief updates the snr information of the relevant AirFrames
     *  called in handleLowerMsgStart(AirFrame*)
     */
    void addNewSnr();

    void handlePublish(RSSI *r);
  
    /** @brief updates the snr information of the relevant AirFrame
     *  called in handleLowerMsgEnd(AirFrame*)
     */
    void modifySnrList(OFDMSnrList& list);

    /** @brief Calculate duration of this message */
    virtual double calcDuration(cMessage* m) {
        return static_cast<double>(m->length()) / bitrate;
    }

    /** @brief module destructor */
    virtual void finish();
    
    
protected:
    /**
     * @brief Struct to store a pointer to the mesage, rcvdPower AND a
     * SnrList, needed in SnrEval::addNewSnr
     **/
    struct SnrStruct {
        /** @brief Pointer to the message this information belongs to*/
        AirFrame* ptr;
        /** @brief Received power of the message*/
        ChannelState chState;
        /** @brief Snr list to store the SNR values*/
        OFDMSnrList sList;
    };
    /**
     * @brief SnrInfo stores the snrList and the the recvdPower for the
     * message currently beeing received together with a pointer to the
     * message.
     **/
    SnrStruct snrInfo;

    /**
     * @brief Typedef used to store received messages together with
     * receive power.
     **/
    typedef std::map<AirFrame*,ChannelState> cRecvBuff;

    /**
     * @brief A buffer to store a pointer to a message and the related
     * receive power.
     **/
    cRecvBuff recvBuff;

    /** @brief Current state of active channel (radio), set using radio, updated via BB */
    RadioState::States radioState;
    /** @brief category number given by bb for RadioState */
    int catRadioState;
    
    /** @brief Last RSSI level */
    RSSI rssi;
    /** @brief category number given by bb for RSSI */
    int catRSSI;
    /** @brief if false, the RSSI will not be published during the reception of a frame
     *
     *  Set it to false if you want a small speed up in the simulation. 
     */
    bool publishRSSIAlways;
    
    /** @brief Cache the module ID of the NIC */
    int nicModuleId;
    
    /** @brief The noise level of the channel.*/
    double noiseLevel;

    /** @brief Used to store the time a radio switched to recieve.*/
    double recvTime;
    
    /** 
     * @brief Thermal noise on the channel. Can be specified in
     * omnetpp.ini. Default: -100 dBm
     **/
    double thermalNoise;

    double globalSpeed;

    /** @brief do we send on the surface of a torus */
    bool useTorus;
    
    /** @brief then we need to know the edges of the playground */
    Coord playground;

    /** @brief keep bitrate to calculate duration */
    double bitrate;
    /** @brief BB category of bitrate */
    int catBitrate;

    /** @brief channel state calculation object */
    ChannelMap channels;


  private:
    // magic number to identify OFDMA multi frames
    int currentMagicNumber;

    OFDMChannelSim* pathLossChannelSim;

    // channelSim parameters
    double LIGHTSPEED;

    // for path loss
    double TENLOGK;
    double ALPHA;

    // for shading loss
    double MEAN;
    double STD_DEV;

    // for fading loss
    double DELAY_RMS;               // Mean Delay Spread
    double FREQUENCY_SPACING;       // frequency sample spacing
    int FADING_PATHS;               // number of different simulated fading paths
    double CENTER_FREQUENCY;           // center frequency
    int SUBBANDS;
    
    int CALCULATE_PATH_LOSS;
    int CALCULATE_SHADOWING;
    int CALCULATE_FADING;

    int CORRELATED_SUBBANDS;

    double transmissionPower;
    double transmissionPowerPerSubcarrier;
    double whiteGaussianNoise;
    double whiteGaussianNoisePerSubcarrier;
    double symbolRate;
    double logarithmicSymbolRate;
    double symbolTime;
    double logarithmicSymbolTime;

    int myMacAddr;

    int ignoreFrame;
    int ignoreCount;
};

#endif
