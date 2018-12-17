#include "catch2/catch.hpp"
#include "veins/modules/mobility/traci/VehicleSignal.h"

using Veins::VehicleSignal;
using Veins::VehicleSignalSet;

SCENARIO("vehicle signal set gets correctly build correctly from integer bitfield", "[vehiclesignals]")
{
    GIVEN("A valid integer bitfield of vehicle signals")
    {
        uint32_t bitfield = 0;
        INFO("Blinker left is bit " << static_cast<uint32_t>(VehicleSignal::blinker_left));
        bitfield += 1 << static_cast<int>(VehicleSignal::blinker_left);
        INFO("Break light is bit " << static_cast<uint32_t>(VehicleSignal::brakelight));
        bitfield += 1 << static_cast<int>(VehicleSignal::brakelight);
        INFO("native bitfield: " << std::bitset<32>(bitfield).to_string());
        CAPTURE(sizeof(size_t));

        THEN("A vehicle signal set constructed from it has the right flags set")
        {
            VehicleSignalSet signals{bitfield};
            CAPTURE(signals.to_string());
            REQUIRE(signals.test(VehicleSignal::blinker_left));
            REQUIRE(signals.test(VehicleSignal::brakelight));

            AND_THEN("the vehicle signal set can be converted back to the originial traci int")
            {
                REQUIRE(signals.to_ulong() == bitfield);
            }
        }
    }
}

SCENARIO("vehicle signal set produces correct integer bitfield", "[vehiclesignals]")
{
    GIVEN("A vehicle signal set with enabled blinker right and backdrive")
    {
        VehicleSignalSet signals;
        signals.set(VehicleSignal::blinker_right);
        signals.set(VehicleSignal::backdrive);

        THEN("The resulting bitset should be 000000010000001")
        {
            REQUIRE(signals.to_string() == "000000010000001");
        }
    }
    GIVEN("A vehicle signal set to be undefined")
    {
        VehicleSignalSet signals;
        signals.set(VehicleSignal::undefined);

        THEN("The resulting bitset should be 100000000000000")
        {
            REQUIRE(signals.to_string() == "100000000000000");
        }
    }
    GIVEN("A vehicle signal set with no signal set")
    {
        VehicleSignalSet signals;
        THEN("The signal set should be equal to zero")
        {
            REQUIRE(signals.to_ulong() == 0);
        }
    }
}

SCENARIO("vehicle signal sets conform to set logic", "[vehiclesignals]")
{
    GIVEN("Two distinct VehicleSignals and VehicleSignalSets made from them respectively")
    {
        const auto bit_a = VehicleSignal::blinker_left;
        const auto bit_b = VehicleSignal::blinker_right;
        const VehicleSignalSet set_a(bit_a);
        const VehicleSignalSet set_b(bit_b);

        THEN("OR-ing sets containing only them gives a set with both signals set")
        {
            auto res = set_a | set_b;
            REQUIRE(res.test(bit_a));
            REQUIRE(res.test(bit_b));
        }

        THEN("OR-ing the bits themselves gives a set with both signals set")
        {
            auto res = bit_a | bit_b;
            REQUIRE(res.test(bit_a));
            REQUIRE(res.test(bit_b));
        }

        THEN("OR-ing one of the sets with the other bit gives a set with both signals set")
        {
            auto set_or_bit = set_a | bit_b;
            REQUIRE(set_or_bit.test(bit_a));
            REQUIRE(set_or_bit.test(bit_b));
            auto bit_or_set = bit_a | set_b;
            REQUIRE(bit_or_set.test(bit_a));
            REQUIRE(bit_or_set.test(bit_b));
        }

        THEN("Creating a VehicleSignalSet using an initializer list contains the two signals")
        {
            auto list_initialized = VehicleSignalSet({bit_a, bit_b});
            REQUIRE(list_initialized.test(bit_a));
            REQUIRE(list_initialized.test(bit_b));
        }

        THEN("Two VehicleSignalSets created from the same signal(s) are equal")
        {
            auto second_set_a = VehicleSignalSet({bit_a});
            REQUIRE(second_set_a == set_a);
            REQUIRE(set_a == second_set_a);
        }
    }
}
