simple channelStateCalc
	parameters:
		calculatePathLoss,
		calculateShadowing,
		calculateFading,
		correlated_subbands,
		use_linear_scale,
		lightspeed,
		alpha,
		tenlogk,
		mean,
		std_dev,
		fadingPaths,
		center_frequency,
		freq_spacing,
		subbands,
		delay_rms;
	gates:
		in: in;
		out: out;
endsimple

simple errorCalc
	parameters:
	gates:
		in: in;
		out: out;
endsimple

module chann
	parameters:
		subbands;
	gates:
		in: in;
		out: out;
	submodules: 
		channelStateCalculator: channelStateCalc;
			parameters:
				subbands = ref subbands;
		errorCalculator: errorCalc;
	connections:
		in --> channelStateCalculator.in;
		channelStateCalculator.out --> errorCalculator.in; 
		errorCalculator.out --> out; 
endmodule 