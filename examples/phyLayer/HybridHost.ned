import
    "PhyMacNic",
    "BaseMobility",
    "BaseUtility";


module HybridHost
    parameters:
        numHosts : numeric const; // total number of hosts in the network

    gates:
        in: GSMRadioIn;
        in: ISMRadioIn;

    submodules:
        utility: BaseUtility;
            display: "p=70,50;i=block/blackboard";   
        mobility: BaseMobility;
            display: "p=150,50;i=block/cogwheel";  
        ISMNic: PhyMacNic;		
        	display: "b=42,42,rect;o=green;p=70,150;i=block/ifcard";
    	GSMNic: PhyMacNic;		
        	display: "b=42,42,rect;o=red;p=150,150;i=block/ifcard";
    connections:

		GSMRadioIn --> GSMNic.radioIn;
		ISMRadioIn --> ISMNic.radioIn;

    display: "p=10,10;b=200,200,rect;o=white";
endmodule

