/* -*- mode:c++ -*- ********************************************************
 * file:        Mac80211a.h
 *
 * author:      Thomas Freitag, David Raguin/Marc Löbbers
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


#ifndef MAC_80211A_H
#define MAC_80211A_H

#include <list>
#include <map>
#include "BasicLayer.h"
#include "Mac80211Pkt_m.h"
#include "Consts80211a.h"
#include "RadioState.h"
#include "Bitrate.h"
#include <ActiveChannel.h>
#include "SingleChannelRadio.h"
#include <Blackboard.h>
#include <OFDMASpecials.h>
#include <AssignmentSpecials.h>
#include <BasicAssignmentMessage_m.h>
#include <SinkMessage_m.h>
#include <SinkSpecials.h>

enum BufferState { EMPTY, LOW, HIGH, FULL } ;

class BBBufferState : public BBItem 
{
	public:
		BBBufferState() { bs = EMPTY ; };
		BBBufferState(BufferState b) { bs = b; };
		BufferState getBufferState() { return bs; } ;
		void setBufferState(BufferState b) { bs = b; };
	protected:
		BufferState bs;
};

/**
 * @brief An implementation of the 802.11a MAC.
 *
 * For more info, see the NED file.
 *
 * @ingroup macLayer
 * @author David Raguin
 */
class  Mac80211a : public BasicLayer
{
protected:
    typedef std::list<Mac80211Pkt*> MacPktList;

    typedef std::map<int,MacPktList> MultiMacPktList;

    struct SendListEntry {
    	int eCts;
    	int bytesSent;
	int bytesAcked;
	int curFragNum;
	int maxFragNum;
	Mac80211Pkt* frame;
    };

    typedef std::map<int,SendListEntry> SendList;

    /** Definition of the timer types */
    enum timerType {
      TIMEOUT,
      NAV,
      CONTENTION,
      END_SIFS
    };

    /** Definition of the states*/
    enum State { // TODO additional states necessary? In combination with dest
      WFDATA = 0, // waiting for data packet
      QUIET = 1,  // waiting for the communication between two other nodes to end
      IDLE = 2,   // no packet to send, no packet receiving
      CONTEND = 3,// contention state (battle for the channel)
      WFCTS = 4,  // RTS sent, waiting for CTS
      WFACK = 5,  // DATA packet sent, waiting for ACK
      BUSY = 6    // during transmission of an ACK or a BROADCAST packet
    };

    struct NeighborEntry {
        int address;
        simtime_t age;
        long bitrate;
    };

    typedef std::list<NeighborEntry> NeighborList;
    
