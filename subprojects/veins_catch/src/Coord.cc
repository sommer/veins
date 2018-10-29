#include "catch/catch.hpp"

#include "veins/base/utils/Coord.h"

using Veins::Coord;

SCENARIO("Coord", "[coord]")
{

    GIVEN("A Coord that is (1,2,3)")
    {
        auto c = Coord(1, 2, 3);

        THEN("its z value is 3")
        {
            REQUIRE(c.z == 3);
        }

        WHEN("changing its yaw angle by 30 degrees")
        {
            auto c2 = c.rotatedYaw(M_PI / 180 * 30);

            THEN("its x value is -1+0.5*sqrt(3)")
            {
                REQUIRE(c2.x == Approx(-1 + 0.5 * sqrt(3)));
            }

            THEN("its y value is 0.5+sqrt(3)")
            {
                REQUIRE(c2.y == Approx(0.5 + sqrt(3)));
            }

            THEN("its z value is still 3")
            {
                REQUIRE(c2.z == 3);
            }
        }

        WHEN("inverting its y axis")
        {
            auto c2 = c.flippedY();

            THEN("its z value is still 3")
            {
                REQUIRE(c2.z == 3);
            }

            THEN("its y value is -2")
            {
                REQUIRE(c2.y == -2);
            }
        }
    }

    GIVEN("A new Coord of length 2 from a 90 degree yaw angle")
    {
        auto c2 = Coord::fromYaw(M_PI / 180 * 90, 2);

        THEN("its value is (0,2,0)")
        {
            REQUIRE(c2.x == Approx(0.0).margin(1e-9));
            REQUIRE(c2.y == Approx(2.0).margin(1e-9));
            REQUIRE(c2.z == 0);
        }
    }

    GIVEN("A Coord that is (1,1,0)")
    {
        auto c = Coord(1, 1, 0);

        WHEN("converting it to yaw")
        {
            auto d = c.toYaw();

            THEN("its yaw value is 45 degrees")
            {
                REQUIRE(d == M_PI / 180 * 45);
            }
        }
    }
}
