import Node;
import BaseWorldUtility;
import ConnectionManager;

module Network
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        double numNodes; // total number of hosts in the network
        string prop;

        @display("bgb=,,white,,");
    submodules:
        propagationmodel: <prop> like ConnectionManager;
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=68,54;i=abstract/multicast");
        }
        node[numNodes]: Node {
            parameters:
                @display("p=170,50;i=device/wifilaptop");
        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}


// basic example simulation MiXiM
network baseSim extends Network
{
    parameters:
}

