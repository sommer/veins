#include "catch/catch.hpp"

#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/base/messages/AirFrame_m.h"

// Helper methods
namespace {
    class DummySimulation {
    public:
        DummySimulation(cNullEnvir* envir)
            : simulation("DummySimulation", envir)
        {
            // envir is stored and deleted automatically by omnet++
            cSimulation::setActiveSimulation(&simulation);
            SimTime::setScaleExp(-9);
        }
        ~DummySimulation()
        {
            cSimulation::setActiveSimulation(nullptr);
        }
    private:
        cStaticFlag csf;
        cSimulation simulation;
    }; // end DummySimulation


    AirFrame createAirframe(double centerFreq, double bandwidth, simtime_t start, simtime_t length, double power) {
        Signal s(start, length);
        Mapping* txPowerMapping = MappingUtils::createMapping(Argument::MappedZero(), DimensionSet::timeFreqDomain(), Mapping::LINEAR);
        Argument pos(DimensionSet::timeFreqDomain());
        pos.setArgValue(Dimension::frequency(), centerFreq - bandwidth/2);
        pos.setTime(start);
        txPowerMapping->setValue(pos, power);
        pos.setTime(start + length);
        txPowerMapping->setValue(pos, power);
        pos.setArgValue(Dimension::frequency(), centerFreq + bandwidth/2);
        txPowerMapping->setValue(pos, power);
        pos.setTime(start);
        txPowerMapping->setValue(pos, power);
        s.setTransmissionPower(txPowerMapping);

        AirFrame frame;
        frame.setSignal(s);
        return frame;
    }
}


SCENARIO("TwoRayInterferenceModel", "[analogueModel]") {

    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr));

    GIVEN("An AirFrame at 2.4e9 sent from (0,0)") {
    
        AirFrame frame = createAirframe(2.4e9, 10e6, 0, .001, 1);
        Signal& s = frame.getSignal();
        TwoRayInterferenceModel tri(1.02, false);
        Argument start(DimensionSet::timeFreqDomain());
        start.setTime(0);
        start.setArgValue(Dimension::frequency(), 2.4e9);

        Coord senderPos(0, 0, 2);


        WHEN("the receiver is at (10,0)") {
            Coord receiverPos(10, 0, 2);

            THEN("TwoRayInterferenceModel drops power from 1 to 959.5e-9") {
                tri.filterSignal(&frame, senderPos, receiverPos);
                REQUIRE(s.getReceivingPower()->getValue(start) == Approx(959.5e-9).epsilon(.01));
            }
        }

        WHEN("the receiver is at (100,0)") {
            Coord receiverPos(100, 0, 2);

            THEN("TwoRayInterferenceModel drops power from 1 to 20.3e-9") {
                tri.filterSignal(&frame, senderPos, receiverPos);
                REQUIRE(s.getReceivingPower()->getValue(start) == Approx(20.3e-9).epsilon(.01));
            }
        }

    }
}
