package org.mixim.examples.analogueModels;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

//
//This example shows how to use AnalogueModels.
//
//It contains the following modules/classes:
//
//RandomFreqTimeModel:
//An AnalogueModel implementation which defines random attunation values
//in time and frequency using the MultiDimMapping.
//
//RandomFrequencyOnlyModel:
//An AnalogueModel implementation which defines random attenuation values
//in the frequency domain by using the MultiDimMapping class.
//Since every mapping represented by the Mapping interface of MiXiM has to be
//defined over time this shows how to "simulate" mappings which arent't 
//defined over time.
//
//ThresholdDecider:
//A quick and dirty implementation of a very verbose Decider. It is only
//used for a better illustration of how the AnalogueModels affect the signal
//during runtime and shouldn't be used as a template for an actual Decider.
//
//SamplePhyLayer:
//A PhyLayer subclass which knows how to initialize our own AnalogueModels
//and Decider.
//
//SimpleMacLayer:
//A very basic implementation of a Mac layer without using the actual
//BaseMacLayer. This is only to keep the example independent from the 
//BaseMacLayer, normally you whould just use the BaseMacLayer.
//
//
//The network itself consists of only two host which keep ping-pong-ing
//messages between each other.
//
//The simulation has the following run configurations:
//
//- "General" 		- infinite ping-pong-ing without simulation of propagation delay
//- "WithPropDelay"	- same with simulation of propagation delay
//- "PerfTest"		- meant to be used when called with CmdEnv for some performance
//			 		  testing (time limited)
//	
//
//See the comments in the source for more details.
//
network AnalogueModels
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=100,0;i=misc/globe");

        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=200,0;b=42,42,rect,red,,;i=abstract/multicast");

        }
        node[numHosts]: PhyMacHost {
            parameters:
                numHosts = numHosts;
                @display("p=50,50;b=42,42,rect,red,,;i=device/wifilaptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

