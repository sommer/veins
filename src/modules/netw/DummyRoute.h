/***************************************************************************
 * file:        DummyRoute.h
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2009 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Placeholder module that simply "translates" netwControlInfo to macControlInfo
 *
 **************************************************************************/

#ifndef dummyroute_h
#define dummyroute_h

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"

class NetwPkt;
/**
 * @brief Placeholder module that simply "translates" netwControlInfo to macControlInfo
 *
 * @ingroup netwLayer
 * @author Jerome Rousselot
 **/

class MIXIM_API DummyRoute : public BaseNetwLayer
{
public:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

protected:

    bool stats, trace;

    int networkID;  // the network to which we belong

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMsg(cMessage* msg) { };

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    virtual void handleUpperControl(cMessage* msg) { delete msg; }

    NetwPkt* encapsMsg(cPacket *appPkt);

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(NetwPkt *msg);

};

#endif
