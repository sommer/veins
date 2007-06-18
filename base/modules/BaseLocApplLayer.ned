import
	"BaseLocalization",
	"BaseLocAppl",
	"LocFilter";

// An application layer for applications which need localization support.
//
// This module is "defined like" BaseApplLayer, so  that it can easily be 
// used instead of the BaseApplLayer.
//
// This layer consists of three modules: BaseLocAppl, BaseLocalization
// and LocFilter. In order to create an application with localization
// capabilities using this layer, the modules BaseLocAppl and 
// BaseLocalization should be extended.
//
// @author Peterpaul Klein Haneveld
module BaseLocApplLayer
    parameters:
        debug: bool, // debug switch
        headerLength: numeric const, // length of the application message header (in bits)
        locType: string, // type of localization module
        applType: string; // type of application module

    gates:
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer

    submodules:
        appl: applType like BaseLocAppl;
            display: "p=100,50;b=50,30";
        loc: locType like BaseLocalization;
            display: "p=200,50;b=50,30";
        filter: LocFilter;
            display: "p=150,100;b=50,30";
    connections:
        // connect gates to filter
        lowergateIn --> filter.lowergateIn;
        lowergateOut <-- filter.lowergateOut;
        lowerControlIn --> filter.lowerControlIn;
        lowerControlOut <-- filter.lowerControlOut;
        // connect filter to appl
        filter.applgateOut --> appl.lowergateIn;
        filter.applgateIn <-- appl.lowergateOut;
        filter.applControlOut --> appl.lowerControlIn;
        filter.applControlIn <-- appl.lowerControlOut;
        // connect filter to loc
        filter.locgateOut --> loc.lowergateIn;
        filter.locgateIn <-- loc.lowergateOut;
        filter.locControlOut --> loc.lowerControlIn;
        filter.locControlIn <-- loc.lowerControlOut;
endmodule

