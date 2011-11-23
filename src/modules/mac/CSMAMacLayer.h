#ifndef CSMAMAC_LAYER_H
#define CSMAMAC_LAYER_H

#include <list>

#include "MiXiMDefs.h"
#include "BaseMacLayer.h"


/**
 * @class CSMAMacLayer
 * @brief MAC module which provides non-persistent CSMA
 *
 * This is an implementation of a simple non-persistent CSMA. The
 * idea of nonpersistent CSMA is "listen before talk". Before
 * attempting to transmit, a station will sense the medium for a
 * carrier signal, which, if present, indicates that some other
 * station is sending.
 *
 * If the channel is busy a random waiting time is computed and after
 * this time the channel is sensed again. Once the channel gets idle
 * the message is sent. (State of the channel is obtained from ChannelInfo
 * via phyLayer.)
 *
 * An option of this module is to use a queue. If a packet from the
 * upper layer arrives although there is still another packet waiting
 * for its transmission or being transmitted the new packet can be
 * stored in this queue. The length of this queue can be specified by
 * the user in the omnetpp.ini file. By default the length is 0. If
 * the queue is full or there is no queue (length = 0) new packet(s)
 * will be deleted.
 *
 * ATTENTION: Imagine the following scenario:
 *
 * Several stations receive a broadcast request packet, usally exactly
 * at the same time. They will all try to answer at exactly the same
 * time, i.e. they all will sense the channel at exactly the same time
 * and start to transmit because the channel was sensed idle by all of
 * them. Therefore a small random delay should be built into one/some
 * of the upper layers to simulate a random processing time!
 *
 * The TestApplLayer e.g. implements such a processing delay!
 *
 * @ingroup macLayer
 * @ingroup csma
 * @author Marc L�bbers, Yosia Hadisusanto, Andreas Koepke, Karl Wessel
 */
class MIXIM_API CSMAMacLayer : public BaseMacLayer
{
  public:
	virtual ~CSMAMacLayer();

  protected:
    /** @brief Type for a queue of cPackets.*/
    typedef std::list<cPacket*> MacQueue;

    /** @brief MAC states
     *
     *  The MAC states help to keep track what the MAC is actually
     *  trying to do -- this is esp. useful when radio switching takes
     *  some time.
     */
    enum States {
    	/** @brief MAC accepts packets from PHY layer.*/
        RX,
        /** @brief Clear Channel Assessment - MAC checks whether medium is busy.*/
        CCA,
        /** @brief MAC transmits a packet. */
        TX,
    };

    /** @brief kepp track of MAC state */
    States macState;

    /** @brief Duration of a slot
     *
     *  This duration can be a mini-slot (that is an RX-TX-turnaround
     *  time) but in general it should be the time it takes to send a
     *  packet plus some guard times (RX-TX-turnaround + a minimum
     *  inter-packet space).
     */
    double slotDuration;

    /** @brief Maximum time between a packet and its ACK
     *
     * Usually this is slightly more then the tx-rx turnaround time
     * The channel should stay clear within this period of time.
     */
    double difs;

    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    MacQueue macQueue;

    /** @brief length of the queue*/
    unsigned queueLength;

    /** @brief Timer for backoff (non-persistent CSMA) */
    cMessage* backoffTimer;

    /** @brief Multi-purpose message.
     *
     * Keeps track of things like minimum time of clear channel
     * (important for atomic acks) and race condition avoidance.
     */
    cMessage* minorMsg;

    /** @brief count the number of tx attempts
     *
     * This holds the number of transmission attempts, since no Acks
     * are used this is the way how we can empty our queue even with
     * overloaded channels.
     */
    unsigned txAttempts;

     /** @brief maximum number of transmission attempts
     *
     * The packet is discarded when this number is reached.
     */
    unsigned maxTxAttempts;

    /** @brief the bit rate at which we transmit */
    double bitrate;

    /** @brief The power at which we transmit. */
    double txPower;

    /** @brief initial contention window size */
    unsigned initialCW;

    /** @brief Counts the total number of backoffs. */
    double nbBackoffs;

    /** @brief Counts the total time spent in backoff. */
    double backoffValues;

    /** @brief Counts the number of frames transmitted. */
    unsigned long nbTxFrames;

    /** @brief Inspect reasons for dropped packets */
    //DroppedPacket droppedPacket;

    /** @brief plus category from BB */
    //int catDroppedPacket;

    /** @brief publish dropped packets nic wide */
    //int nicId;

protected:
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

    /** @brief schedule a backoff
     *
     *  A non-persistent CSMA ideally uses a fixed backoff window, but
     *  it could also increase this window in a linear fashion, or
     *  even exponential -- here, a linear increase is used. Overwrite
     *  this function to define another schedule function.  It also
     *  checks the retransmission counters -- if you overwrite it, do
     *  it with care.
     */
    virtual void scheduleBackoff();

    /**
     * @brief Encapsulates the NetwPkt into the MacPkt and sets the parameters.
     */
    virtual MacPkt* encapsMsg(cPacket* pkt);
};

#endif

