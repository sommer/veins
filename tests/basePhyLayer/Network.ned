import  
    "TestHost", 
    "ConnectionManager",
    "BaseWorldUtility"; 
 
module Network 
    parameters: 
        playgroundSizeX : numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY : numeric const, // y size of the area the nodes are in (in meters)
        playgroundSizeZ : numeric const, // z size of the area the nodes are in (in meters)
        numHosts : numeric const; // total number of hosts in the network
                                  
    submodules: 
    	world: BaseWorldUtility;
    		parameters: 
                playgroundSizeX = playgroundSizeX, 
                playgroundSizeY = playgroundSizeY,
                playgroundSizeZ = playgroundSizeZ;
                
        channelcontrol: ConnectionManager;             
            display: "p=60,30;i=eye14";
            
        node: TestHost[numHosts]; 
            parameters: 
                numHosts = numHosts; 
            display: "p=50,50;i=laptop";
            
    connections nocheck: 
                         // all connections and gates are to be generated dynamically
                         
    display: "p=10,10;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule 

network tbpSim : Network
endnetwork 

