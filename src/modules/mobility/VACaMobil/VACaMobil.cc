//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "VACaMobil.h"
#include "TraCIConstants.h"
#include <iostream>
#include <fstream>
#include "BaseMobility.h"

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("ChooseVACaMobilMode");
    if (!e) enums.getInstance()->add(e = new cEnum("ChooseVACaMobilMode"));
    e->insert(VACaMobil::STEADYSTATE, "SteadyState");
    e->insert(VACaMobil::RANDOM, "RandomGeneration");
);
Define_Module(VACaMobil);

void VACaMobil::initialize(int stage)
{
    TraCIScenarioManagerLaunchd::initialize(stage);
    if(stage == 1){
        vehicleSequenceId = 0;
        lastReturnedRoute = 0;
        targetNumber = 0;
        initialized = false;
        lastDowntime = simTime();

        RandomMode = false;
        const char *ModeStr = par("chooseVACaMobilMode").stringValue();
        int Mode = cEnum::get("ChooseVACaMobilMode")->lookup(ModeStr);

        if (Mode == -1)
               throw cRuntimeError("Invalid chooseVACaMobilMode: '%s'", ModeStr);
        if(Mode == VACaMobil::RANDOM) {
            RandomMode = true;
        }

        RandomAddVehicle = NULL;

        if(RandomMode) {
            RandomAddVehicle = new cMessage("AddVehicle");
        }

        totalActualMean = 0;
        countActualMean = 0;
        vRates = par("vehicleRates").stdstringValue().c_str();
        userMean = (int) par("meanNumberOfCars").doubleValue();
        userMean = (userMean >= 0) ? userMean : 0;
        carHysteresisValue = (int) par("carHysteresisValue").doubleValue();
        carHysteresisValue = (carHysteresisValue >= 0) ? carHysteresisValue : 0;

        doNothing = par("doNothing").boolValue();

        int minimalWarmupTime = firstStepAt.dbl();
        warmUpSeconds = par("warmUpSeconds").doubleValue();
        if(warmUpSeconds < minimalWarmupTime){
            warmUpSeconds = simulation.getWarmupPeriod().dbl();
            if(warmUpSeconds < minimalWarmupTime){
                warmUpSeconds = minimalWarmupTime;
            }
        }
        tooManyCars = false;

        WATCH(activeVehicleCount);
        WATCH(onSimulationCars);
        WATCH(warmUpSeconds);
        WATCH(targetNumber);
        WATCH(tooManyCars);

        onSimulationCarsSignal = registerSignal("onSimulationCarsSignal");
    }
}

void VACaMobil::handleMessage(cMessage *msg)
{
    if (msg == RandomAddVehicle) {
            addCarWholeMap();
            scheduleAt(simTime().dbl()+par("interArrivalTime").doubleValue(), RandomAddVehicle);
    } else {
    TraCIScenarioManagerLaunchd::handleMessage(msg);
    }
    bool canAddCar = true;
    onSimulationCars = activeVehicleCount + teleportedVehiclesCount;

    if (msg == executeOneTimestepTrigger){
        if(!doNothing){
            if(!initialized) {
                retrieveInitialInformation();
                initialized = true;
                if(RandomMode) {
                    scheduleAt(simTime(), RandomAddVehicle);
            }
            }
            if(!RandomMode) {
            if(simTime() <= warmUpSeconds){
                canAddCar = warmupPeriodAddCars();
            } else {
                int numberOfCarsToAdd = carsToAdd();
                if(numberOfCarsToAdd > 0){
                    canAddCar = AddCarsUntil(0,userMean - carHysteresisValue);
                    onSimulationCars = activeVehicleCount + teleportedVehiclesCount;
                    canAddCar = AddCarsUntil(timeLimitToAdd, targetNumber);
                }
            }
        }
        onSimulationCars = activeVehicleCount + teleportedVehiclesCount;
            ASSERT(!RandomMode || onSimulationCars >= (userMean - carHysteresisValue) || (onSimulationCars <= userMean + carHysteresisValue));
        totalActualMean = totalActualMean + onSimulationCars;
        countActualMean++;
        emit(onSimulationCarsSignal, onSimulationCars);
    }
        
    }
    if(!RandomMode) {
    ASSERT2(canAddCar, "A new car cannot be added, check the number of routes");
}
}

