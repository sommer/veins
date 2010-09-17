package mixnet.tests.mixnetBridge;

import mixnet.MixnetWorldUtility;
import mixnet.MixnetBridge;

import org.mixim.base.modules.BaseWorldUtility;
import org.mixim.modules.netw.ArpHost;
import org.mixim.base.modules.BaseUtility;


simple OmnetTest {
    parameters:
        
        
    gates:
        input fromUpperBridge; 
        output toUpperBridge;

        input fromLowerBridge;
        output toLowerBridge;
        input fromCtrlLowerBridge; 
        output toCtrlLowerBridge;
}

moduleinterface IHost {
}

simple DummyTable {
}

module TestHost like IHost{
    submodules:
        interfaceTable: DummyTable {
            parameters:
                @display("p=60,200");
        }
        
        bridge:MixnetBridge {
        }
        
        testModule:OmnetTest {
        }
        
    connections:
        bridge.upperGateOut --> testModule.fromUpperBridge;
        bridge.upperGateIn <-- testModule.toUpperBridge;

        testModule.toLowerBridge --> { @display("ls=black;m=m,25,50,25,0"); } --> bridge.lowerGateIn;
        testModule.fromLowerBridge <-- { @display("ls=black;m=m,15,50,15,0"); } <-- bridge.lowerGateOut;
        testModule.toCtrlLowerBridge --> { @display("ls=red;m=m,75,50,75,0"); } --> bridge.lowerControlIn;
        testModule.fromCtrlLowerBridge <-- { @display("ls=red;m=m,85,0,85,0"); } <-- bridge.lowerControlOut;
}

module AddressingTestHost extends TestHost like IHost {
	submodules:
	    utility: BaseUtility {
	        parameters:
	            coreDebug = true;
	    }
	    addressing: ArpHost {
	        parameters:
	            coreDebug = true;
	    }   
}

network BaseNetwork {
    parameters:
        int run;
        string hostType = default("TestHost");
    submodules:
    	testHost: <hostType> like IHost {
        }
}

network Network extends BaseNetwork
{
    submodules:
        world: MixnetWorldUtility {
            parameters:
                playgroundSizeX = 100m;
                playgroundSizeY = 100m;
                playgroundSizeZ = 100m;
				coreDebug = true;
				useTorus = false;
				use2D = true;
        }   
}

network NoWorldNetwork extends BaseNetwork
{
}

network WrongWorldNetwork extends BaseNetwork
{    
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = 100m;
                playgroundSizeY = 100m;
                playgroundSizeZ = 100m;
				coreDebug = true;
				useTorus = false;
				use2D = true;
        } 	
}