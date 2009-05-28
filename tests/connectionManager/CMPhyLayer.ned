package org.mixim.tests.connectionManager;

moduleinterface CMPhyLayer
{
    parameters:
        bool usePropagationDelay;
    gates:
        input radioIn; // for sendDirect
}


simple ConnectedBCNodePhyLayer like CMPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

simple NotConnectedBCNodePhyLayer like CMPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

simple ConnectedRNodePhyLayer like CMPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

simple NotConnectedRNodePhyLayer like CMPhyLayer
{
    parameters:
        bool usePropagationDelay = false;
    gates:
        input radioIn; // for sendDirect
}

