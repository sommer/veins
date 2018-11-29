#include "catch/catch.hpp"

#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/base/messages/AirFrame_m.h"
#include "testutils/Simulation.h"
#include "testutils/AirFrame.h"

using namespace Veins;

SCENARIO("TwoRayInterferenceModel", "[analogueModel]")
{

    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr));

    GIVEN("An AirFrame at 2.4e9 sent from (0,0)")
    {

        AirFrame frame = createAirframe(2.4e9, 10e6, 0, .001, 1);
        Signal& s = frame.getSignal();
        TwoRayInterferenceModel tri(1.02);

        Coord senderPos(0, 0, 2);

        WHEN("the receiver is at (10,0)")
        {
            Coord receiverPos(10, 0, 2);

            THEN("TwoRayInterferenceModel drops power from 1 to 959.5e-9")
            {
                tri.filterSignal(&s, senderPos, receiverPos);
                REQUIRE(s(2.4e9) == Approx(9.5587819943e-07).epsilon(1e-9));
            }
        }

        WHEN("the receiver is at (100,0)")
        {
            Coord receiverPos(100, 0, 2);

            THEN("TwoRayInterferenceModel drops power from 1 to 20.3e-9")
            {
                tri.filterSignal(&s, senderPos, receiverPos);
                REQUIRE(s(2.4e9) == Approx(2.0317806459e-08).epsilon(1e-9));
            }
        }
    }
}
