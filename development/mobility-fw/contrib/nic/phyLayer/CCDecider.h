/* -*- mode:c++ -*- *******************************************************
 * file:        Decider80211a.h
 *
 * authors:     Thomas Freitag, David Raguin / Marc Loebbers
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
 **************************************************************************/


#ifndef  DECIDER_80211A_H
#define  DECIDER_80211A_H

#include <omnetpp.h>

#include <BasicLayer.h>
#include <AirFrame80211_m.h>
#include "Consts80211a.h"
#include "PhyControlInfo.h"

/**
 * @brief Decider for the 802.11a modules
 *
 * Depending on the minimum of the snr included in the PhySDU this
 * module computes a bit error probability. The header (1 Mbit/s) is
 * always modulated with DBQPSK. The PDU is normally modulated either
 * with DBPSK (1 and 2 Mbit/s) or CCK (5.5 and 11 Mbit/s). CCK is not
 * easy to model, therefore it is modeled as DQPSK with a 16-QAM for
 * 5.5 Mbit/s and a 256-QAM for 11 Mbit/s.
 *
 *
 * @ingroup decider
 * @author Marc Löbbers, David Raguin
 */
class  Decider80211a : public BasicLayer
{
    Module_Class_Members(Decider80211a, BasicLayer,0);

public:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    
protected:
    /**
     * @brief Check whether we received the packet correctly.
     * Also appends the PhyControlInfo
     */
    virtual void  handleLowerMsg(cMessage *msg);

    /**
     * @brief This function should not be called.
     */
    virtual void handleUpperMsg(cMessage *msg) {
        error("Decider80211a does not handle messages from upper layers");
    }
    
    virtual void handleSelfMsg(cMessage *msg) {
        error("Decider80211a does not handle selfmessages");
    }
    
    /** @brief Handle control messages from lower layer
     *  Just cut them through
     */
    virtual void handleLowerControl(cMessage *msg) {
        sendControlUp(msg);
    };

    /** @brief converts a dB value into a normal fraction*/
    double dB2fraction(double);

    /** @brief computes if packet is ok or has errors, based on channel sim error calculation */
    bool packetOk(double snirMin, int dot11aMode);

    /** @brief calculates coding gain for 802.11a modes */
    double calcCodingGain(double codeRate);

    /** @brief returns decoding threshold for given mode*/
    double getThreshold(int mode);


    
#ifdef _WIN32
    /**
     * @brief Implementation of the error function
     *
     * Unfortunately the windows math library does not provide an
     * implementation of the error function, so we use an own
     * implementation (Thanks to Jirka Klaue)
     *
     * @author Jirka Klaue
     */
    double erfc(double);
#endif

 protected:
    /** @brief should be set in the omnetpp.ini*/
    double bitrate;
    /** @brief should be set in the omnetpp.ini; everthing below this
        threshold can't be read and is therefor considered as a
        collision*/
    double snirThreshold[8];

    /** @brief clear channel assignment */
    double ccaThreshold;

    /** @brief degree of freeness of convolutional encoder */
    double DF;

    /** @brief code rates used by 802.11a modes */
    double dot11aCodeRates[8];/* = { 0.5, // BPSK 1/2
                                        0.75, // BPSK 3/4
                                        0.5, // QPSK 1/2
                                        0.75, // QPSK 3/4
                                        0.5, // 16-QAM 1/2
                                        0.75, // 16-QAM 3/4
                                        2./3., // 64-QAM 2/3
                                        0.75 // 64-QM 3/4
                                        }; */


    double transmissionPower;
    double transmissionPowerPerSubcarrier;
    double whiteGaussianNoise;
    double whiteGaussianNoisePerSubcarrier;
    double symbolRate;
    double symbolTime;
    int myMacAddr;
};
#endif