  public:
    Module_Class_Members(Mac80211a, BasicLayer, 0);

    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage*);
    
  protected:
    void sendToAssignment(BasicAssignmentMessage* m);
    void removeFromList(int dest);
    void clearAssignmentList();
    void requestAssignment();
    void recordToSink(int kind, double doubleVal);
    void recordToSink(int kind, long longVal);
    void recordToSink(int kind, int longVal);
    void recordToSink(int kind, int idx, int longVal);
    void recordModesToSink(int);
    int getModeFromBitrate(long bitrate);
    bool eRtsForMe(Mac80211Pkt*);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerControl(cMessage*);

    /** @brief handle end of contention */
    virtual void handleEndContentionTimer();
    
    /** @brief handle a message that is not for me or errornous*/
    void handleMsgNotForMe(cMessage *af, double duration);
    /** @brief handle a message that was meant for me*/
    void handleMsgForMe(Mac80211Pkt*);
    // ** @brief handle a Broadcast message*/
    void handleBroadcastMsg(Mac80211Pkt*);

    void handleAssignmentMsg(cMessage*);

    /** @brief handle the end of a transmission...*/
    void handleEndTransmission();

    /** @brief handle end of SIFS*/
    void handleEndSifsTimer();
    /** @brief handle time out*/
    void handleTimeoutTimer();
    /** @brief NAV timer expired, the exchange of messages of other
       stations is done*/
    void handleNavTimer();

    void handleRTSframe(Mac80211Pkt*);
    void handleeRTSframe(Mac80211Pkt*);

    void handleDATAframe(Mac80211Pkt*);
    void handleeDATAframe(Mac80211Pkt*);

    void handleACKframe(Mac80211Pkt*);

    void handleCTSframe(Mac80211Pkt*);
    void handleeCTSframe(Mac80211Pkt*);

    /** @brief send data frame */
    virtual void sendDATAframe();

    /** @brief send Acknoledgement */
    void sendACKframe(Mac80211Pkt*);

    /** @brief send CTS frame */
    void sendCTSframe(Mac80211Pkt*);
    
    void sendeCTSframe(Mac80211Pkt*);

    /** @brief send RTS frame */
    virtual void sendRTSframe();
    
    virtual void sendeRTSframe();

    /** @brief send broadcast frame */
    void sendBROADCASTframe();

    /** @brief encapsulate packet */
    Mac80211Pkt* encapsMsg(cMessage *netw);

    /** @brief decapsulate packet */
    cMessage* decapsMsg(Mac80211Pkt *frame);
    
    /** @brief start a new contention period */
    virtual void beginNewCycle();

    /** @brief Compute a backoff value */
    double backoff();

    /** @brief Compute a new contention window */
    int contentionWindow();

    /** @brief Test if maximum number of retries to transmit is exceeded */
    void testMaxAttempts();
    void testMaxAttempts(Mac80211Pkt* frame);

    /** @brief return a timeOut value for a certain type of frame*/
    double timeOut(frameType_802_11 type, double br);

    /** @brief computes the duration of a transmission over the physical channel, given a certain bitrate */
    double packetDuration(int bits, double br);
    double packetDuration(int bits, double br, int numSta);

    double frameDuration(double, int);
    double frameDuration(double);


    /** @brief Produce a readable name of the given state */
    const char *stateName(State state);

    /** @brief Produce a readable name of given frame type */
    const char *Mac80211a::frameTypeName(int kind);

    /** @brief Sets the state, and produces a log message in between */
    void setState(State state);

    /** @brief Check whether the next packet should be send with RTS/CTS */
    bool rtsCts(Mac80211Pkt* m) {
        return m->length() > rtsCtsThreshold;
    }

    /** @brief suspend an ongoing contention, pick it up again when the channel becomes idle */
    void suspendContention();

    /** @brief figure out at which bitrate to send to this particular destination */
    long retrieveBitrate(int destAddress);

    /** @brief take care of short retry counter and its dependencies */
    void incrementShortRetry();

    /** @brief add a new entry to the neighbor list */
    void addNeighbor(Mac80211Pkt *af);

    /** @brief find a neighbor based on his address */
    NeighborList::iterator findNeighbor(int id)  {
        NeighborList::iterator it;
        for(it = neighbors.begin(); it != neighbors.end(); ++it) {
            if(it->address == id) break;
        }
        return it;
    }

    /** @brief find the oldest neighbor -- usually in order to overwrite this entry */
    NeighborList::iterator findOldestNeighbor() {
        NeighborList::iterator it = neighbors.begin();
        NeighborList::iterator oldIt = neighbors.begin();
        simtime_t age = it->age;
        for(; it != neighbors.end(); ++it) {
            if(it->age < age) {
                age = it->age;
                oldIt = it;
            }
        }
        return oldIt;
    }

    // additions by Thomas Freitag <tf@uni-paderborn.de>
    bool isLastFrag(Mac80211Pkt* af);
    void setFragmentNumber(Mac80211Pkt* af, int num);
    void addToDefragBuffer(Mac80211Pkt* af);
    bool Mac80211a::defragBufferComplete();
    void Mac80211a::clearDefragBuffer();
    int Mac80211a::nextSequenceNumber();
    int Mac80211a::currentSequenceNumber();
    int getMode(double);

    void deleteFrame(Mac80211Pkt* f);
    void appendFrame(Mac80211Pkt* f);
    void cancelFrame(int dest);
    void eraseFromDestList(int dest);

    long getOFDMABitRate(int);

