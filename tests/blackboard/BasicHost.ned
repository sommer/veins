// empty host to test bb

package org.mixim.tests.blackboard;

module BasicHost
{
    parameters:
        // parameters for the framework
        double numHosts;
        string appendDisplay;
        string color;

        @display("bgb=250,250,$color,,;bgp=10,10");
    submodules:
        statechanger: StateChanger {
            parameters:
                @display("p=50,90;b=30,25");
        }
        statevisualize: StateVisualize {
            parameters:
                @display("p=200,30;b=30,25");
        }
        stateview: StateView {
            parameters:
                @display("p=200,110;b=30,25");
        }
        testparam: TestStateSub {
            parameters:
                @display("p=200,160;b=30,25");

        }
    connections:

}
