
/* -*- mode:c++ -*- ********************************************************
 * file:        REDMacLayer.h
 *
 * author:      Jochen Adamek, Andreas Köpke
 *
 * copyright:   (C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
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
 
#ifndef RED_MAC_H
#define RED_MAC_H

#include <list>
#include <cstdlib> 
#include <ActiveChannel.h>
#include <RadioState.h>
#include <RSSI.h>
#include <Bitrate.h>
#include <DroppedPacket.h>
#include <BasicMacLayer.h>
#include <Blackboard.h>
#include <SingleChannelRadio.h>
#include <SimpleAddress.h>
#include <REDMacPkt_m.h>
#include <REDMacControlInfo.h>
#include "MediumIndication.h"

class REDMacLayer : public BasicMacLayer
{
//public:
//Module_Class_Members(REDMacLayer, BasicMacLayer, 0);
	
protected:

    /**  
     *  @brief MAC states
     *
     *  The MAC states help to keep track what the MAC is actually
     *  trying to do -- this is esp. useful when radio switching takes
     *  some time.
     *  RX  -- MAC accepts packets from PHY layer
     *  RX_ACK -- RX_Acknowledgements - MAC is ready 
     *         for receiving packets
     *  CCA -- Clear Channel Assessment - MAC checks
     *         whether medium is busy
     *  CCA_ACK -- CCA-Acknowledgements -
     *  RX_P -- rx mode done, receive packet
     *  RX_ACK_P -- tx mode done, transmit packet
     *  SLEEP -- the node sleeps, another node can start sending
     *  TX  -- MAC accepts transmiting a packet
     *  TX_ACK -- MAC is ready for transmitting packets
     *  INIT -- initial state
     *  STOP --
     */
     
        enum States {
        RX,
        RX_ACK, 
        CCA,
        CCA_ACK,
        RX_P,
        RX_ACK_P,
        SLEEP,
        TX,
        TX_ACK,
        INIT,
        STOP,
    };
    
    States macState;
    States action; 
     
    static const int BYTE_TIME;                 
    static const int PREAMBLE_BYTE_TIME;
    static const int PHY_HEADER_TIME;
    static const int TIME_CORRECTION;
    
    static const int SUB_HEADER_TIME;
    static const int SUB_FOOTER_TIME;
    
    static const int DEFAULT_SLEEP_TIME;
    static const int DATA_DETECT_TIME;
    static const int RX_SETUP_TIME;             // time to set up receiver
    static const int TX_SETUP_TIME;             // time to set up transmitter
    static const int RECEIVE_DONE_TIME;
    static const int ADDED_DELAY;
    static const int RX_ACK_TIMEOUT;
    static const int TX_GAP_TIME;
    static const int ACK_DURATION;
    static const int MAX_SHORT_RETRY;
    static const int MAX_LONG_RETRY;
    static const unsigned int MAX_AGE; 
    static const unsigned int MSG_TABLE_ENTRIES;
    static const int TOKEN_ACK_FLAG;
    static const int TOKEN_ACK_MASK;
 
    
    //static const int PREAMBLE_LONG;
    //static const int PREAMBLE_SHORT;
    
    static const int ZERO_BACKOFF_MASK; 
       
    enum Flags {
    	SWITCHING = 1,
    	RSSI_STABLE = 2,
    	UNHANDLED_PACKET = 4,
    	MESSAGE_PREPARED = 8,
    	RESUME_BACKOFF = 16,
    	CANCEL_SEND = 32,
    	ACTION_DETECTED = 64,
    	TEAMGEIST_ACTIVE = 128,
    }; 
    
    enum Error_t {
    	SUCCESS,
    	FAIL,
    	XCANCEL,
    	XBUSY,
    };
    	
   enum AckStatus {
   	    ACK_REQUESTED,
        NO_ACK_REQUESTED,
        WAS_NOT_ACKED,
        WAS_ACKED,
    };
   
   enum Addr {
   	    AM_BROADCAST_ADDR,
        RELIABLE_MCAST_MIN_ADDR,
   };
    	
    Error_t error;	
    	
    struct History {
    	int index;
    	States state;
    	int place;
    };
    
    struct KnownMsgTable {
    	int src;
    	int token;
    	unsigned age;
    };
    
   /* struct RInfo {
        double bitrate;
    
     double snr;
        double strength;
        int ack;
    
    };*/
    
    KnownMsgTable knownMsgTable[20];
    
      
    /** @brief Current state of active channel (radio), set using radio, updated via BB */
    RadioState::States radioState;
    
    /** @brief category number given by bb for RadioState */
    int catRadioState;
    
    /** @brief Last RSSI level */
    double rssi;
    
    /** @brief category number given by bb for RSSI */
    int catRSSI;

    /** @brief RSSI level where medium is considered busy */
    double busyRSSI;
    
    /** @brief cached pointer to radio module */
    SingleChannelRadio* radio;
    
    /** @brief pointer to the REDMacControlInfo, an extending class of the ControlInfo */
    REDMacControlInfo* cInfo;
    
    /** @brief track and publish current occupation state of medium */
    MediumIndication indication;
    
    MediumIndication mediumState;
    
    int catIndication;
    
    double snr;
    
    double maxPacketLength;
    
    /** @brief Inspect reasons for dropped packets */
    DroppedPacket droppedPacket;
    
    /** @brief plus category from BB */
    int catDroppedPacket;
    
    /** @brief publish dropped packets nic wide */
    int nicId;
    
    /** @brief the bit rate at which we transmit */
    double bitrate;
              
    int flags;
        
    double checkCounter;
    
    double repCounter;
    
    int longRetryCounter;
    
    int shortRetryCounter;
    
    int sleepTime;
    
    double rxTime;
    
    double sim_time;
    
    int restLaufzeit;
    
    int MIN_BACKOFF_MASK;
            
    int congestionLevel;
    
    int txLen;
    
    int seqNo;
    
    int teamgeistType;
        
    /** @brief Timer for backoff */
    cMessage* timer;
    
    cMessage* sampleTimer;
    
    cMessage* rssiStableTimer;
    
    cMessage* timeOut;
    
    REDMacPkt* txBufPtr;
    
    REDMacPkt* ackMsg;
    
