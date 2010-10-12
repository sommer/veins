package mixnet.tests.mixnetWorldUtility;
import mixnet.modules.MixnetWorldUtility;

simple OmnetTest {
}

network OmnetTestNetwork
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
        
        test: OmnetTest {
        } 	
}
