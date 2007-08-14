// empty host to test bb

import
    "StateChanger",
    "StateVisualize",
    "StateView",
    "TestStateSub",
    "BaseUtility";

module BasicHost
    parameters:
        // parameters for the framework
        numHosts                   : numeric const,
        appendDisplay              : string,
        color                      : string;

    submodules:
        blackboard: BaseUtility;
            display: "p=50,30;b=30,25";
        statechanger: StateChanger
            display: "p=50,90;b=30,25";
        statevisualize: StateVisualize
            display: "p=200,30;b=30,25";
        stateview: StateView
            display: "p=200,110;b=30,25";
        testparam: TestStateSub
            display: "p=200,160;b=30,25";

    connections:
    
    display: "p=10,10;b=250,250,rect;o=$color";
endmodule
