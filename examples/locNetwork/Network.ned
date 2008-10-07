import Node;
import BaseObject;
import BaseWorldUtility;
import ConnectionManager;

module BaseNetwork
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        double numObjects; // total number of objects in the network
        double numNodes; // total number of hosts in the network
        string prop_type;

        @display("bgb=,,white,,");
    submodules:
        propagationmodel: <prop_type> like ConnectionManager;
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
        }
        node[numNodes]: LocNode {
            parameters:
                @display("p=170,50;i=device/wifilaptop");
        }
        object[numObjects]: BaseObject {
            parameters:
                @display("b=40,24;p=164,114");
        }
    connections allowunconnected:

}


// basic example simulation MiXiM
network baseSim extends BaseNetwork
{
    parameters:
}

