package veins.modules.application.ieee80211p;

//
// Adapter module for integrating the Vanetza library into the Veins framework.
// Sits between the application layer and the NIC, handling:
//   - CAM generation logic (ETSI EN 302 637-2 TRC rules)
//   - Serialization and deserialization of V2X messages (CAM and DENM)
//   - Forwarding of data and control messages between appl and nic
//
simple VanetzaToVeinsBridge
{
    parameters:
        @display("i=block/network2");
        @class(veins::VanetzaToVeinsBridge);

        @signal[camSent](type=long);
        @statistic[camSent](
            title="CAM sent";
            source="camSent";
            record=vector,count,sum;
            interpolationmode=none
        );

        @signal[camReceived](type=long);
        @statistic[camReceived](
            title="CAM received";
            source="camReceived";
            record=vector,count,sum;
            interpolationmode=none
        );

        @signal[denmSent](type=long);
        @statistic[denmSent](
        	title="DENM sent";
        	source="denmSent";
        	record=vector,count,sum;
        	interpolationmode=none
       );

        @signal[denmReceived](type=long);
        @statistic[denmReceived](
        	title="DENM received";
        	source="denmReceived";
        	record=vector,count,sum;
        	interpolationmode=none
        );

    gates:
        // --- Data plane ---
        input upperLayerIn;    // incoming data from application layer
        output upperLayerOut;  // outgoing data to application layer
        input lowerLayerIn;    // incoming data from NIC
        output lowerLayerOut;  // outgoing data to NIC

        // --- Control plane ---
        input upperControlIn;    // control messages from application layer
        output upperControlOut;  // control messages to application layer
        input lowerControlIn;    // control messages from NIC
        output lowerControlOut;  // control messages to NIC
}