import  
    "PhyMacHost", 
    "ConnectionManager",
    "BaseWorldUtility";
 
module Network 
    parameters: 
        playgroundSizeX : numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY : numeric const, // y size of the area the nodes are in (in meters)
        playgroundSizeZ : numeric const, // z size of the area the nodes are in (in meters)
        numGSMHosts : numeric const, // total number of weak hosts in the network
        numISMHosts : numeric const; // total number of strong hosts in the network
                                  
    submodules: 
    	world: BaseWorldUtility;
    		parameters: 
                playgroundSizeX = playgroundSizeX, 
                playgroundSizeY = playgroundSizeY,
                playgroundSizeZ = playgroundSizeZ;
            display: "p=250,0;i=misc/globe";
                                
        ISMChannel: ConnectionManager;
            display: "p=350,0;b=42,42,rect;o=green;i=abstract/multicast";
        GSMChannel: ConnectionManager;
            display: "p=450,0;b=42,42,rect;o=red;i=abstract/multicast";
            
        GSMNode: PhyMacHost[numGSMHosts]; 
            parameters: 
                numHosts = numGSMHosts; 
            display: "p=50,50;b=42,42,rect;o=red;i=device/palm";
            
        ISMNode: PhyMacHost[numISMHosts]; 
            parameters: 
                numHosts = numISMHosts; 
            display: "p=75,50;b=42,42,rect;o=green;i=device/wifilaptop";
            
        hybridNode: HybridHost;
        	parameters:
        		numHosts = 1;
        	display: "p=100,50;b=42,42,rect;o=yellow;i=device/antennatower";
            
    connections nocheck: 
                         // all connections and gates are to be generated dynamically
                         
    display: "p=10,50;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule 

network phySim : Network
endnetwork 

