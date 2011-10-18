/*
 * ProbabilisticBroadcast.h
 *
 *  Created on: Nov 26, 2008
 *      Author: Damien Piguet, Dimitris Kotsakos, Jérôme Rousselot
 */

#ifndef ADAPTIVEPROBABILISTICBROADCAST_H_
#define ADAPTIVEPROBABILISTICBROADCAST_H_

#include <map>

#include "MiXiMDefs.h"
#include "ProbabilisticBroadcast.h"
#include "SimpleAddress.h"

/**
 * @brief This class extends ProbabilisticBroadcast by adding
 *        an algorithm which adapts broadcasting parameters
 *        according to network conditions.
 *
 * @ingroup netwLayer
 * @author Dimitris Kotsakos, George Alyfantis, Damien Piguet
 **/
class MIXIM_API AdaptiveProbabilisticBroadcast : public ProbabilisticBroadcast
{
public:


    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

protected:
    typedef std::map<LAddress::L3Type, cMessage*> NeighborMap;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle self messages */
    virtual void handleSelfMsg(cMessage* msg);

    void updateNeighMap(ProbabilisticBroadcastPkt* m);

    void updateBeta();

    //read from omnetpp.ini
    simtime_t timeInNeighboursTable; //Default ttl for NeighborTable entries in seconds


    cOutVector bvec;


    NeighborMap neighMap;

};

#endif /* ADAPTIVEPROBABILISTICBROADCAST_H_ */