void VACaMobil::retrieveInitialInformation(){

    //Get routes from SUMO
    std::list<std::string> routeIds = commandGetRouteIds();
    routeNames.clear();
    routeNames.reserve(routeIds.size());
    routeNames.insert(routeNames.end(),routeIds.begin(),routeIds.end());

    EV << "Found " << routeIds.size() << " routes"<< endl;

    routes = new std::map<std::string, std::list<std::string> >();
    for(std::list<std::string>::iterator it = routeIds.begin(); it != routeIds.end();it++){
        std::string route = *it;
        std::list<std::string> edgeList = commandGetRouteEdgeIds(route);
        routes->insert(std::make_pair(route, edgeList));
    }
    EV << routes->size() << " routes added.\n";

    //Get lanes from SUMO
    std::list<std::string> laneIds = commandGetLaneIds();
    edges = new std::map<std::string, std::list<std::string>* >();
    for(std::list<std::string>::iterator it = laneIds.begin(); it != laneIds.end();it++){
        std::string lane = *it;
        std::string edge = commandGetLaneEdgeId(lane);
        std::map<std::string, std::list<std::string>*>::iterator itLanes = edges->find(edge);
        std::list<std::string>* lanes;
        if(itLanes != edges->end()){
            lanes = itLanes->second;
            edges->erase(edge);
        } else {
            lanes = new std::list<std::string>();
        }
        lanes->push_back(lane);
        edges->insert(std::make_pair(edge, lanes));
    }
    EV << edges->size() << " edges added.\n";

    retrieveVehicleInformation();
}

/*
 * Get information from traci
 * Sorts vehicles by probability and also normalizes it.
 */
void VACaMobil::retrieveVehicleInformation()
{
    //Get the different vehicles types defined in SUMO and assign them a probability
    cStringTokenizer tokenizer(vRates);
    const char *token;

    std::list<std::string> vehiclesTraCI = commandGetVehicleIds();
        if(debug) {
            EV << "Found "<< vehiclesTraCI.size() << " vehicles."<< std::endl;
            EV << "VRATES " << vRates << std::endl;
        }
    double totalRate = 0;
    std::list<Typerate> vehicles;
    std::list<std::string>::iterator it = vehiclesTraCI.begin();
    while (it != vehiclesTraCI.end()){
        std::string type = *it;
        double rate = 0;
        const char * substr;
        if((substr = strstr(vRates, type.c_str())) != NULL){
            char * finalPtr;
            substr = substr + strlen(type.c_str()) + 1;
            cStringTokenizer minitokenizer(substr);
            rate = strtod(minitokenizer.nextToken(), &finalPtr);
        } else {
            token = tokenizer.nextToken();
            char * finalPtr;
            while (token != NULL && strstr(token, "=") != NULL){
                token = tokenizer.nextToken();
            }
            if(token != NULL){
                rate = strtod(token, &finalPtr);
            }
        }
        EV << "Vehicle"<< type.c_str()<< rate;
        vehicles.push_back(Typerate{type, rate});
        totalRate = totalRate + rate;
        it++;
    }
    EV << vehicles.size() << " vehicles added.\n";

    //Sort vehicles
    std::list<Typerate>::iterator itVehicles;
    for( itVehicles = vehicles.begin(); itVehicles != vehicles.end(); itVehicles++){
        Typerate aux = *itVehicles;
        aux.rate = aux.rate / totalRate;
        this->vehicles.insert(aux);
    }
}

Coord VACaMobil::commandGetPosition(std::string nodeId) {
    return genericGetCoord(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}

/*
 * Returns all the route id names of SUMO
 */
std::list<std::string> VACaMobil::commandGetRouteIds(){
    return genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);
}

/*
 * Returns all the vehicle id names of SUMO
 */
