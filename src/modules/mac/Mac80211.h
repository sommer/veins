/* -*- mode:c++ -*- ********************************************************
 * file:        Mac80211.h
 *
 * author:      David Raguin/Marc L�bbers
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


#ifndef MAC_80211_H
#define MAC_80211_H

#include <list>

#include "MiXiMDefs.h"
#include "BaseMacLayer.h"
#include "Consts80211.h"
#include "Mac80211Pkt_m.h"

class ChannelSenseRequest;

/**
 * @brief An implementation of the 802.11b MAC.
 *
 * For more info, see the NED file.
 *
 * @ingroup macLayer
 * @ingroup ieee80211
 * @author David Raguin, Karl Wessel (port for MiXiM)
 */
class MIXIM_API Mac80211 : public BaseMacLayer
{
public:

	/** @brief frame kinds */
	enum Mac80211MessageKinds {
	  //between MAC layers of two nodes
	  RTS = LAST_BASE_MAC_MESSAGE_KIND, // request to send
	  CTS,                 // clear to send
	  ACK,                 // acknowledgement
	  DATA,
	  BROADCAST,
	  LAST_MAC_80211_MESSAGE_KIND
	};
protected:
	/** @brief Type for a queue of Mac80211Pkts.*/
    typedef std::list<Mac80211Pkt*> MacPktList;

    /** Definition of the timer types */
    enum timerType {
      TIMEOUT,
      NAV
    };

    /** Definition of the states*/
    enum State {
      WFDATA = 0, // waiting for data packet
      QUIET = 1,  // waiting for the communication between two other nodes to end
      IDLE = 2,   // no packet to send, no packet receiving
      CONTEND = 3,// contention state (battle for the channel)
      WFCTS = 4,  // RTS sent, waiting for CTS
      WFACK = 5,  // DATA packet sent, waiting for ACK
      BUSY = 6    // during transmission of an ACK or a BROADCAST packet
    };

    /** @brief Data about a neighbor host.*/
    struct NeighborEntry {
    	/** @brief The neighbors address.*/
    	LAddress::L2Type address;
        int              fsc;
        simtime_t        age;
        double           bitrate;
    };

    /** @brief Type for a list of NeighborEntries.*/
    typedef std::list<NeighborEntry> NeighborList;

  public:
    Mac80211();
    virtual ~Mac80211();

    virtual void initialize(int);
    virtual void finish();

 protected:

    /** @brief Handle self messages such as timer... */
	virtual void handleSelfMsg(cMessage*);

	/** @brief Handle messages from upper layer */
	virtual void handleUpperMsg(cMessage* msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage*);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerControl(cMessage*);


    /** @brief handle end of contention */
    virtual void handleEndContentionTimer();

    /** @brief handle a message that is not for me or errornous*/
    void handleMsgNotForMe(cMessage *af, simtime_t_cref duration);
    /** @brief handle a message that was meant for me*/
    void handleMsgForMe(Mac80211Pkt*);
    // ** @brief handle a Broadcast message*/
    void handleBroadcastMsg(Mac80211Pkt*);

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

    void handleDATAframe(Mac80211Pkt*);

    void handleACKframe(Mac80211Pkt*);

    void handleCTSframe(Mac80211Pkt*);

    void dataTransmissionFailed();

    void rtsTransmissionFailed();

    /** @brief send data frame */
    virtual void sendDATAframe(Mac80211Pkt*);

    /** @brief send Acknoledgement */
    void sendACKframe(Mac80211Pkt*);

    /** @brief send CTS frame */
    void sendCTSframe(Mac80211Pkt*);

    /** @brief send RTS frame */
    virtual void sendRTSframe();

    /** @brief send broadcast frame */
    void sendBROADCASTframe();

    /** @brief encapsulate packet */
    Mac80211Pkt* encapsMsg(cPacket *netw);

    /** @brief decapsulate packet */
    cMessage* decapsMsg(Mac80211Pkt *frame);

    /** @brief start a new contention period */
    virtual void beginNewCycle();

    /** @brief Compute a backoff value */
    simtime_t backoff(bool rtscts = true);

    /** @brief Test if maximum number of retries to transmit is exceeded */
    void testMaxAttempts();

    /** @brief return a timeOut value for a certain type of frame*/
    simtime_t timeOut(Mac80211MessageKinds type, double br);

    /** @brief computes the duration of a transmission over the physical channel, given a certain bitrate */
    simtime_t packetDuration(double bits, double br);

