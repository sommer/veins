import
    "BaseApplLayer",
    "BaseNetwLayer",
    "BaseNic",
    "BaseMobility",
    "BaseArp",
    "BaseBattery",
    "BaseUtility";



module BaseNode
    submodules:
        utility: BaseUtility;
            display: "p=130,38,rect;b=24,24;o=black";
        arp: BaseArp;
            display: "p=130,84,rect;b=24,24;o=blue";
        battery: BaseBattery;
            display: "p=130,128,rect;b=24,24;o=red";
        mobility: BaseMobility;
            display: "p=130,172;i=cogwheel2";
        appl: BaseApplLayer;
            display: "p=60,50;i=app";
        net: BaseNetwLayer;
            display: "p=60,108;i=prot1";
        nic: BaseNic;
            display: "p=60,166;i=iface";
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> net.lowerControlIn display "m=m,70,0,70,0;o=red";
        nic.upperControlIn <-- net.lowerControlOut display "m=m,70,0,70,0;o=red";

        net.uppergateOut --> appl.lowergateIn;
        net.uppergateIn <-- appl.lowergateOut;
        net.upperControlOut --> appl.lowerControlIn display "m=m,70,0,70,0;o=red";
        net.upperControlIn <-- appl.lowerControlOut display "m=m,70,0,70,0;o=red";

    display: "p=10,10;b=180,200,rect;o=white";
endmodule

