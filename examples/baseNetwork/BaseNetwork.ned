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
            display: "p=60,30;i=eye14";
        node: BaseNode[numNodes];
            display: "p=50,50;i=laptop";
        object: BaseObject[numObjects];
            display: "b=40,24;p=100,50";
    display: "p=10,10;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule


// basic example simulation MiXiM
network baseSim : BaseNetwork
endnetwork

