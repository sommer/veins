import
    "TestPhyLayer";

module TestNic
	parameters:
		connectionManagerName: string, //name of the ConnectionManager module
		phyLayerType: string; //type of the physical layer
    gates:
		in: radioIn; // radioIn gate for sendDirect
    submodules:
        phy: phyLayerType like TestPhyLayer;
            display: "p=60,150;i=prot3";
    connections:
    
	radioIn --> phy.radioIn;

    display: "p=10,10;b=101,180,rect;o=white";
endmodule

