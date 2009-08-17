// Energy Framework for Omnet++, version 0.9
//
// Author:  Laura Marie Feeney 
// 
// Copyright 2009 Swedish Institute of Computer Science.  
// 
// This software is provided `as is' and without any express or implied
// warranties, including, but not limited to, the implied warranties of
// merchantability and fitness for a particular purpose.
//
//*********************************************************************
package org.mixim.tests.power.deviceTestMulti;

module Network
{
    parameters:
        double playgroundSizeX;
        double playgroundSizeY;
        double numHosts;

    submodules:
        host[numHosts]: SimpleHost {
            parameters:
                @display("p=50,50;b=30,25,rect");
        }
    connections allowunconnected:
}

network sim extends Network
{
    parameters:
}
