import
	"wirelessChannel";
	
simple ms
	parameters:
		playgroundSizeX,
		playgroundSizeY,
		mobilityModel,
		gridSpacing,
		speed,
		waitTime,
		qty_ms;
	gates:
		in: in[];			// one input from channelManager
		out: out[];			// possibly multiple outputs for e.g. MIMO (qty_channels)
endsimple

module mobileStation
	parameters:
		playgroundSizeX,
		playgroundSizeY,
		subbands,
		qty_ms,
		qty_channels;
	gates:
		in: in[];			// qty_channels times
		out: out[];			// qty_channels times
	submodules:
		msBehavior: ms; 
			parameters:	
				playgroundSizeX = ref playgroundSizeX,
				playgroundSizeY = ref playgroundSizeY,
				qty_ms = ref qty_ms;
			gatesizes:
				in[qty_channels],
				out[qty_channels];
			display: "i=device/cellphone";

		Channel: chann[qty_channels]
			parameters:
				subbands = ref subbands;
			display: "i=block/mac_vs";
//		mobility: mobilityType like BasicMobility;
//			display: "p=132,141;i=block/cogwheel";
		
	connections:
		for c=0..qty_channels - 1  do
			in[c] --> msBehavior.in[c];
			msBehavior.out[c] --> Channel[c].in;
			Channel[c].out --> out[c];
		endfor;
//	display:
endmodule