    /** @brief Produce a readable name of the given state */
    const char *stateName(State state);

    /** @brief Sets the state, and produces a log message in between */
    void setState(State state);

    /** @brief Check whether the next packet should be send with RTS/CTS */
    bool rtsCts(Mac80211Pkt* m) {
        return m->getBitLength() - MAC80211_HEADER_LENGTH > rtsCtsThreshold;
    }

    /** @brief suspend an ongoing contention, pick it up again when the channel becomes idle */
    void suspendContention();

    /** @brief figure out at which bitrate to send to this particular destination */
    double retrieveBitrate(const LAddress::L2Type& destAddress);

    /** @brief add a new entry to the neighbor list */
    void addNeighbor(Mac80211Pkt *af);

    /** @brief find a neighbor based on his address */
    NeighborList::iterator findNeighbor(const LAddress::L2Type& address)  {
        NeighborList::iterator it;
        for(it = neighbors.begin(); it != neighbors.end(); ++it) {
            if(it->address == address) break;
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


    /**
     * @brief Starts a channel sense request which sense the channel for the
     * passed duration or until the channel is busy.
     *
     * Used during contend state to check if the channel is free.
     */
    void senseChannelWhileIdle(simtime_t_cref duration);

    /**
     * @brief Creates the signal to be used for a packet to be sent.
     */
    Signal* createSignal(simtime_t_cref start, simtime_t_cref length, double power, double bitrate);

protected:

    // TIMERS:

    /** @brief Timer used for time-outs after the transmission of a RTS,
       a CTS, or a DATA packet*/
    cMessage* timeout;

    /** @brief Timer used for the defer time of a node. Also called NAV :
       networks allocation vector*/
    cMessage* nav;

    /** @brief Used to sense if the channel is idle for contention periods*/
    ChannelSenseRequest* contention;

    /** @brief Timer used to indicate the end of a SIFS*/
    ChannelSenseRequest* endSifs;

    /** @brief Stores the the time a channel sensing started.
     * Used to calculate the quiet-time of the channel if the sensing was
     * aborted. */
    simtime_t chSenseStart;

    /** @brief Current state of the MAC*/
    State state;

    /** @brief Default bitrate
     *
     * The default bitrate must be set in the omnetpp.ini. It is used
     * whenever an auto bitrate is not appropriate, like broadcasts.
     */
    double defaultBitrate;

    /** @brief The power at which data is transmitted */
    double txPower;

    /** @brief Stores the center frequency the Mac uses. */
    double centerFreq;

    /** @brief Current bit rate at which data is transmitted */
    double bitrate;
    /** @brief and category number */
    int catBitrate;
    /** @brief Auto bit rate adaptation -- switch */
    bool autoBitrate;
    /** @brief Hold RSSI thresholds at which to change the bitrates */
    std::vector<double> snrThresholds;

    /** @brief Maximal number of packets in the queue; should be set in
       the omnetpp.ini*/
    unsigned queueLength;

    /** @brief Boolean used to know if the next packet is a broadcast packet.*/
    bool nextIsBroadcast;

    /** @brief Buffering of messages from upper layer*/
    MacPktList fromUpperLayer;

    /** @brief Number of frame transmission attempt
     *
     *  Incremented when the SHORT_RETRY_LIMIT is hit, or when an ACK
     *  or CTS is missing.
     */
    unsigned longRetryCounter;

    /** @brief Number of frame transmission attempt*/
    unsigned shortRetryCounter;

    /** @brief remaining backoff time.
     * If the backoff timer is interrupted,
     * this variable holds the remaining backoff time. */
    simtime_t remainingBackoff;

    /** @brief current IFS value (DIFS or EIFS)
     * If an error has been detected, the next backoff requires EIFS,
     * once a valid frame has been received, resets to DIFS. */
    simtime_t currentIFS;

    /** @brief Number of bits in a packet before RTS/CTS is used */
    int rtsCtsThreshold;

    /** @brief Very small value used in timer scheduling in order to avoid
       multiple changements of state in the same simulation time.*/
    simtime_t delta;

    /** @brief Keep information for this many neighbors */
    unsigned neighborhoodCacheSize;
    /** @brief Consider information in cache outdate if it is older than this */
    simtime_t neighborhoodCacheMaxAge;

    /** @brief A list of this hosts neighbors.*/
    NeighborList neighbors;

    /** take care of switchover times */
    bool switching;

    /** sequence control -- to detect duplicates*/
    int fsc;
};

#endif
