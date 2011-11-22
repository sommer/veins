package org.mixim.modules.node;

//
// This NED module defines a host using an IEEE 802.15.4 transceiver at 2.4GHz
// for wireless communications, that can be used to simulate wireless sensor
// networks. 
//
// For instance, you can use it to compare your own MAC protocol to
// the 802.15.4 non beacon enabled MAC protocol, or to evaluate IEEE 802.15.4
// wireless sensor networks. It is also a good starting point to define your
// own Host file.
//
// This model was independently validated on a wireless sensor network testbed.
// For more information, see 
// Accurate Timeliness Simulations for Real-Time Wireless Sensor Networks,
// J. Rousselot, J.-D. Decotignie, M. Aoun, P. van der Stok, R. Serna Oliver,
// G. Fohler. In Proceedings of the 2009 Third UKSim European Symposium on Computer 
// Modeling and Simulation. <a href="http://dx.doi.org/10.1109/EMS.2009.34">http://dx.doi.org/10.1109/EMS.2009.34</a>.
//
// @author Jérôme Rousselot
//
module Host802154_2400MHz extends WirelessNodeBatteryPlusTran
{
    parameters:
        int numHosts; // total number of hosts in the network

        applicationType = default("SensorApplLayer");
        transportType   = default("Aggregation");
        nicType         = default("Nic802154_TI_CC2420");  //type of used nic
        arpType         = default("org.mixim.modules.netw.ArpHost");

        batteryStats.detail = default(false);
        batteryStats.timeSeries = default(false);

        battery.nominal = default(1000 mAh);
        battery.capacity = default(battery.nominal);
        battery.voltage = default(3 V);
        battery.resolution = default(60 s);
        battery.publishDelta = default(1);
        battery.publishTime = default(battery.resolution);
        battery.numDevices = default(1);
}

