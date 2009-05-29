/***************************************************************************
 * file:        DummyRoute.h
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2009 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Adaptor module that simply "translates" netwControlInfo to macControlInfo
 *
 **************************************************************************/

#ifndef dummyroute_h
#define dummyroute_h

#include <omnetpp.h>

//#include <fstream>
#include "BaseLayer.h"
#include "NetwPkt_m.h"
#include "MacPkt_m.h"
#include "BaseMacLayer.h"
#include "SimTracer.h"
#include "NetwControlInfo.h"
#include "MacControlInfo.h"
#include "BaseMobility.h"

#include <map>
#include <list>
#include <math.h>

using namespace std;

/**
 * @brief Adaptor module that simply "translates" netwControlInfo to macControlInfo
 *
 * @ingroup netwLayer
 * @author Jerome Rousselot
 **/

class DummyRoute : public BaseLayer
{
public:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

protected:
    /**
     * @brief Length of the NetwPkt header
     * Read from omnetpp.ini
     **/
    int headerLength;

    /** @brief cached variable of my network address */
    int netaddress;
    int macaddress;


    bool trace, stats, debug;


    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMsg(cMessage* msg) { };

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    virtual void handleUpperControl(cMessage* msg) { delete msg; }

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(NetwPkt *msg);

};

#endif
