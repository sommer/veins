import
    "BaseNode",
    "BaseObject",
    //"PropagationModel";
    "ChannelControl";

module BaseNetwork
    parameters:
        playgroundSizeX: numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY: numeric const, // y size of the area the nodes are in (in meters)
        numObjects: numeric const, // total number of objects in the network
        numNodes: numeric const; // total number of hosts in the network

    submodules:
        propagationmodel: ChannelControl;
            parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY;
            display: "p=68,54;i=abstract/multicast";
        node: BaseNode[numNodes];
            display: "p=170,50;i=device/wifilaptop";
        object: BaseObject[numObjects];
            display: "b=40,24;p=164,114";
    connections nocheck:
                         // all connections and gates are to be generated dynamically

    display: "b=,,rect;o=white";
endmodule


// basic example simulation MiXiM
network baseSim : BaseNetwork
endnetwork

