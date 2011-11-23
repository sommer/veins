/*
 *  LMACLayer.cc
 *  LMAC for MF 2.02, omnetpp 3.4
 *
 *  Created by Anna Förster on 10/10/08.
 *  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
 *
 *  Converted to MiXiM by Kapourniotis Theodoros
 *
 */

#ifndef LMAC_LAYER_H
#define LMAC_LAYER_H

#include <list>

#include "MiXiMDefs.h"
#include "DroppedPacket.h"
#include "BaseMacLayer.h"
#include "PhyUtils.h"
#include "SimpleAddress.h"

class LMacPkt;

/**
 * @brief Implementation of L-MAC (Lightweight Medium Access Protocol for
 * Wireless Sensor Networks [van Hoesel 04] ).
 *
 * Each node has its own unique timeslot. Nodes can use their timeslots to
 * transfer data without having to contend for the medium or having to deal
 * with energy wasting collisions or retransmissions.
 *
 * During the first 5 full frames nodes will be waking up every controlDuration
 * to setup the network first by assigning a different timeslot to each node.
 * Normal packets will be queued, but will be send only after the setup phase.
 *
 * During its timeslot a node wakes up, checks the channel for a short random
 * period (CCA) to check for possible collision in the slot and, if the
 * channel is free, sends a control packet. If there is a collision it tries
 * to change its timeslot to an empty one. After the transmission of the control
 * packet it checks its packet queue and if its non-empty it sends a data
 * packet.
 *
 * During a foreign timeslot a node wakes up, checks the channel for
 * 2*controlDuration period for an incoming control packet and if there in
 * nothing it goes back to sleep and conserves energy for the rest of the
 * timeslot. If it receives a control packet addressed for itself it stays awake
 * for the rest of the timeslot to receive the incoming data packet.
 *
 * The finite state machine of the protocol is given in the below figure:
 *
 * \image html LMACFSM.jpg "State chart for LMAC layer"
 *
 * A paper describing the protocol is:
 *
 * L. van Hoesel and P. Havinga. A lightweight medium access
 * protocol (LMAC) for wireless sensor networks. In Proceedings of the 3rd
 * International Symposium on Information Processing in Sensor Networks (IPSN),
 * pages 55-60, Berkeley, CA, February 2004. April.
 *
 * @ingroup macLayer
 **/
class MIXIM_API  LMacLayer : public BaseMacLayer
{
  public:
	/** @brief Clean up messges.*/
	virtual ~LMacLayer();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual MacPkt* encapsMsg(cMessage*);
	

  protected:
    typedef std::list<LMacPkt*> MacQueue;
    
    /** @brief MAC states
     *
     *  The MAC states help to keep track what the MAC is actually
     *  trying to do -- this is esp. useful when radio switching takes
     *  some time.
	 *  SLEEP -- the node sleeps but accepts packets from the network layer
     *  RX  -- MAC accepts packets from PHY layer
     *  TX  -- MAC transmits a packet
     *  CCA -- Clear Channel Assessment - MAC checks
     *         whether medium is busy
     */
    
    enum States {
    	INIT,
		SLEEP,
        CCA,
        WAIT_CONTROL,
        WAIT_DATA,
        SEND_CONTROL,
        SEND_DATA
    };
	
	enum TYPES {
		LMAC_CONTROL=167,
		LMAC_TIMEOUT=168,
		LMAC_WAKEUP=169,
		LMAC_SEND_DATA=170,
		LMAC_SETUP_PHASE_END=171,
		LMAC_CHECK_CHANNEL=172,
		LMAC_SOMEBODY=173,
		LMAC_DATA=174,
		LMAC_START_LMAC=175,
		LMAC_SEND_CONTROL=176
	};
	
	/** @brief dummy receiver address to indicate no pending packets in the control packet */
	static const LAddress::L2Type LMAC_NO_RECEIVER;
	static const LAddress::L2Type LMAC_FREE_SLOT;
	
	/** @brief the setup phase is the beginning of the simulation, where only control packets at very small slot durations are exchanged. */
	bool SETUP_PHASE;
	
	/** @brief indicate how often the node needs to change its slot because of collisions */
	cOutVector* slotChange;
	
    /** @brief keep track of MAC state */
    States macState;
    
    /** @brief Current state of active channel (radio), set using radio, updated via BB */
    Radio::RadioState radioState;
	
    /** @brief category number given by bb for RadioState */
    int catRadioState;
	
	/** @brief track and publish current occupation state of medium */
    int catIndication;


    /** @brief Duration of a slot */
    double slotDuration;
	/** @brief Duration of teh control time in each slot */
	double controlDuration;
	/** @brief my slot ID */
	int mySlot;
	/** @brief how many slots are there */
	int numSlots;
	/** @brief The current slot of the simulation */
	int currSlot;
	/** @brief Occupied slots from nodes, from which I hear directly */
	LAddress::L2Type occSlotsDirect[64];
	/** @brief Occupied slots of two-hop neighbors */
	LAddress::L2Type occSlotsAway[64];
	/** @brief The first couple of slots are reserved for nodes with special needs to avoid changing slots for them (mobile nodes) */
	int reservedMobileSlots;

    
    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    MacQueue macQueue;
    
    /** @brief length of the queue*/
    unsigned queueLength;
    
	cMessage* wakeup;
	cMessage* timeout;
	cMessage* sendData;
	cMessage* initChecker;
	cMessage* checkChannel;
	cMessage* start_lmac;
	cMessage* send_control;
    
    /** @brief the bit rate at which we transmit */
    double bitrate;

	/** @brief find a new slot */
	void findNewSlot();
	
    /** @brief Inspect reasons for dropped packets */
    DroppedPacket droppedPacket;
    
    /** @brief plus category from BB */
    int catDroppedPacket;
    
    /** @brief publish dropped packets nic wide */
    int nicId;
    
	/** @brief Internal function to attach a signal to the packet */
	void attachSignal(MacPkt *macPkt);

	/** @brief Transmission power of the node */
	double txPower;


};

#endif

