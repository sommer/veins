#include "catch2/catch.hpp"

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

        WHEN("changing its heading by a yaw angle by 30 degrees")
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
}
