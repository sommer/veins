#include "catch2/catch.hpp"

#include "veins/modules/analogueModel/SimplePathlossModel.h"
#include "veins/base/toolbox/Spectrum.h"
#include "veins/base/toolbox/Signal.h"
#include "testutils/Simulation.h"
#include "testutils/Component.h"

using namespace Veins;

namespace {

int dummyId = -1;

AntennaPosition createDummyAntennaPosition(Coord c)
{
    return AntennaPosition(dummyId, c, Coord(0, 0, 0), simTime());
}

} // namespace

SCENARIO("SimplePathlossModel with alpha = 2", "[analogueModel]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr));
    DummyComponent dc(&ds);
    double centerFreq = 5.9e9;
    std::vector<double> freqs = {centerFreq - 5e6, centerFreq, centerFreq + 5e6};
    Spectrum spec(freqs);
    SimplePathlossModel spm(&dc, 2.0, false, {0, 0, 0});

    GIVEN("A signal sent from (0, 0) with powerlevel 1")
    {
        Signal s(spec);
        s = 1;
        s.setSenderPoa({createDummyAntennaPosition(Coord(0, 0, 2)), {}, nullptr});
        WHEN("the receiver is at (2, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(2, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 4.0874e-6")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(4.0874e-6).epsilon(0.001));
            }
        }

        WHEN("the receiver is at (5, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(5, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 6.539e-7")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(6.539e-7).epsilon(0.001));
            }
        }

        WHEN("the receiver is at (10, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(10, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 1.634e-7")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(1.634e-7).epsilon(0.001));
            }
        }

        WHEN("the receiver is at (100, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(100, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 1.634e-9")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(1.634e-9).epsilon(0.001));
            }
        }
    }
}

SCENARIO("SimplePathlossModel with alpha = 2.2", "[analogueModel]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr));
    DummyComponent dc(&ds);
    double centerFreq = 5.9e9;
    std::vector<double> freqs = {centerFreq - 5e6, centerFreq, centerFreq + 5e6};
    Spectrum spec(freqs);
    SimplePathlossModel spm(&dc, 2.2, false, {0, 0, 0});

    GIVEN("A signal sent from (0, 0) with powerlevel 1")
    {
        Signal s(spec);
        s = 1;
        s.setSenderPoa({createDummyAntennaPosition(Coord(0, 0, 2)), {}, nullptr});
        WHEN("the receiver is at (2, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(2, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 3.5583e-6")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(3.5583e-6).epsilon(0.001));
            }
        }

        WHEN("the receiver is at (5, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(5, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 4.7400e-7")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(4.7400e-7).epsilon(0.001));
            }
        }

        WHEN("the receiver is at (10, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(10, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 1.0316e-7")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(1.0316e-7).epsilon(0.001));
            }
        }

        WHEN("the receiver is at (100, 0)")
        {
            s.setReceiverPoa({createDummyAntennaPosition(Coord(100, 0, 2)), {}, nullptr});
            THEN("SimplePathlossModel drops power from 1 to 6.5090e-10")
            {
                spm.filterSignal(&s);
                REQUIRE(s.at(1) == Approx(6.5090e-10).epsilon(0.001));
            }
        }
    }
}
