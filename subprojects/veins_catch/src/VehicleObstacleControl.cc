#include "catch/catch.hpp"

#include "veins/modules/obstacle/VehicleObstacleControl.h"
#include "veins/base/toolbox/Spectrum.h"
#include "veins/base/toolbox/Signal.h"
#include "testutils/Simulation.h"

using Veins::Coord;
using Veins::Signal;
using Veins::Spectrum;
using Veins::VehicleObstacleControl;

SCENARIO("Using VehicleObstacleControl", "[vehicleObstacles]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works

    GIVEN("A constellation of sender and receiver vehicle, as well as an obstacle")
    {

        double h1 = 10;
        double h2 = 8;
        double h = 9;
        double d = 7;
        double d1 = 3;

        THEN("A signal at 5.89 GHz is attenuated by approx. 0.65 dB")
        {

            Spectrum::Frequencies freqs = {5.89e9};
            Signal attenuationPrototype = Signal(Spectrum(freqs));

            auto r = VehicleObstacleControl::getVehicleAttenuationSingle(h1, h2, h, d, d1, attenuationPrototype);

            REQUIRE(r.at(0) == Approx(0.6454291084));
        }

    }
}
