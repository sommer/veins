import BaseMobility;
import BaseUtility;

module BaseObject
{
    parameters:
        @display("bgb=180,200,white,,;bgp=10,10");
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38,rect;b=24,24,,black,,");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=130,172;i=cogwheel2");

        }
}

