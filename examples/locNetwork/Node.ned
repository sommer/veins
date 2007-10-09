import
    "BaseApplLayer",
//    "BaseNetwLayer",
    "QueuedRouting",
    "BaseLocalization",
    "BaseNic",
//    "BaseMobility",
    "ConstSpeedMobility",
    "BaseArp",
    "BaseBattery",
    "BaseUtility";

module LocNode
    parameters:
        applType: string, // type of the application layer
	locType: string; // type of the localization layer
    gates:
	    in: radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility;
            display: "p=130,38,rect;b=24,24;o=black";
        arp: BaseArp;
            display: "p=130,84,rect;b=24,24;o=blue";
        battery: BaseBattery;
            display: "p=130,128,rect;b=24,24;o=red";
        mobility: ConstSpeedMobility;// BaseMobility;
            display: "p=130,172;i=cogwheel2";
        appl: applType like BaseApplLayer;
            display: "p=60,50;i=app";
        net: QueuedRouting;
            display: "p=60,108;i=prot1";
	loc: locType like BaseLocalization;
	    display: "p=60,137";
        nic: BaseNic;
            display: "p=60,166;i=iface";
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> net.lowerControlIn display "m=m,70,0,70,0;o=red";
        nic.upperControlIn <-- net.lowerControlOut display "m=m,70,0,70,0;o=red";

        net.uppergateOut --> loc.lowergateIn;
        net.uppergateIn <-- loc.lowergateOut;
        net.upperControlOut --> loc.lowerControlIn display "m=m,70,0,70,0;o=red";
        net.upperControlIn <-- loc.lowerControlOut display "m=m,70,0,70,0;o=red";

        loc.uppergateOut --> appl.lowergateIn;
        loc.uppergateIn <-- appl.lowergateOut;
        loc.upperControlOut --> appl.lowerControlIn display "m=m,70,0,70,0;o=red";
        loc.upperControlIn <-- appl.lowerControlOut display "m=m,70,0,70,0;o=red";

	radioIn --> nic.radioIn;

    display: "b=,,rect;o=white";
endmodule

