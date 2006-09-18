import
    "world";
import
    "node";

module SimNetwork
    parameters:
        nodeCount: numeric const,
        nodeType: string,
        radioType: string,
        routingType: string,
        radioPropagationType: string,
        usPropagationType: string,
        ultrasoundType: string,
        macType: string,
        radioRange: numeric const,
        applicationName: string,
		lpl: bool,
		lpl_on: numeric,
		lpl_off: numeric,
		siftNodes: numeric;

    submodules:
        nodes: NodeContainer[nodeCount];
            parameters:
                nodeType = nodeType,
                radioType = radioType,
                routingType = routingType,
                ultrasoundType = ultrasoundType,
                macType = macType,
                radioRange = radioRange,
                applicationName = applicationName,
				lpl = lpl,
				lpl_on = lpl_on,
				lpl_off = lpl_off,
				siftNodes = siftNodes;

            display: "i=abstract/accesspoint_s";
        world: WorldContainer;
            parameters:
                radioPropagationType = radioPropagationType,
                usPropagationType = usPropagationType,
                nodeCount = nodeCount;
            gatesizes:
                radioPropagationInPort[nodeCount],
                radioPropagationOutPort[nodeCount],
                usPropagationInPort[nodeCount],
                usPropagationOutPort[nodeCount];

            display: "i=misc/globe_l;p=200,200";
    connections:
        for i=0..nodeCount-1 do
            nodes[i].radioInPort <-- world.radioPropagationOutPort[i];
            nodes[i].radioOutPort --> world.radioPropagationInPort[i];
            nodes[i].ultrasoundInPort <-- world.usPropagationOutPort[i];
            nodes[i].ultrasoundOutPort --> world.usPropagationInPort[i];
        endfor;
    display: "b=400,400";
endmodule

network net : SimNetwork
endnetwork

