#include "catch2/catch.hpp"

#include "veins/modules/phy/SampledAntenna1D.h"
#include "testutils/Simulation.h"
#include "veins/base/utils/FWMath.h"

using Veins::Coord;
using Veins::FWMath;
using Veins::SampledAntenna1D;

SCENARIO("Using SampledAntenna1D", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works

    GIVEN("A SampledAntenna1D")
    {
        std::vector<double> values = {FWMath::mW2dBm(1), FWMath::mW2dBm(2), FWMath::mW2dBm(3), FWMath::mW2dBm(4)};
        std::string offsetType = "";
        std::vector<double> offsetParams;
        std::string rotationType = "";
        std::vector<double> rotationParams;
        cRNG* rng = nullptr;

        auto p = SampledAntenna1D(values, offsetType, offsetParams, rotationType, rotationParams, rng);

        const std::vector<std::tuple<Coord, Coord, Coord, double>> checks = {
            std::make_tuple(Coord(0, 0, 0), Coord(1, 0, 0), Coord(1, 0, 0), 1.0), // front
            std::make_tuple(Coord(0, 0, 0), Coord(0, 1, 0), Coord(1, 0, 0), 2.0), // right
            std::make_tuple(Coord(0, 0, 0), Coord(-2, 0, 0), Coord(1, 0, 0), 3.0), // back
            std::make_tuple(Coord(0, 0, 0), Coord(0, -2, 0), Coord(1, 0, 0), 4.0), // left
            std::make_tuple(Coord(0, 0, 0), Coord(1, 0, 0), Coord(0, 1, 0), 4.0), // also left
            std::make_tuple(Coord(0, 0, 0), Coord(1, 1, 0), Coord(1, 0, 0), FWMath::dBm2mW(0.5 * (FWMath::mW2dBm(1) + FWMath::mW2dBm(2)))), // 45 deg to the right
            std::make_tuple(Coord(0, 0, 0), Coord(1, -1, 0), Coord(1, 0, 0), FWMath::dBm2mW(0.5 * (FWMath::mW2dBm(1) + FWMath::mW2dBm(4)))), // 45 deg to the left
        };
        for (auto& check : checks) {
            auto ownPos = std::get<0>(check);
            auto otherPos = std::get<1>(check);
            auto ownOrient = std::get<2>(check);
            auto res = std::get<3>(check);

            INFO("sending from " << ownPos << " to " << otherPos << " while looking at " << ownOrient << " should return " << res);
            double gain = p.getGain(ownPos, ownOrient, otherPos);
            REQUIRE(gain == Approx(res));
        }
    }
}
