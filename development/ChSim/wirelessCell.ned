import
	"mobileStation";

simple triggerGen
	parameters:
		sample_per_sec,
		max_time;
	gates:
		out: trigger;
endsimple

simple fileWriter
	parameters:
		sample_per_sec,
		max_time,
		subbands,
		qty_ms,
		qty_channels,
		writeFiles,
		precision,
		directory_name: string,
		filename_prefix: string,
		mem_buf_size;
	gates:
		in: data_in;
endsimple

simple channelManager
	parameters:
		subbands,
		qty_ms,
		qty_channels;
	gates:
		in: trigger;
		out: data_out;
		in: in[];			// qty_ms * qty_channels times
		out: out[];			// qty_ms times
endsimple

module cell
	parameters:
		playgroundSizeX: numeric const,
		playgroundSizeY: numeric const,
		sample_per_sec,
		max_time,
		subbands,
		qty_ms,
		qty_channels;
	submodules: 
		TriggerGen: triggerGen;
			parameters:
				sample_per_sec = ref sample_per_sec,
				max_time = ref max_time;
			display: "i=block/source";
		FileWriter: fileWriter;
			parameters:
				sample_per_sec = ref sample_per_sec,
				max_time = ref max_time,
				subbands = ref subbands,
				qty_ms = ref qty_ms,
				qty_channels = ref qty_channels;
			display: "i=block/sink";
		ChannelManager: channelManager;
			parameters:
				subbands = ref subbands,
				qty_ms = ref qty_ms,
				qty_channels = ref qty_channels;
			gatesizes:
				in[qty_ms*qty_channels],
				out[qty_ms*qty_channels];
			display: "i=block/wrxtx";
		MobileStation: mobileStation[qty_ms]; 
			parameters:	
				playgroundSizeX = ref playgroundSizeX,
				playgroundSizeY = ref playgroundSizeY,
				subbands = ref subbands,
				qty_ms = ref qty_ms,
				qty_channels = ref qty_channels;
			gatesizes:
				in[qty_channels], out[qty_channels];
			display: "i=device/cellphone";

		connections:
			TriggerGen.trigger --> ChannelManager.trigger;
			ChannelManager.data_out --> FileWriter.data_in; 
			for j=0..qty_ms - 1, c=0..qty_channels - 1  do
				ChannelManager.out[j * qty_channels + c] --> MobileStation[j].in[c];
				MobileStation[j].out[c] --> ChannelManager.in[j * qty_channels + c];
			endfor;
		display: "p=0,0;b=$playgroundSizeX,$playgroundSizeY";
endmodule

network msMovementCell: cell
endnetwork
