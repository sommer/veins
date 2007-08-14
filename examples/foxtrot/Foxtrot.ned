import
    "Node",
    "BaseWorldUtility",
    "ConnectionManager";

module Network
    parameters:
        playgroundSizeX: numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY: numeric const, // y size of the area the nodes are in (in meters)
        playgroundSizeZ: numeric const, // z size of the area the nodes are in (in meters)
        numNodes: numeric const; // total number of hosts in the network

    submodules:
        propagationmodel: ConnectionManager;
		world: BaseWorldUtility;
            parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY,
                playgroundSizeZ = playgroundSizeZ;
            display: "p=68,54;i=abstract/multicast";
        node: Node[numNodes]
            display: "p=170,50;i=device/wifilaptop";
    connections nocheck:
                         // all connections and gates are to be generated dynamically

    display: "b=,,rect;o=white";
endmodule


// basic example simulation MiXiM
network baseSim : Network
endnetwork

