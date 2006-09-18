import "radio";
import "ultrasound";
import "software";

simple NodeClass
    parameters:
        pos_x: numeric const,
        pos_y: numeric const,
        pos_z: numeric const,
        node_id: numeric const,
		radioRange: numeric const;

    gates:
	in: in;
	out: out;
endsimple

module NodeContainer
    parameters:
        nodeType: string,
        radioType: string,
        routingType: string,
        ultrasoundType: string,
        macType: string,
        radioRange: numeric const,
        applicationName: string,
		lpl: bool,
		lpl_on: numeric,
		lpl_off: numeric,
		siftNodes: numeric;

    gates:
        in: radioInPort;
        out: radioOutPort;
        in: ultrasoundInPort;
        out: ultrasoundOutPort;

    submodules:
        node: nodeType like NodeClass;
			parameters:
				radioRange = radioRange;
            display: "i=device/memory;p=170,40";
        radio: radioType like RadioClass;
            display: "i=device/antennatower;p=263,112";
        ultrasound: ultrasoundType like UltrasoundClass;
            display: "i=block/wrxtx;p=82,112";
        software: Software;
            parameters:
                macType = macType,
                routingType = routingType,
                applicationName = applicationName,
				lpl = lpl,
				lpl_on = lpl_on,
				lpl_off = lpl_off,
				siftNodes = siftNodes;

            display: "i=device/cpu;p=172,112";
    connections:
        radio.fromAir <-- radioInPort;
        radio.toAir --> radioOutPort;
        radio.fromMac <-- software.toRadio;
        radio.toMac --> software.fromRadio;

        ultrasound.fromAir <-- ultrasoundInPort;
        ultrasound.toAir --> ultrasoundOutPort;
        ultrasound.fromSoftware <-- software.toUS;
        ultrasound.toSoftware --> software.fromUS;
        software.out --> node.in;
        node.out --> software.in;
    display: "b=319,179";
endmodule

