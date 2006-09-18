import "propagationModel";

simple World
endsimple

module WorldContainer
	parameters:
		radioPropagationType: string,
		usPropagationType: string,
		nodeCount: const;

	gates:
		in: radioPropagationInPort[], usPropagationInPort[];
		out: radioPropagationOutPort[], usPropagationOutPort[];

	submodules:
		world: World;
		radioPropagation: radioPropagationType like PropagationModelClass;
			gatesizes:
				inPort[nodeCount],
				outPort[nodeCount];
		usPropagation: usPropagationType like PropagationModelClass;
			gatesizes:
				inPort[nodeCount],
				outPort[nodeCount];

	connections:
		for i=0..nodeCount-1 do
			radioPropagation.inPort[i] <-- radioPropagationInPort[i];
			radioPropagation.outPort[i] --> radioPropagationOutPort[i];
			usPropagation.inPort[i] <-- usPropagationInPort[i];
			usPropagation.outPort[i] --> usPropagationOutPort[i];
		endfor
endmodule
