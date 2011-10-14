#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

#include <vector>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseModule.h"

class MIXIM_API TestApplication : public BaseModule {


    public:
	virtual ~TestApplication();

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
        std::vector < cStdDev > latencies;
        cOutVector latenciesRaw;
        cStdDev testStat;

};

#endif // TEST_APPLICATION_H

