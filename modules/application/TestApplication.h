#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#include <omnetpp.h>
#include "BaseModule.h"
#include "MacToPhyInterface.h"
#include "MacToPhyControlInfo.h"
#include "MacControlInfo.h"
#include "Signal_.h"
#include "MacPkt_m.h"
#include "ApplPkt_m.h"
#include "AlohaMacLayer.h"
#include "NicControlType.h"
#include "Mapping.h"

class TestApplication : public BaseModule {


    public:
	virtual void initialize(int stage);
	virtual void handleMessage(cMessage* msg);
        virtual void finish();

    protected:
        // gates
    	int dataOut;
    	int dataIn;
        int ctrlOut;
        int ctrlIn;

        // timers
        cMessage* delayTimer;

        // module parameters
        int nbPackets, remainingPackets, headerLength, nodeAddr, dstAddr;
        double trafficParam;
        bool debug, stats, trace;
        bool flood;
        bool isTransmitting;

        // constants
        int INITIAL_DELAY;
        int PAYLOAD_SIZE;

        // state variables
        int nbPacketsReceived;
        vector < cStdDev > latencies;
        cOutVector latenciesRaw;
        cStdDev testStat;

};

#endif // TEST_APPLICATION_H

