#include "catch/catch.hpp"

SCENARIO("Integers can be added and subtracted", "[helloworld]") {

    GIVEN("An integer that is 42") {
        int i = 42;
        REQUIRE(i == 42);

        WHEN("take away one") {
            i -= 1;

            THEN("the integer is 41") {
                REQUIRE(i == 41);
            }
        }

        WHEN("add one") {
            i += 1;

            THEN("the integer is 43") {
                REQUIRE(i == 43);
            }
        }
    }
}
