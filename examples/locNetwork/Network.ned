import
    "Node",
    "BaseObject",
    "BaseWorldUtility",
	"ConnectionManager";

module BaseNetwork
    parameters:
        playgroundSizeX: numeric const, // x size of the area the nodes are in (in meters)
        playgroundSizeY: numeric const, // y size of the area the nodes are in (in meters)
        playgroundSizeZ: numeric const, // z size of the area the nodes are in (in meters)
        numObjects: numeric const, // total number of objects in the network
        numNodes: numeric const, // total number of hosts in the network
	prop_type : string;

    submodules:
		propagationmodel: prop_type like ConnectionManager;
		world: BaseWorldUtility;
           parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY,
                playgroundSizeZ = playgroundSizeZ;
        node: LocNode[numNodes];
            display: "p=170,50;i=device/wifilaptop";
        object: BaseObject[numObjects];
            display: "b=40,24;p=164,114";
    connections nocheck:

    display: "b=,,rect;o=white";
endmodule


// basic example simulation MiXiM
network baseSim : BaseNetwork
endnetwork

