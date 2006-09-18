// Definitions of simple modules to allow dynamic creation.
// First two are the scenario manager and area manager, others are traffic patterns
simple ScenarioManager
endsimple
simple AreaManager
endsimple

simple LocalBC
	parameters:
		replyprob:string, 
		replydelay:string;
	gates:
		in: in; out:out;
endsimple

simple LocalUC
	parameters:
		replyprob:string,
		replydelay:string;
	gates: in: in; out:out;
endsimple

simple ToSink
	parameters:
		sinknode:string,
		ttl:string,
		aggmode:string,
		aggdelay:string,
		aggmax:string;
	gates: in: in; out: out;
endsimple

simple LocalGossip
	parameters:
		centernode:string,
		hops:string,
		replyprob:string,
		replydelay:string;
	gates:
		in: in; out: out;
endsimple

simple HopLatency
	parameters:
		source:string;
	gates:
		in: in; out: out;
endsimple

simple Idle
	parameters:
	gates:
		in: in; out: out;
endsimple

simple BCFlood
	parameters:
		source:string;
	gates:
		in: in; out: out;
endsimple
