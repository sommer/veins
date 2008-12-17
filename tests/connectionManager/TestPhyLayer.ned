package org.mixim.tests.connectionManager;

moduleinterface TestPhyLayer
{
    parameters:
        bool usePropagationDelay;
    gates:
        input radioIn; // for sendDirect
}


simple ConnectedBCNodePhyLayer like TestPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

simple NotConnectedBCNodePhyLayer like TestPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

simple ConnectedRNodePhyLayer like TestPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

simple NotConnectedRNodePhyLayer like TestPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

