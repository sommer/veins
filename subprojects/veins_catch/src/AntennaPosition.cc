#include "catch2/catch.hpp"

#include "veins/base/utils/AntennaPosition.h"
#include "testutils/Simulation.h"

using Veins::AntennaPosition;
using Veins::Coord;

#ifndef NDEBUG
SCENARIO("Using non-initialized AntennaPosition", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works

    GIVEN("A default-constructed AntennaPosition")
    {
        AntennaPosition p;

        WHEN("Its position is requested")
        {
            THEN("An error should be raised (when in DEBUG mode)")
            {
                REQUIRE_THROWS(p.getPositionAt(0));
            }
        }
    }
}
#endif

SCENARIO("Using AntennaPosition", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works

    GIVEN("An AntennaPosition at (1, 0, 0) moving by (1, 0, 0) each second after 0")
    {
        int hostA = 1;
        Coord posA = Coord(1, 0, 0);
        Coord speedA = Coord(1, 0, 0);
        simtime_t timeA = SimTime(0, SIMTIME_S);
        auto p = AntennaPosition(hostA, posA, speedA, timeA);

        THEN("its x value at t=1 is 2")
        {
            REQUIRE(p.getPositionAt(1).x == 2);
        }

        WHEN("compared with a different antenna")
        {
            int hostB = 2;
            auto p2 = AntennaPosition(hostB, posA, speedA, timeA);
            THEN("it is found to be different")
            {
                REQUIRE(p.isSameAntenna(p2) == false);
            }
        }
    }
}
