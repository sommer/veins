import "mac";

simple Application
    gates:
        in: RadioInMsg;
        out: RadioOutMsg;
        in: USInMsg;
        out: USOutMsg;
endsimple

simple Routing
    gates:
        in: fromApp;
        out: toApp;
        in: fromMac;
        out: toMac;
endsimple

module Software
    parameters:
        macType: string,
        routingType: string,
        applicationName: string,
		lpl: bool,
		lpl_on: numeric,
		lpl_off: numeric,
		siftNodes: numeric;

    gates:
        in: fromRadio;
        out: toRadio;
        in: fromUS;
        out: toUS;
	in: in;
	out: out;

    submodules:
        mac: macType like MacClass;
			parameters:
				lpl = lpl,
				lpl_on = lpl_on,
				lpl_off = lpl_off,
				siftNodes = siftNodes;
            display: "i=abstract/multicast;p=214,72";
        application: applicationName like Application;
            display: "i=device/terminal;p=55,72";
        routing: routingType like Routing;
            display: "i=abstract/dispatcher;p=124,72";

    connections:
        mac.toRouting --> routing.fromMac;
        mac.fromRouting <-- routing.toMac;
        mac.toRadio --> toRadio;
        mac.fromRadio <-- fromRadio;
        routing.toApp --> application.RadioInMsg;
        routing.fromApp <-- application.RadioOutMsg;

	application.USInMsg <-- fromUS;
	application.USOutMsg --> toUS;
	
	in --> out
		display "m=m,0,50,0,50";
    display: "b=260,138";
endmodule

