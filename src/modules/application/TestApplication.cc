#include "TestApplication.h"

#include "BaseMacLayer.h"
#include "NetwControlInfo.h"
#include "ApplPkt_m.h"

using std::endl;

Define_Module(TestApplication);

void TestApplication::initialize(int stage) {
    BaseModule::initialize(stage);


    if (stage == 0) {
        // begin by connecting the gates
        // to allow messages exchange
        dataOut = findGate("lowerLayerOut");
        dataIn = findGate("lowerLayerIn");
        ctrlOut = findGate("lowerControlOut");
        ctrlIn  = findGate("lowerControlIn");


        // Retrieve parameters
        debug = par("debug").boolValue();
        stats = par("stats").boolValue();
        trace = par("trace").boolValue();

        isTransmitting    = false;

        nbPackets         = par("nbPackets");
        trafficParam      = par("trafficParam").doubleValue();
        nodeAddr          = LAddress::L3Type( par("nodeAddr").longValue() );
        dstAddr           = LAddress::L3Type( par("dstAddr").longValue() );
        flood             = par("flood").boolValue();
        PAYLOAD_SIZE      = par("payloadSize"); // data field size
        PAYLOAD_SIZE      = PAYLOAD_SIZE * 8; // convert to bits
        // Configure internal state variables and objects
        nbPacketsReceived = 0;
        remainingPackets  = nbPackets;

        INITIAL_DELAY     = 5; // initial delay before sending first packet

        // start timer if needed
        if (nodeAddr != 0 && remainingPackets > 0) {
            delayTimer = new cMessage("app-delay-timer");
            scheduleAt(simTime() + INITIAL_DELAY +uniform(0,0.001), delayTimer); // we add a small shift to avoid systematic collisions
        } else {
            delayTimer = 0;
        }

        if (stats) {
            // we should collect statistics
            cModule *host = getParentModule();
            int nbNodes = host->size();
            for (int i = 0; i < nbNodes; i++) {
                std::ostringstream oss;
                oss << "latency";
                oss << i;
                cStdDev aLatency(oss.str().c_str());
                latencies.push_back(aLatency);
            }
        }

        if (trace) {
            // record all packet arrivals
            latenciesRaw.setName("rawLatencies");
        }

    }

}

TestApplication::~TestApplication() {
	if (delayTimer)
        cancelAndDelete(delayTimer);
}

void TestApplication::finish() {

    if (stats) {
        recordScalar("Packets received", nbPacketsReceived);
        recordScalar("Aggregate latency", testStat.getMean());
        for (unsigned int i = 0; i < latencies.size(); i++) {
            cStdDev aLatency = latencies[i];
            aLatency.record();
        }
    }
}

void TestApplication::handleMessage(cMessage * msg) {
	debugEV << "In TestApplication::handleMessage" << endl;
    if (msg == delayTimer) {
        if (debug) {
        	debugEV << "  processing application timer." << endl;
        }
        if (! isTransmitting) {
			// create and send a new message
			ApplPkt* msg = new ApplPkt("SomeData");
			msg->setBitLength(PAYLOAD_SIZE);
			msg->setSrcAddr(nodeAddr);
			msg->setDestAddr(dstAddr);
			NetwControlInfo::setControlInfo(msg, dstAddr);
			if (debug) {
				debugEV << " sending down new data message to Aloha MAC layer for radio transmission." << endl;
			}
			send(msg, dataOut);
			isTransmitting = true;
			// update internal state
			remainingPackets--;
		}
        // reschedule timer if appropriate
        if (remainingPackets > 0) {
            if (!flood && !delayTimer->isScheduled()) {
                scheduleAt(simTime() + exponential(trafficParam) + 0.001, delayTimer);
            }
        } else {
            cancelAndDelete(delayTimer);
            delayTimer = 0;
        }
    } else if (msg->getArrivalGateId() == dataIn) {
        // we received a data message from someone else !
        ApplPkt* m = dynamic_cast<ApplPkt*>(msg);
        if (debug)
        	debugEV << "I (" << nodeAddr << ") received a message from node "
                << m->getSrcAddr() << " of size " << m->getBitLength() << "." << endl;
        nbPacketsReceived++;

        if (stats) {
            simtime_t theLatency = msg->getArrivalTime() - msg->getCreationTime();
            latencies[m->getSrcAddr()].collect(SIMTIME_DBL(theLatency));
            testStat.collect(SIMTIME_DBL(theLatency));
        }

        if (trace) {
            simtime_t theLatency = msg->getArrivalTime() - msg->getCreationTime();
            latenciesRaw.record(SIMTIME_DBL(theLatency));
        }

        delete msg;
    } else if (msg->getArrivalGateId() == ctrlIn) {
    	debugEV << "Received a control message." << endl;
        // msg announces end of transmission.
        if (msg->getKind() ==BaseMacLayer::TX_OVER) {
        	isTransmitting = false;
            if (remainingPackets > 0 && flood && !delayTimer->isScheduled()) {
                scheduleAt(simTime() + 0.001*001 + uniform(0, 0.001*0.001), delayTimer);
            }
        }
        delete msg;
    } else {
        // default case
        if (debug) {
			ApplPkt* m = static_cast<ApplPkt*>(msg);
			debugEV << "I (" << nodeAddr << ") received a message from node "
					<< (static_cast<ApplPkt*>(msg))->getSrcAddr() << " of size " << m->getBitLength() << "." << endl;
        }
        delete msg;
    }
}

