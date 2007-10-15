import
    "TestBMBaseMobility",
    "BaseUtility";




module CrashHost
    
    parameters:
        numHosts : numeric const, 		// total number of hosts in the network
        borderPolicy : numeric const; 	// this hosts border policy


    submodules:
        blackboard: BaseUtility;
            display: "p=130,60;b=25,25;o=black";
        mobility: TestBMBaseMobility;
        	parameters:
            display: "p=130,130;i=cogwheel2";        

endmodule