std::list<std::string> VACaMobil::commandGetVehicleIds(){
    return genericGetStringList(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}

/*
 * This function returns the number of cars to add.
 * Linear probability of introducing a new car, directly related with the distance to the "meanCars - Hysteresis" value
 */
int VACaMobil::carsToAdd(void) {
    int newCars = 0;

    uint32_t upperBound = userMean+carHysteresisValue;
    uint32_t lowerBound = userMean-carHysteresisValue;
    lowerBound = (0 > lowerBound) ? 0 : lowerBound;

    //Check .
    bool getNewTarget;
    if(tooManyCars && (targetNumber >= (unsigned int)onSimulationCars)) {
        getNewTarget = true;
    }
    else if(!tooManyCars && (targetNumber <= (unsigned int)onSimulationCars)){
            getNewTarget = true;
    }
    else{
        getNewTarget = false;
    }

    if(getNewTarget || targetNumber <= lowerBound) {
        double actualMean = (totalActualMean/(double) countActualMean);
        double biasedMean = userMean + (userMean - actualMean);//slightly biased mean, to quickly converge to desired values

        targetNumber = (int)(normal(biasedMean, carHysteresisValue/3.0)+0.5); //+0.5 to round up, the number of vehicles must be an integer

        targetNumber = (targetNumber > upperBound) ? upperBound : targetNumber;
        targetNumber = (targetNumber < lowerBound) ? lowerBound : targetNumber;

        //define the "introducing new cars" interval duration
        double intervalDuration = simTime().dbl() - lastDowntime.dbl();//Get the last "idle" interval duration
        timeLimitToAdd = simTime().dbl() + intervalDuration;

        newCars = targetNumber - onSimulationCars;

        lastDowntime = simTime();
        tooManyCars = (newCars < 0);
    } else {
        newCars = targetNumber - onSimulationCars;
    }
    return newCars;
}

/*
 * This function adds the necessary cars at the warmup period
 */
bool VACaMobil::warmupPeriodAddCars() {

    return AddCarsUntil(warmUpSeconds, userMean);
}

/*
 * Add (objectiveNumber-currentNumberOfCars)/(finalTime-simTime()) cars every step between simTime and finalTime
 */
bool VACaMobil::AddCarsUntil(double finalTime, int objectiveNumber) {
    bool sucess = true;

    double remainingTime = finalTime - simTime().dbl();
    int remainingCarsToAdd = objectiveNumber - onSimulationCars;

    int carsToAdd;
    int steps;
    steps = remainingTime / updateInterval.dbl();
    if(steps > 0) {
        carsToAdd = round((remainingCarsToAdd)/(double)steps);
    } else {
        carsToAdd = remainingCarsToAdd;
    }

    for( int i = 0; i < carsToAdd && sucess; i++) {
        sucess = addCarWholeMap();
    }

    return sucess;
}


/*
 * Add a new vehicle using any of the defined routes
 * if not possible return false
 */
bool VACaMobil::addCarWholeMap(void) {
    bool carAdded = false;

    char vehicleString[1000];
    sprintf(vehicleString,"vehicleRandom%d", ++vehicleSequenceId);

    //Get vehicleType
    std::string actualType = getRandomVehicleType();

    EV << "Adding new vehicle: Type=" << actualType.c_str() << std::endl;
    std::string actualRoute = getRandomRoute();

    int firstRoute = lastReturnedRoute;
    do {
        std::list<std::string> lanes = getFirstEdgeLanes(actualRoute);
        for(std::list<std::string>::iterator laneIterator = lanes.begin(); laneIterator != lanes.end() && !carAdded; laneIterator++) {
            carAdded = TraCIScenarioManager::commandAddVehicle(vehicleString, actualType.c_str(), actualRoute.c_str(), laneIterator->c_str(), 0.0, 0.0);
        }
        actualRoute = getNextRoute();
    }while(!carAdded && firstRoute != lastReturnedRoute);

    return carAdded;
}

/*
 * Tries to add a car.
 * Returns true or false indicating whether if it was successful or not.
 */
bool VACaMobil::addCar()
{
    char vehicleString[1000];
    sprintf(vehicleString,"vehicleRandom%d", ++vehicleSequenceId);

    //Get vehicleType
    std::string actualType = getRandomVehicleType();

    //Get route
    std::string actualRoute = getRandomRoute();

    //Get lane
    std::string actualLane = getRandomLaneFromRoute(actualRoute);

    //Add car
    return TraCIScenarioManager::commandAddVehicle(vehicleString, actualType.c_str(), actualRoute.c_str(), actualLane.c_str(), 0.0, 0.0);
}

/*
 * Returns the string containing the ID Name of one of the different vehicle types
 * considering the defined probabilities
 */
std::string VACaMobil::getRandomVehicleType(void)
{

    double probability = uniform(0,1);
    double acumulated = 0.0;
    std::string lastType;

    std::set<Typerate, Comp>::iterator it = vehicles.begin();
    while(it != vehicles.end()) {
        acumulated += it->rate;
        if(probability <= acumulated) {

            return it->type;
        }
        it++;
    }
    return vehicles.rbegin()->type;

}

/*
 * Returns the string containing the ID Name of one of the different routes
 */
std::string VACaMobil::getRandomRoute(void)
{
    double probability = uniform(0,routeNames.size());
    int routeSelected = (int)probability % routeNames.size();

    lastReturnedRoute = routeSelected;

    return routeNames.at(routeSelected);
}

/*
 * Returns a new route, being the route after the last call to "getRouteName" or "getNextRoute"
 */
std::string VACaMobil::getNextRoute(void) {

    lastReturnedRoute = (lastReturnedRoute + 1) % routeNames.size();

    return routeNames.at(lastReturnedRoute);
}

/*
 * Returns the string containing the ID Name of one lane at the first edge of the route "routeName"
 */
std::string VACaMobil::getRandomLaneFromRoute(std::string routeName)
{
    std::list<std::string> lanes = getFirstEdgeLanes(routeName);

    int laneNumber = lanes.size();

    double probability = uniform(0,laneNumber);
    int selectedLane = (int)probability % laneNumber;

    std::vector<std::string> orderedLanes;

    int i = 0;
    for(std::list<std::string>::iterator it = lanes.begin(); it != lanes.end(); it++) {
        if(i++ == selectedLane) {
            return *it;
        }
    }

    return lanes.front();
}

/*
 * Returns a list of lane Ids corresponding to the first edge of the route
 */
std::list<std::string> VACaMobil::getFirstEdgeLanes(std::string routeName) {
    std::string firstEdge = routes->find(routeName)->second.front();
    std::list<std::string> lanes = *(edges->find(firstEdge)->second);

    return lanes;
}

void VACaMobil::finish(){
    if(RandomMode && RandomAddVehicle != NULL) {
        cancelAndDelete(RandomAddVehicle);
        RandomAddVehicle = NULL;
    }
    TraCIScenarioManagerLaunchd::finish();
}

