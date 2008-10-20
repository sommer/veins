package org.mixim.tests.connectionManager;

moduleinterface TestPhyLayer
{
    gates:
        input radioIn; // for sendDirect
}


simple ConnectedBCNodePhyLayer like TestPhyLayer
{
    gates:
        input radioIn; // for sendDirect
}

simple NotConnectedBCNodePhyLayer like TestPhyLayer
{
    gates:
        input radioIn; // for sendDirect
}

simple ConnectedRNodePhyLayer like TestPhyLayer
{
    gates:
        input radioIn; // for sendDirect
}

simple NotConnectedRNodePhyLayer like TestPhyLayer
{
    gates:
        input radioIn; // for sendDirect
}

