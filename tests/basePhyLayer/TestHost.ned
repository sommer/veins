import
    "TestNic",
    "BaseMobility",
    "BaseUtility";


module TestHost
    parameters:
        numHosts : numeric const; // total number of hosts in the network

    gates:
        in: radioIn;

    submodules:
        utility: BaseUtility;
            display: "p=130,60;b=25,25;o=black";   
        nic: TestNic;
        parameters:			
            display: "b=32,30;p=60,166;i=iface";
    connections:

		radioIn --> nic.radioIn;

    display: "p=10,10;b=180,200,rect;o=white";
endmodule

