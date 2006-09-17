// empty host to test radio

import
    "RadioStateChanger",
    "SingleChannelRadio",
    "Blackboard";

module BasicHost
    parameters:
        // parameters for the framework
        numHosts                   : numeric const,
        appendDisplay              : string,
        color                      : string;

    submodules:
        blackboard: Blackboard;
            display: "p=50,30;b=30,25";
        statechanger: RadioStateChanger
            display: "p=50,90;b=30,25";
        radio: SingleChannelRadio
            display: "p=200,30;b=30,25";

    connections:
    
    display: "p=10,10;b=250,250,rect;o=$color";
endmodule
