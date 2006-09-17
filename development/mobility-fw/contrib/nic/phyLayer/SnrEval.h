/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval.h
 *
 * copyright:   (C) 2004-6 Telecommunication Networks Group (TKN) at
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


#ifndef SNR_EVAL_H
#define SNR_EVAL_H

#include <BasicSnrEval.h>
#include "RadioState.h"
#include "RSSI.h"

#include <list>
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

class SnrEval : public BasicSnrEval
{
    Module_Class_Members( SnrEval, BasicSnrEval, 0 );

public:
    /** @brief Initialize variables and publish the radio status*/
    virtual void initialize(int);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

protected:
    
    /** @brief Buffer the frame and update noise levels and snr information...*/
    virtual void handleLowerMsgStart(AirFrame*);

    /** @brief Unbuffer the frame and update noise levels and snr information*/
    virtual void handleLowerMsgEnd(AirFrame*);

    
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
    void modifySnrList(SnrList& list);

    /** @brief Calculate duration of this message */
    virtual double calcDuration(cMessage* m) {
        return static_cast<double>(m->length()) / bitrate;
    }
    
    
protected:
    /**
     * @brief Struct to store a pointer to the mesage, rcvdPower AND a
     * SnrList, needed in SnrEval::addNewSnr
     **/
    struct SnrStruct{
        /** @brief Pointer to the message this information belongs to*/
        AirFrame* ptr;
        /** @brief Received power of the message*/
        double rcvdPower;
        /** @brief Snr list to store the SNR values*/
        SnrList sList;
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
    typedef std::map<AirFrame*,double> cRecvBuff;

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
     * @brief The wavelength. Calculated from the carrier frequency specified in the omnetpp.ini
     * The carrier frequency used. Can be specified in the
     * omnetpp.ini file. If not present it is read from the ChannelControl
     * module.
     **/
    double waveLength;
  
    /** 
     * @brief Thermal noise on the channel. Can be specified in
     * omnetpp.ini. Default: -100 dBm
     **/
    double thermalNoise;

    /**
     * @brief Path loss coefficient.
     * 
     * Can be specified in omnetpp.ini. If not it is read from the
     * ChannelControl module. This value CANNOT be smaller than the
     * one specified in the ChannelControl module. The simulation will
     * exit with an error!  The stored value is smaller to enable
     * faster calculation using the square of the distance.
     * 
     **/
    double pathLossAlphaHalf;
    
    /** @brief Speed of light */
    static const double speedOfLight;

    /** @brief do we send on the surface of a torus */
    bool useTorus;
    
    /** @brief then we need to know the edges of the playground */
    Coord playground;

    /** @brief keep bitrate to calculate duration */
    double bitrate;
    /** @brief BB category of bitrate */
    int catBitrate;
};

#endif
