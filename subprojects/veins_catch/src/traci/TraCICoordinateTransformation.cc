#include "catch/catch.hpp"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"

using Veins::TraCICoordinateTransformation;
using Veins::TraCICoord;
using OmnetCoord = TraCICoordinateTransformation::OmnetCoord;
using TraCICoordList = TraCICoordinateTransformation::TraCICoordList;
using OmnetCoordList = TraCICoordinateTransformation::OmnetCoordList;

SCENARIO( "TraCICoordinateTransformation gets built with network boundaries", "[netbound]") {
    GIVEN( "A network size from (5, 10) to (105, 210) with margin 10" ) {
        Veins::TraCICoord topleft(5, 10);
        Veins::TraCICoord bottomright(105, 210);
        float margin = 10;
        THEN( "A NetworkCoordinateTranslator can be built from it" ) {
            TraCICoordinateTransformation nb{topleft, bottomright, margin};
            REQUIRE( &nb );
        }
    }
}

SCENARIO( "TraCICoordinateTransformation correctly translates for Erlangen boundaries", "[netbound]") {
    auto orig_omnet = OmnetCoord(2414.90142, 1578.44161, 0.0);
    auto orig_traci = TraCICoord(646854.991, 5493242.54);
    OmnetCoordList orig_omnet_list = { orig_omnet };
    TraCICoordList orig_traci_list = { orig_traci };
    double orig_traci_angle = -114.542;
    double orig_omnet_angle = -2.71324;

    GIVEN( "The boundaries from the Erlangen scenario" ) {
        TraCICoordinateTransformation nb{ {644465.09, 5491786.25}, {647071.55,5494795.98}, 25 };

        // single coordinates
        THEN( "omnet coords correctly translate to traci coords" ) {
            auto new_traci = nb.omnet2traci(orig_omnet);
            REQUIRE( new_traci.x == Approx(orig_traci.x) );
            REQUIRE( new_traci.y == Approx(orig_traci.y) );
        }
        THEN( "traci coords correctly translate to omnet coords" ) {
            auto new_omnet = nb.traci2omnet(orig_traci);
            REQUIRE( new_omnet.x == Approx(orig_omnet.x) );
            REQUIRE( new_omnet.y == Approx(orig_omnet.y) );
        }

        // lists of coordinates
        THEN( "omnet coord lists correctly translate to traci coord lists" ) {
            auto new_traci = nb.omnet2traci(orig_omnet_list);
            REQUIRE( new_traci.front().x == Approx(orig_traci.x) );
            REQUIRE( new_traci.front().y == Approx(orig_traci.y) );
        }
        THEN( "traci coord lists correctly translate to omnet coord lists" ) {
            auto new_omnet = nb.traci2omnet(orig_traci_list);
            REQUIRE( new_omnet.front().x == Approx(orig_omnet.x) );
            REQUIRE( new_omnet.front().y == Approx(orig_omnet.y) );
        }

        // angles
        THEN( "omnet angles correctly translate to traci angles" ) {
            auto new_traci = nb.omnet2traciAngle(orig_omnet_angle);
            REQUIRE( new_traci == Approx(orig_traci_angle) );
        }
        THEN( "traci angles correctly translate to omnet angles" ) {
            auto new_omnet = nb.traci2omnetAngle(orig_traci_angle);
            REQUIRE( new_omnet == Approx(orig_omnet_angle) );
        }
    }
}
