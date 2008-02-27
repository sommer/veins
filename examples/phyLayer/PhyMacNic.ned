import
    "TestPhyLayer",
    "SimpleMacLayer";

module PhyMacNic
	parameters:
		id: numeric const,				//unique id of nic
		connectionManagerName: string; 	//name of the ConnectionManager module
		
    gates:
		in: radioIn; // radioIn gate for sendDirect
		
    submodules:
        phy: TestPhyLayer;
        	display: "p=60,120;b=80,30,rect;o=white";
            //display: "p=60,150;i=prot3";
            
        mac: SimpleMacLayer;
        	display: "p=60,45;b=80,30,rect;o=white";
        	//display: "p=60,70;i=prot3";
            
    connections:    
    	phy.upperGateOut --> mac.lowerGateIn display "m=m,25,50,25,0;o=black";
        phy.upperGateIn <-- mac.lowerGateOut display "m=m,15,50,15,0;o=black";
        phy.upperControlOut --> mac.lowerControlIn display "m=m,75,50,75,0;o=red";
        phy.upperControlIn <-- mac.lowerControlOut display "m=m,85,0,85,0;o=red";
        
		radioIn --> phy.radioIn;

    display: "p=10,10;b=101,150,rect;o=white";
endmodule

