import
    "BaseMacLayer",
    "BasePhyLayer",
    "BaseChannel";

module MfNic
    gates:
        in: uppergateIn; // to upper layers
        out: uppergateOut; // from upper layers
        in: upperControlIn; // control to upper
        out: upperControlOut; // control from upper
	//in: radioIn; // radioIn gate for sendDirect
    submodules:
        mac: BaseMacLayer;
            display: "p=60,50;i=prot2";
        chan: BaseChannel;
            display: "p=68,100;i=box2_s;o=white";
        phy: BasePhyLayer;
            display: "p=60,150;i=prot3";
    connections:
        phy.uppergateOut --> mac.lowergateIn;
        phy.uppergateIn <-- mac.lowergateOut;
        phy.upperControlOut --> mac.lowerControlIn display "m=m,50,50,75,0;o=red";
        phy.upperControlIn <-- mac.lowerControlOut display "m=m,25,0,25,0;o=red";

        mac.uppergateOut --> uppergateOut;
        mac.uppergateIn <-- uppergateIn;
        mac.upperControlOut --> upperControlOut display "m=m,80,0,60,0;o=red";
        mac.upperControlIn <-- upperControlIn display "m=m,60,0,80,0;o=red";

	//radioIn --> phy.radioIn;

    display: "p=10,10;b=101,180,rect;o=white";
endmodule

