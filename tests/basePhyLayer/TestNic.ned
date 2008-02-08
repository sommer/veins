import
    "TestPhyLayer",
    "TestMacLayer";

module TestNic
	parameters:
		connectionManagerName: string; //name of the ConnectionManager module
		
    gates:
		in: radioIn; // radioIn gate for sendDirect
		
    submodules:
        phy: TestPhyLayer;
            display: "p=60,150;i=prot3";
            
        mac: TestMacLayer;
            
    connections:    
    	phy.upperGateOut --> mac.lowerGateIn;
        phy.upperGateIn <-- mac.lowerGateOut;
        phy.upperControlOut --> mac.lowerControlIn display "m=m,50,50,75,0;o=red";
        phy.upperControlIn <-- mac.lowerControlOut display "m=m,25,0,25,0;o=red";
        
		radioIn --> phy.radioIn;

    display: "p=10,10;b=101,180,rect;o=white";
endmodule

