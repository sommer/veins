package org.mixim.base.modules;

module BaseObject
{
    parameters:
        @display("bgb=180,200,white,,;bgp=10,10");
    submodules:
        mobility: BaseMobility {
            parameters:
                @display("p=130,172;i=cogwheel2");

        }
}

