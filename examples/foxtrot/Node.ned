import
    "BaseApplLayer",
    "BaseAggLayer",
    "BaseNetwLayer",
    "BaseNic",
    "BaseMobility",
    "BaseArp",
    "BaseBattery",
    "BaseUtility";

module Node
    parameters:
		isSink: bool,
        routingType: string, //type of the network layer
        aggType: string, //type of the aggregation layer
        applType: string; //type of the application layer
    submodules:
        utility: BaseUtility;
            display: "p=130,38,rect;b=24,24;o=black";
        arp: BaseArp;
            display: "p=130,84,rect;b=24,24;o=blue";
        battery: BaseBattery;
            display: "p=130,128,rect;b=24,24;o=red";
        mobility: BaseMobility;
            display: "p=130,172;i=cogwheel2";
        appl: applType like BaseApplLayer;
            display: "p=60,50;i=app";
        agg: aggType like BaseAggLayer;
            display: "p=60,50;i=agg";
        net: routingType like BaseNetwLayer;
            display: "p=60,108;i=prot1";
        nic: BaseNic;
            display: "p=60,166;i=iface";
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> net.lowerControlIn display "m=m,70,0,70,0;o=red";
        nic.upperControlIn <-- net.lowerControlOut display "m=m,70,0,70,0;o=red";

        net.uppergateOut --> agg.lowergateIn;
        net.uppergateIn <-- agg.lowergateOut;
        net.upperControlOut --> agg.lowerControlIn display "m=m,70,0,70,0;o=red";
        net.upperControlIn <-- agg.lowerControlOut display "m=m,70,0,70,0;o=red";

        agg.uppergateOut --> appl.lowergateIn;
        agg.uppergateIn <-- appl.lowergateOut;
        agg.upperControlOut --> appl.lowerControlIn display "m=m,70,0,70,0;o=red";
        agg.upperControlIn <-- appl.lowerControlOut display "m=m,70,0,70,0;o=red";
    display: "b=,,rect;o=white";
endmodule