void packetSendDone(REDMacPkt *pkt, Error_t error);

Error_t packetSend(REDMacPkt *txBufPtr);

REDMacPkt* receiveDone(REDMacPkt *pkt, int len, Error_t error);

void receiveDetected();

/****** Secure switching of radio modes ***/

void releaseAdcTask();
    
void requestAdc();

void setRxMode();

void setTxMode();

void setSleepMode();

void setSleepTime(int sT);

int getSleepTime();

void ageMsgsTask();

/****************  SplitControl  *****************/

void startDoneTask();

void stopDoneTask();

Error_t splitControlStop();

Error_t splitControlStart();

/****** Radio(Mode) events *************************/

void rssiStable();

void rxModeDone();

void txModeDone();

void sleepModeDone();

void checkSend();

int backoff(int counter);

bool needsAckTx(REDMacPkt *pkt);

bool needsAckRx(REDMacPkt *pkt);

void prepareMsgTask();

bool prepareRepetition();

void signalSendDone(Error_t error);

void updateRetryCounters();

void updateLongRetryCounters();

bool ackIsForMe(REDMacPkt *pkt);

void interruptBackoffTimer();

bool msgIsForMe(REDMacPkt *pkt);

bool isControl(REDMacPkt *pkt);

virtual void computeBackoff();

bool isNewMsg(REDMacPkt *pkt);

unsigned findOldest();

void rememberMsg(REDMacPkt *pkt);

void prepareAck(REDMacPkt *pkt);

double calcGeneratedTime(REDMacPkt *pkt);

void channelBusy();

void channelIdle();

void checkOnBusy();

void checkOnIdle();

void timerFired();

void sampleTimerFired();

void macReceiveDone(REDMacPkt *pkt);

/****** MacSend events *************************/

double macSend(cMessage *msg, int len);

void macSendDone(REDMacPkt *pkt, Error_t error);

double macCancel(REDMacPkt *pkt);

void updateNoiseFloor();

void updateNoiseFloorDone();

bool isOwner();

void setFlag(int *which, int pos);

void clearFlag(int *which, int pos);

bool isFlagSet(const int *which, int pos);

void channelMonitorStart();

/***** default Teamgeist events **************************/

int observedAMType(REDMacPkt *pkt);

bool tgNeedsAck(REDMacPkt *pkt, int src, int dest, double snr);

double estimateForwarders(REDMacPkt *pkt);

int getDestination(REDMacPkt *pkt, int retryCounter);

void gotAck(REDMacPkt *pkt, int acksender, double snrValue);

void congestionEvent(int level);

/*******************************************************/
  
/** @brief: method to change tic-values into seconds */  
double startOneShot(int t);   

double milliToDB(double db);

REDMacPkt* encapsMsg(cMessage *msg);  
    	
public:

Module_Class_Members(REDMacLayer, BasicMacLayer, 0);

virtual void initialize(int stage);

virtual void handleUpperMsg(cMessage *msg);

virtual void handleLowerMsg(REDMacPkt *pkt);

virtual void handleUpperControl(REDMacPkt *pkt);

virtual void handleSelfMsg(cMessage *msg);

virtual void handleLowerControl(cMessage *msg);

virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);   

virtual void finish();
};

#endif
