package org.mixim.modules.application;

simple TestApplication

{
    parameters:
        bool debug; // debug switch
        bool stats;   // stats switch
        bool trace; // trace switch
        double trafficParam;   // mean time between packets (poisson arrival rate)
        double nodeAddr;        // node address
	double dstAddr;		// packet destination node address
        bool flood;                   // send packets continuously
        double payloadSize @unit(byte);     // number of bytes per packet
        double nbPackets;	// number of packets to generate


    gates:
        input lowerGateIn; // to receive data from communications stack
        output lowerGateOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack

}