protected:
    /**
     * @brief MAC address (simply module id)
     **/
    int myMacAddr;

    /** @brief duration of cts and ack frames */
    double ackCtsDuration;

    bool noeCtsScheduled;
    bool noDelayedAckScheduled;

    // TIMERS:

    /** @brief Timer used for time-outs after the transmission of a RTS,
       a CTS, or a DATA packet*/
    cMessage* timeout;

    /** @brief Timer used for the defer time of a node. Also called NAV :
       networks allocation vector*/
    cMessage* nav;

    /** @brief Timer used for contention periods*/
    cMessage* contention;
    
    /** @brief Timer used to indicate the end of a SIFS*/
    cMessage* endSifs;

    /** @brief Current state of the MAC*/
    State state;

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

    /** @brief Default bitrate
     *
     * The default bitrate must be set in the omnetpp.ini. It is used
     * whenever an auto bitrate is not appropriate, like broadcasts.
     */
    long defaultBitrate;
    
    /** @brief Current bit rate at which data is transmitted
     */
    long bitrate;

    /** @brief bitrate for control frames */
    long controlBitrate;

    /** @brief bitrate chosen for sending cycle */
    long currentBitrate;

    /** @brief and category number */
    int catBitrate;
    /** @brief Auto bit rate adaptation -- switch */
    bool autoBitrate;
    /** @brief Hold RSSI thresholds at which to change the bitrates */
    std::vector<double> snrThresholds;

    /** @brief cached pointer to radio module */
    SingleChannelRadio* radio;

    /** @brief Maximal number of packets in the queue; should be set in
       the omnetpp.ini*/
    unsigned queueLength;

    /** @brief Boolean used to know if the next packet is a broadcast packet.*/
    bool nextIsBroadcast;

    /** @brief Buffering of messages from upper layer*/
    MacPktList fromUpperLayer;

    /** @brief Holds frames to send, and additional information in an extended cycle */
    SendList sendList;

    /** @brief Holds frames accessible by destination address */
    MultiMacPktList fromUpperLayerByDest;

    bool extendedCycle;



    /** @brief Number of frame transmission attempt
     *
     *  Incremented when the SHORT_RETRY_LIMIT is hit, or when an ACK
     *  or CTS is missing.
     */
    int longRetryCounter;

    /** @brief Number of frame transmission attempt*/
    int shortRetryCounter;
    
    /** @brief remaining backoff time.
     * If the backoff timer is interrupted,
     * this variable holds the remaining backoff time. */

    simtime_t remainingBackoff;
    
    /** @brief Number of bits in a packet before RTS/CTS is used */
    int rtsCtsThreshold;

    /** @brief Very small value used in timer scheduling in order to avoid
       multiple changements of state in the same simulation time.*/
    double delta;

    /** @brief Keep information for this many neighbors */
    unsigned neighborhoodCacheSize;
    /** @brief Consider information in cache outdate if it is older than this */
    simtime_t neighborhoodCacheMaxAge;

    NeighborList neighbors;
   
    // additions by Thomas Freitag <tf@uni-paderborn.de>
    /** @brief list to store received fragments */
    MacPktList defragBuffer;
    // no longer in use // TF //int maxFragmentNumber;
    // no longer in use // TF //int curFragmentNumber;
    int sequenceNumber;
    long fragmentationThreshold;
    bool defragBufferIsComplete;

    BBBufferState* bufState;
    int bufStateCat;

    OFDMSnrListEntry myOFDMStates;

    int currentControlDelay;
    double eCtsDuration;

    bool useOFDMAextensions;

    DestAddrList destList; // generate destination addresslist

    int toAssignment;
    int fromAssignment;
    int toSink;

    Assignment myAssignment;
    
    int currentMagicNumber;

    long overheadBitCounter;
    long payloadBitCounter;
    double overheadTimeCounter;
    double payloadTimeCounter;
    double tempOverheadTimeCounter;
    double tempPayloadTimeCounter;
    double tempPacketRateTimeCounter;
    long tempPacketRateBitCounter;
};

#endif
