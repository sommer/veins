package org.mixim.modules.application;

import org.mixim.base.modules.IBaseApplLayer;

simple TestApplication like IBaseApplLayer
{
    parameters:
        bool   debug = default(false); // debug switch
        bool   stats = default(false);   // stats switch
        bool   trace = default(false); // trace switch
        double trafficParam @unit(s);   // mean time between packets (poisson arrival rate)
        double nodeAddr;        // node address
	    double dstAddr;		// packet destination node address
        bool   flood;                   // send packets continuously
        double payloadSize @unit(byte);     // number of bytes per packet
        double nbPackets;	// number of packets to generate
    	int    headerLength @unit("bit");   // length of the application message header (in bits)

    gates:
        input lowerLayerIn; // to receive data from communications stack
        output lowerLayerOut; // to send data to communications stack
        input lowerControlIn; // to receive control messages from communications stack
        output lowerControlOut; // to send  control messages from communications stack

}
