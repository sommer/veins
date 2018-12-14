//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <algorithm>

#include "veins_testsims/traci/TraCITestApp.h"

#include "veins_testsims/utils/asserts.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins_testsims/traci/TraCITrafficLightTestLogic.h"

using Veins::BaseMobility;
using Veins::TraCIMobility;
using Veins::TraCIMobilityAccess;
using Veins::TraCITestApp;

Define_Module(Veins::TraCITestApp);

void TraCITestApp::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if (stage == 0) {
        testNumber = par("testNumber");
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);

        visitedEdges.clear();
        hasStopped = false;

        EV_DEBUG << "TraCITestApp initialized with testNumber=" << testNumber << std::endl;
    }
}

void TraCITestApp::finish()
{
}

void TraCITestApp::handleSelfMsg(cMessage* msg)
{
}

void TraCITestApp::handleLowerMsg(cMessage* msg)
{
    delete msg;
}

void TraCITestApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        handlePositionUpdate();
    }
}

void TraCITestApp::handlePositionUpdate()
{
    const simtime_t t = simTime();
    const std::string roadId = mobility->getRoadId();
    visitedEdges.insert(roadId);

    int testCounter = 0;

    //
    // TraCIMobility
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCIMobility::getHeading) returns 0 (east)", mobility->getHeading().getRad(), 0);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("TraCIMobility::getHostSpeed magnitude is same as TraCIMobility::getSpeed", mobility->getHostSpeed().length(), mobility->getSpeed());
        }
    }

    //
    // TraCICommandInterface
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertTrue("(TraCICommandInterface::getVersion)", traci->getVersion().first >= 10);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::getLonLat)", -25.0, traci->getLonLat(Coord(0, 0)).first);
            assertClose("(TraCICommandInterface::getLonLat)", 125.0, traci->getLonLat(Coord(0, 0)).second);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            auto o = traci->getRoadMapPos(Coord(100, 100));
            assertEqual("(TraCICommandInterface::getRoadMapPos)", "25", std::get<0>(o));
            assertClose("(TraCICommandInterface::getRoadMapPos)", 75.0, std::get<1>(o));
            assertEqual("(TraCICommandInterface::getRoadMapPos)", 0, std::get<2>(o));
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::getDistance) air", 859., floor(traci->getDistance(Coord(25, 7030), Coord(883, 6980), false)));
            assertClose("(TraCICommandInterface::getDistance) driving", 847., floor(traci->getDistance(Coord(25, 7030), Coord(883, 6980), true)));
        }
    }

    if (testNumber == testCounter++) {
        if (t == 28) {
            bool r = traci->addVehicle("testVehicle0", "vtype0", "route0");
            assertTrue("(TraCICommandInterface::addVehicle) command reports success", r);
        }
        if (t == 30) {
            std::map<std::string, cModule*>::const_iterator i = mobility->getManager()->getManagedHosts().find("testVehicle0");
            bool r = (i != mobility->getManager()->getManagedHosts().end());
            assertTrue("(TraCICommandInterface::addVehicle) vehicle now driving", r);
            const cModule* mod = i->second;
            const TraCIMobility* traci2 = FindModule<TraCIMobility*>::findSubModule(const_cast<cModule*>(mod));
            assertTrue("(TraCICommandInterface::addVehicle) vehicle driving at speed", traci2->getSpeed() > 25);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 30) {
            std::list<std::string> lanes = traci->getLaneIds();
            assertTrue("(TraCICommandInterface::getLaneIds) returns test lane", std::find(lanes.begin(), lanes.end(), "10_0") != lanes.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<std::string> polys = traci->getPolygonIds();
            assertEqual("(TraCICommandInterface::getPolygonIds) number is 1", polys.size(), (size_t) 1);
            assertEqual("(TraCICommandInterface::getPolygonIds) id is correct", *polys.begin(), "poly0");
        }
    }

    if (testNumber == testCounter++) {
        if (t == 30) {
            std::list<Coord> points;
            points.push_back(Coord(100, 100));
            points.push_back(Coord(200, 100));
            points.push_back(Coord(200, 200));
            points.push_back(Coord(100, 200));
            traci->addPolygon("testPoly", "testType", TraCIColor::fromTkColor("red"), true, 1, points);
        }
        if (t == 31) {
            std::list<std::string> polys = traci->getPolygonIds();
            assertEqual("(TraCICommandInterface::addPolygon, TraCICommandInterface::getPolygonIds) number is 2", polys.size(), (size_t) 2);
            assertTrue("(TraCICommandInterface::addPolygon, TraCICommandInterface::getPolygonIds) ids contain added", std::find(polys.begin(), polys.end(), std::string("testPoly")) != polys.end());
            std::string typeId = traci->polygon("testPoly").getTypeId();
            assertEqual("(TraCICommandInterface::Polygon::getTypeId) typeId is correct", typeId, "testType");
            std::list<Coord> shape = traci->polygon("testPoly").getShape();
            assertClose("(TraCICommandInterface::Polygon::getShape) shape x coordinate is correct", 100.0, shape.begin()->x);
            assertClose("(TraCICommandInterface::Polygon::getShape) shape y coordinate is correct", 100.0, shape.begin()->y);
        }
        if (t == 32) {
            traci->polygon("testPoly").remove(1);
        }
        if (t == 33) {
            std::list<std::string> polys = traci->getPolygonIds();
            assertEqual("(TraCICommandInterface::Polygon::remove) number is 1", polys.size(), (size_t) 1);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<std::string> o = traci->getRouteIds();
            assertEqual("(TraCICommandInterface::getRouteIds) number is 1", (size_t) 1, o.size());
            assertEqual("(TraCICommandInterface::getRouteIds) id is correct", "route0", *o.begin());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<std::string> o = traci->getVehicleTypeIds();
            assertTrue("(TraCICommandInterface::getVehicleTypeIds) number is at least 1", o.size() >= 1);
            assertTrue("(TraCICommandInterface::getVehicleTypeIds) has vtype0", std::find(o.begin(), o.end(), "vtype0") != o.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<std::string> o = traci->getTrafficlightIds();
            assertEqual("(TraCICommandInterface::getTrafficlightIds) number is 1", (size_t) 1, o.size());
            assertEqual("(TraCICommandInterface::getTrafficlightIds) id is correct", "10", *o.begin());
        }
    }

    //
    // TraCICommandInterface::Poi
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<std::string> o = traci->getPoiIds();
            assertEqual("(TraCICommandInterface::getPoiIds) number is 0", (size_t) 0, o.size());
        }
        if (t == 2) {
            traci->addPoi("poi0", "building", TraCIColor::fromTkColor("red"), 0, Coord(0, 0));
        }
        if (t == 3) {
            std::list<std::string> o = traci->getPoiIds();
            assertEqual("(TraCICommandInterface::addPoi) number is 1", (size_t) 1, o.size());
        }
        if (t == 4) {
            traci->poi("poi0").remove(0);
        }
        if (t == 5) {
            std::list<std::string> o = traci->getPoiIds();
            assertEqual("(TraCICommandInterface::Poi::remove) number is 0", (size_t) 0, o.size());
        }
    }

    //
    // TraCICommandInterface::Vehicle
    //

    if (testNumber == testCounter++) {
        if (t == 9) {
            assertTrue("(TraCICommandInterface::Vehicle::setSpeed) vehicle is driving", mobility->getSpeed() > 25);
        }
        if (t == 10) {
            traciVehicle->setSpeedMode(0x00);
            traciVehicle->setSpeed(0);
        }
        if (t == 11) {
            assertClose("(TraCICommandInterface::Vehicle::setSpeed, TraCICommandInterface::Vehicle::setSpeedMode) vehicle has stopped", 0.0, mobility->getSpeed());
        }
        if (t == 12) {
            traciVehicle->setSpeedMode(0xff);
            traciVehicle->setSpeed(-1);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->changeRoute("42", 9999);
            traciVehicle->changeRoute("43", 9999);
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Vehicle::changeRoute, 9999) vehicle avoided 42", visitedEdges.find("42") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::changeRoute, 9999) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::changeRoute, 9999) vehicle took 44", visitedEdges.find("44") != visitedEdges.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->changeRoute("42", 9999);
            traciVehicle->changeRoute("43", 9999);
        }
        if (t == 3) {
            traciVehicle->changeRoute("42", -1);
            traciVehicle->changeRoute("44", 9999);
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Vehicle::changeRoute, -1) vehicle took 42", visitedEdges.find("42") != visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::changeRoute, -1) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::changeRoute, -1) vehicle avoided 44", visitedEdges.find("44") == visitedEdges.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->vehicle(mobility->getExternalId()).stopAt("43", 20, 0, 10, 30);
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Vehicle::stopAt) vehicle is at 43", roadId == "43");
            assertClose("(TraCICommandInterface::Vehicle::stopAt) vehicle is stopped", 0.0, mobility->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCICommandInterface::Vehicle::getTypeId)", traciVehicle->getTypeId(), "vtype0");
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->setMaxSpeed(1 / 3.6);
        }
        if (t == 30) {
            assertClose("(TraCICommandInterface::Vehicle::setMaxSpeed)", 0.2280344208055693489, mobility->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            auto o = traciVehicle->getColor();
            assertEqual("(TraCICommandInterface::Vehicle::getColor)", 255, (int) o.red);
            assertEqual("(TraCICommandInterface::Vehicle::getColor)", 255, (int) o.green);
            assertEqual("(TraCICommandInterface::Vehicle::getColor)", 0, (int) o.blue);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->setColor(TraCIColor::fromTkColor("red"));
        }
        if (t == 2) {
            auto o = traciVehicle->getColor();
            assertEqual("(TraCICommandInterface::Vehicle::setColor)", 255, (int) o.red);
            assertEqual("(TraCICommandInterface::Vehicle::setColor)", 0, (int) o.green);
            assertEqual("(TraCICommandInterface::Vehicle::setColor)", 0, (int) o.blue);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->slowDown(1 / 3.6, simtime_t(61));
        }
        if (t == 60) {
            assertTrue("(TraCICommandInterface::Vehicle::slowDown)", mobility->getSpeed() < 10 / 3.6);
        }
        if (t == 70) {
            assertTrue("(TraCICommandInterface::Vehicle::slowDown)", mobility->getSpeed() > 10 / 3.6);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->newRoute("44");
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Vehicle::newRoute) vehicle avoided 42", visitedEdges.find("42") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::newRoute) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::newRoute) vehicle took 44", visitedEdges.find("44") != visitedEdges.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            // TODO: broken in SUMO 1.0.0?
            // traciVehicle->setParking();
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCICommandInterface::Vehicle::getRoadId)", "25", traciVehicle->getRoadId());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCICommandInterface::Vehicle::getLaneId)", "25_0", traciVehicle->getLaneId());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getMaxSpeed)", 70.0, traciVehicle->getMaxSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getLanePosition)", 2.6, traciVehicle->getLanePosition());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            auto o = traciVehicle->getPlannedRoadIds();
            assertEqual("(TraCICommandInterface::Vehicle::getPlannedRoadIds) count", 11, o.size());
            assertEqual("(TraCICommandInterface::Vehicle::getPlannedRoadIds) begin", "25", *o.begin());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCICommandInterface::Vehicle::getRouteId)", "route0", traciVehicle->getRouteId());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCICommandInterface::Vehicle::getLaneIndex)", 0, traciVehicle->getLaneIndex());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traciVehicle->changeVehicleRoute({"25", "28", "31", "34", "37", "40", "13", "44"});
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Vehicle::newRoute) vehicle avoided 42", visitedEdges.find("42") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::newRoute) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
            assertTrue("(TraCICommandInterface::Vehicle::newRoute) vehicle took 44", visitedEdges.find("44") != visitedEdges.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getLength)", 2.5, traciVehicle->getLength());
            assertClose("(TraCICommandInterface::Vehicle::getWidth)", 1.8, traciVehicle->getWidth());
            assertClose("(TraCICommandInterface::Vehicle::getHeight)", 1.5, traciVehicle->getHeight());
            assertClose("(TraCICommandInterface::Vehicle::getAccel)", 3.0, traciVehicle->getAccel());
            assertClose("(TraCICommandInterface::Vehicle::getDeccel)", 9.81, traciVehicle->getDeccel());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getCO2Emissions)", 6150.260674767667297, traciVehicle->getCO2Emissions());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getCOEmissions)", 91.03191431262455069, traciVehicle->getCOEmissions());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getHCEmissions)", 0.574484705658927175, traciVehicle->getHCEmissions());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getPMxEmissions)", 0.1398785382084307971, traciVehicle->getPMxEmissions());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getNOxEmissions)", 2.106876077669999958, traciVehicle->getNOxEmissions());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getFuelConsumption)", 2.643746754251547593, traciVehicle->getFuelConsumption());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getNoiseEmission)", 74.36439646827733441, traciVehicle->getNoiseEmission());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getElectricityConsumption)", 0.0, traciVehicle->getElectricityConsumption());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getWaitingTime)", 0.0, traciVehicle->getWaitingTime());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Vehicle::getAccumulatedWaitingTime)", 0.0, traciVehicle->getAccumulatedWaitingTime());
        }
    }

    //
    // TraCICommandInterface::Junction
    //

    if (testNumber == testCounter++) {
        if (t == 30) {
            std::list<std::string> junctions = traci->getJunctionIds();
            assertTrue("(TraCICommandInterface::Junction::getJunctionIds) returns test junction", std::find(junctions.begin(), junctions.end(), "1") != junctions.end());
            Coord pos = traci->junction("1").getPosition();
            assertClose("(TraCICommandInterface::Junction::getPosition) shape x coordinate is correct", 25.0, pos.x);
            assertClose("(TraCICommandInterface::Junction::getPosition) shape y coordinate is correct", 75.0, pos.y);
        }
    }

    //
    // TraCICommandInterface::Lane
    //

    if (testNumber == testCounter++) {
        if (t == 30) {
            Coord shape_front_coord = traci->lane("10_0").getShape().front();
            assertClose("(TraCICommandInterface::Lane::getShape) shape x coordinate is correct", 523., floor(shape_front_coord.x));
            assertClose("(TraCICommandInterface::Lane::getShape) shape y coordinate is correct", 79., floor(shape_front_coord.y));
        }
    }

    //
    // TraCICommandInterface::Polygon
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::string typeId = traci->polygon("poly0").getTypeId();
            assertEqual("(TraCICommandInterface::Polygon::getTypeId) typeId is correct", typeId, "type0");
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<Coord> shape = traci->polygon("poly0").getShape();
            assertClose("(TraCICommandInterface::Polygon::getShape) shape x coordinate is correct", 130.0, shape.begin()->x);
            assertClose("(TraCICommandInterface::Polygon::getShape) shape y coordinate is correct", 81.65, shape.begin()->y);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<Coord> shape1 = traci->polygon("poly0").getShape();
            assertClose("(TraCICommandInterface::Polygon::getShape) shape x coordinate is correct", 130.0, shape1.begin()->x);
            assertClose("(TraCICommandInterface::Polygon::getShape) shape y coordinate is correct", 81.65, shape1.begin()->y);
            std::list<Coord> shape2 = shape1;
            shape2.begin()->x = 135;
            shape2.begin()->y = 85;
            traci->polygon("poly0").setShape(shape2);
            std::list<Coord> shape3 = traci->polygon("poly0").getShape();
            assertClose("(TraCICommandInterface::Polygon::setShape, Polygon::getShape) shape x coordinate was changed", 135.0, shape3.begin()->x);
            assertClose("(TraCICommandInterface::Polygon::setShape, Polygon::getShape) shape y coordinate was changed", 85.0, shape3.begin()->y);
        }
    }

    //
    // TraCICommandInterface::Route
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            auto o = traci->route("route0").getRoadIds();
            assertEqual("(TraCICommandInterface::Route::getRoadIds) has 11 entries", o.size(), 11);
            assertEqual("(TraCICommandInterface::Route::getRoadIds) starts with 25", *o.begin(), "25");
        }
    }

    //
    // TraCICommandInterface::Road
    //

    if (testNumber == testCounter++) {
        if (t == 30) {
            assertClose("(TraCICommandInterface::Road::getCurrentTravelTime)", 3.428725701943844406, traci->road("25").getCurrentTravelTime());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 30) {
            assertClose("(TraCICommandInterface::Road::getMeanSpeed)", 27.78, traci->road("25").getMeanSpeed());
        }
    }

    //
    // TraCICommandInterface::Lane
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertEqual("(TraCICommandInterface::Lane::getCurrentTravelTime)", "25", traci->lane("25_0").getRoadId());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(TraCICommandInterface::Lane::getLength)", 95.25, traci->lane("25_0").getLength());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 30) {
            assertClose("(TraCICommandInterface::Lane::getMaxSpeed)", 27.78, traci->lane("25_0").getMaxSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 30) {
            assertClose("(TraCICommandInterface::Lane::getMeanSpeed)", 27.78, traci->lane("25_0").getMeanSpeed());
        }
    }

    //
    // TraCICommandInterface::Trafficlight
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->trafficlight("10").setProgram("myProgramRed");
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Trafficlight::setProgram) vehicle is at 31", roadId == "31");
            assertClose("(TraCICommandInterface::Trafficlight::setProgram) vehicle is stopped", 0.0, mobility->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->trafficlight("10").setProgram("myProgramGreenRed");
            traci->trafficlight("10").setPhaseIndex(1);
        }
        if (t == 30) {
            assertTrue("(TraCICommandInterface::Trafficlight::setProgram, Trafficlight::setPhaseIndex) vehicle is at 31", roadId == "31");
            assertClose("(TraCICommandInterface::Trafficlight::setProgram, Trafficlight::setPhaseIndex) vehicle is stopped", 0.0, mobility->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            assertEqual("(TraCICommandInterface::Trafficlight::getCurrentState)", "rrrrrrGGG", traci->trafficlight("10").getCurrentState());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            assertEqual("(TraCICommandInterface::Trafficlight::getDefaultCurrentPhaseDuration)", simtime_t(999), traci->trafficlight("10").getDefaultCurrentPhaseDuration());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            auto o = traci->trafficlight("10").getControlledLanes();
            assertEqual("(TraCICommandInterface::Trafficlight::getControlledLanes) returns correct count", 9, o.size());
            assertTrue("(TraCICommandInterface::Trafficlight::getControlledLanes) returns test lane", std::find(o.begin(), o.end(), "67_0") != o.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            auto o = traci->trafficlight("10").getControlledLinks();
            assertEqual("(TraCICommandInterface::Trafficlight::getControlledLinks) returns correct count", 9, o.size());
            assertEqual("(TraCICommandInterface::Trafficlight::getControlledLinks) returns correct link", "67_0", o.begin()->begin()->incoming);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            assertEqual("(TraCICommandInterface::Trafficlight::getCurrentPhaseIndex)", 0, traci->trafficlight("10").getCurrentPhaseIndex());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            assertEqual("(TraCICommandInterface::Trafficlight::getCurrentProgramID)", "myProgramGreenRed", traci->trafficlight("10").getCurrentProgramID());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            auto o = traci->trafficlight("10").getProgramDefinition();
            assertTrue("(TraCICommandInterface::Trafficlight::getProgramDefinition)", o.hasLogic("myProgramGreenRed"));
            assertEqual("(TraCICommandInterface::Trafficlight::getProgramDefinition)", "rrrrrrGGG", o.getLogic("myProgramGreenRed").phases.begin()->state);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            assertEqual("(TraCICommandInterface::Trafficlight::getAssumedNextSwitchTime)", simtime_t(999), traci->trafficlight("10").getAssumedNextSwitchTime());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            traci->trafficlight("10").setState("rrrrrrGGr");
        }
        if (t == 6) {
            assertEqual("(TraCICommandInterface::Trafficlight::setState)", "rrrrrrGGr", traci->trafficlight("10").getCurrentState());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            traci->trafficlight("10").setPhaseDuration(simtime_t(1));
        }
        if (t == 8) {
            assertEqual("(TraCICommandInterface::Trafficlight::setPhaseDuration)", "GggGGgrrr", traci->trafficlight("10").getCurrentState());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 5) {
            auto phase = TraCITrafficLightProgram::Phase();
            phase.duration = 999;
            phase.minDuration = 999;
            phase.maxDuration = 999;
            phase.state = "rGrGrGrGr";

            auto logic = TraCITrafficLightProgram::Logic();
            logic.id = "logic0";
            logic.currentPhase = 0;
            logic.phases = {phase};
            logic.type = 0;
            logic.parameter = 0;
            traci->trafficlight("10").setProgramDefinition(logic, 0);
        }
        if (t == 8) {
            assertEqual("(TraCICommandInterface::Trafficlight::setProgramDefinition)", "rGrGrGrGr", traci->trafficlight("10").getCurrentState());
        }
    }

    //
    // TraCICommandInterface::LaneAreaDetector
    //
    if (testNumber == testCounter++) {
        if (t == 30) {
            std::list<std::string> o = traci->getLaneAreaDetectorIds();
            assertEqual("(TraCICommandInterface::getLaneAreaDetectorIds) number is correct", (size_t) 1, o.size());
            assertEqual("(TraCICommandInterface::getLaneAreaDetectorIds) id is correct", "e2", *o.begin());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 2) {
            auto o = traci->laneAreaDetector("e2").getLastStepVehicleNumber();
            assertEqual("(TraCICommandInterface::LaneAreaDetector::getLastStepVehicleNumber) number is correct", 1, o);
        }
    }

    //
    // TraCICommandInterface::GuiView
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            // TODO: cannot be tested (no programmatic feedback)
            // traci->guiView("View #0").setScheme("real world");
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            // TODO: cannot be tested (no programmatic feedback)
            // traci->guiView("View #0").setZoom(200);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            // TODO: cannot be tested (no programmatic feedback)
            // traci->guiView("View #0").setBoundary(Coord(0, 0), Coord(10, 10));
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            // TODO: cannot be tested (no programmatic feedback)
            // traci->guiView("View #0").takeScreenshot();
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            // TODO: cannot be tested (no programmatic feedback)
            // traci->guiView("View #0").trackVehicle("flow0.0");
        }
    }

    //
    // Veins::TraCITrafficLightAbstractLogic (see org.car2x.veins.subprojects.veins_testsims.traci.TraCITrafficLightTestLogic class)
    //

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->trafficlight("10").setState("rrrrrrrrr");
            traci->trafficlight("10").setPhaseDuration(999);
            auto* logic = FindModule<TraCITrafficLightTestLogic*>::findSubModule(getSimulation()->getSystemModule());
            ASSERT(logic);
            logic->startChangingProgramAt(simTime() + 12);
        }
        if (t == 15) {
            assertTrue("Vehicle is supposed to wait in front of a red traffic light", mobility->getSpeed() < 0.1);
        }
        if (t == 25) {
            assertTrue("Vehicle is supposed to drive again (Traffic light turned green)", mobility->getSpeed() > 0.1);
        }
    }

    //
    // End
    //
}
