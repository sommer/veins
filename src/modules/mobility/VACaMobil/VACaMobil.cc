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
#define DENSITY 1
#define HEATMAP_AREA "area.dat"
#define HEATMAP_ROADS "roads.dat"

Define_Module(VACaMobil);

void VACaMobil::initialize(int stage)
{
    TraCIScenarioManagerLaunchd::initialize(stage);
    if(stage == 1){
        vehicleSequenceId = 0;
        lastReturnedRoute = 0;
        carsToAchieve = 0;
        initialized = false;
        lastDowntime = simTime();

        acumMedia = 0;
        countMedia = 0;
        getStats = par("getStatistics").boolValue();

        if(getStats){
            heatmapArea = new std::map<std::pair<int, int>, int >();
            heatmapRoads = new std::map<std::string, int >();
        }

        vRates = par("vehicleRates").stdstringValue().c_str();
        meanNumberOfCars = (int) par("meanNumberOfCars").doubleValue();
        meanNumberOfCars = (meanNumberOfCars >= 0) ? meanNumberOfCars : 0;
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
        goingDown = false;

        WATCH(activeVehicleCount);
        WATCH(onSimulationCars);
        WATCH(warmUpSeconds);
        WATCH(carsToAchieve);
        WATCH(goingDown);

        onSimulationCarsSignal = registerSignal("onSimulationCarsSignal");

        nRandomRsu = par("nRandomRsu");
        namePrefix = par("rsuPrefix").stringValue();

        rsuInitialized = false;
    }
}

void VACaMobil::handleMessage(cMessage *msg)
{

    TraCIScenarioManagerLaunchd::handleMessage(msg);
    bool canAddCar = true;
    onSimulationCars = activeVehicleCount + teleportedVehiclesCount;

    if (msg == executeOneTimestepTrigger){
        if(!doNothing){
            if(!initialized) {
                retrieveInitialInformation();
                initialized = true;
            }
            if(simTime() <= warmUpSeconds){
                canAddCar = warmupPeriodAddCars();
            } else {
                int numberOfCarsToAdd = isGoingToAddCar();
                if(numberOfCarsToAdd > 0){
                    canAddCar = AddCarsUntil(0,meanNumberOfCars - carHysteresisValue);
                    onSimulationCars = activeVehicleCount + teleportedVehiclesCount;
                    canAddCar = AddCarsUntil(timeLimitToAdd, carsToAchieve);
                }
            }
        }
        onSimulationCars = activeVehicleCount + teleportedVehiclesCount;
     //   ASSERT(onSimulationCars > meanNumberOfCars - carHysteresisValue);
     //   ASSERT(onSimulationCars < meanNumberOfCars + carHysteresisValue);
        acumMedia = acumMedia + onSimulationCars;
        countMedia++;
        emit(onSimulationCarsSignal, onSimulationCars);

        if(simTime() > warmUpSeconds && getStats){
            updateHeatmaps();
        }

        if(!rsuInitialized){
            if( nRandomRsu > 0){
                generateRandomRsus(nRandomRsu);
            }

            parseRsu();
            placeRsus();
            rsuInitialized = true;
        }
    }
    ASSERT2(canAddCar, "A new car cannot be added, check the number of routes");
}

void VACaMobil::retrieveInitialInformation(){

    //Recuperamos las rutas
    std::list<std::string> routeIds = commandGetRouteIds();
    routeNames.clear();
    routeNames.reserve(routeIds.size());
    routeNames.insert(routeNames.end(),routeIds.begin(),routeIds.end());

    //printf("Recuperadas %d rutas\n", routeIds.size());
    routes = new std::map<std::string, std::list<std::string> >();
    for(std::list<std::string>::iterator it = routeIds.begin(); it != routeIds.end();it++){
        std::string route = *it;
        std::list<std::string> edgeList = commandGetRouteEdgeIds(route);
        routes->insert(std::make_pair(route, edgeList));
        //printf("Ruta %s %d\n", route.c_str(), edgeList.size());
    }
    EV << routes->size() << " routes added.\n";

    //Recuperamos las lanes y las ordenamos por edges
    std::list<std::string> laneIds = commandGetLaneIds();
    //printf("Recuperadas %d lanes\n", laneIds.size());
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
        //printf("Edge %s %s\n", edge.c_str(), lane.c_str());
    }
    EV << edges->size() << " edges added.\n";

    retrieveVehicleInformation();

}

/*
 * Encapsulates all the vehicle retrieval information from parameters
 * Sorts vehicles by probability and also normalizes it.
 */
void VACaMobil::retrieveVehicleInformation()
{
    //Parseamos la información de los tipos de vehículos
        cStringTokenizer tokenizer(vRates);
        const char *token;

        std::list<std::string> vehiclesTraCI = commandGetVehicleIds();
        //printf("Recuperados %d vehiculos\n", vehiclesTraCI.size());
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
            //printf("Vehiculo %s %f\n", type.c_str(), rate);
            vehicles.push_back(Typerate{type, rate});
            totalRate = totalRate + rate;
            it++;
        }
        EV << vehicles.size() << " vehicles added.\n";

        //Ordenando los vehículos
        std::list<Typerate>::iterator itVehicles;
        for( itVehicles = vehicles.begin(); itVehicles != vehicles.end(); itVehicles++){
            Typerate aux = *itVehicles;
            aux.rate = aux.rate / totalRate;
            this->vehicles.insert(aux);
        }

        //Mostrando los vehiculos por pantalla
//        std::set<struct typerate>::iterator itaux;
//        for( itaux = this->vehicles.begin(); itaux != this->vehicles.end(); itaux++){
//            struct typerate aux = *itaux;
//            printf("Vehiculo %s %f\n", aux.type.c_str(), aux.rate);
//        }
}

void VACaMobil::parseRsu()
{
    cXMLElement* rsuPlacement = par("rsuPlacement").xmlValue();
    std::string rootTag = rsuPlacement->getTagName();
    ASSERT(rootTag == "poas");
    cXMLElementList rsusList = rsuPlacement->getElementsByTagName("poa");

    for (cXMLElementList::const_iterator i = rsusList.begin(); i != rsusList.end(); ++i) {
        cXMLElement* e = *i;
        std::string id;

        double x;
        double y;

        ASSERT(e->getAttribute("id"));
        id = e->getAttribute("id");
        ASSERT(e->getAttribute("x"));
        x = atof(e->getAttribute("x"));
        ASSERT(e->getAttribute("y"));
        y = atof(e->getAttribute("y"));

        Coord omnetLoc = traci2omnet(TraCICoord(x,y));

        rsusLocation.push_back(omnetLoc);
        rsusNames.push_back(id);
    }
}

void VACaMobil::createRsu(Coord pos, std::string name)
{
    cModule *parent = getParentModule();
    std::string type = par("rsuModule");
    cModuleType* moduleType = cModuleType::get(type.c_str());
    if (!moduleType) error("Module Type \"%s\" not found", type.c_str());
    cModule *mod = moduleType->create(name.c_str(), parent);
    mod->finalizeParameters();
    mod->buildInside();

    // pre-initialize TraCIMobility
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
        cModule* submod = iter();
        BaseMobility* mm = dynamic_cast<BaseMobility*>(submod);
        if (!mm) continue;
        mm->par("x") = pos.x;
        mm->par("y") = pos.y;
        mm->par("z") = 0;
    }

    mod->callInitialize();
}

void VACaMobil::generateRandomRsus(uint n)
{
    Coord omnetNetbounds1 = traci2omnet(netbounds1);
    Coord omnetNetbounds2 = traci2omnet(netbounds2);
    std::list<std::string> junctionList = commandGetJunctionIds();
    std::list<Coord> junctionLocations;
    for(std::list<std::string>::iterator i = junctionList.begin(); i != junctionList.end(); i++){
        junctionLocations.push_back(commandGetJunctionPosition(*i));
    }

    for(uint i=0; i <n ; i++){
        Coord desiredCoord;
        desiredCoord.x = uniform(omnetNetbounds1.x, omnetNetbounds2.x);
        desiredCoord.y = uniform(omnetNetbounds1.y, omnetNetbounds2.y);
        Coord finalCoord = *junctionLocations.begin();
        double minDistance = desiredCoord.distance(*junctionLocations.begin());
        for(std::list<Coord>::iterator iter= ++junctionLocations.begin(); iter != junctionLocations.end(); iter++){
            double distance = desiredCoord.distance(*iter);
            if( distance< minDistance){
                minDistance= distance;
                finalCoord = *iter;
            }
        }
        char name[20];
        snprintf(name, sizeof(name), "randomRsu%d", i);
        rsusLocation.push_back(finalCoord);
        rsusNames.push_back(name);
    }
}

void VACaMobil::placeRsus()
{
    ASSERT(rsusLocation.size() == rsusNames.size());
    for(uint i = 0; i < rsusLocation.size(); i++){
        std::string prefix = namePrefix;
        createRsu(rsusLocation.at(i), prefix.append(rsusNames.at(i)));
    }
}

void VACaMobil::updateHeatmaps(){
    std::map<std::string, cModule*>::iterator it;
    for(it = hosts.begin(); it != hosts.end(); it++){
        std::string id = it->first;
        //Roads
        std::string currentEdge = this->commandGetEdgeId(id);
        std::map<std::string,  int >::iterator eit = this->heatmapRoads->find(currentEdge);
        if(eit != this->heatmapRoads->end()){
            eit->second++;
        } else {
            this->heatmapRoads->insert(std::make_pair(currentEdge, 1));
        }
        //Area
        Coord pos = this->commandGetPosition(id);
        std::pair<int,int> currentij = std::make_pair(pos.x / DENSITY, pos.y / DENSITY);
        std::map<std::pair<int,int>,  int >::iterator ait = this->heatmapArea->find(currentij);
        if(ait != this->heatmapArea->end()){
            ait->second++;
        } else {
            this->heatmapArea->insert(std::make_pair(currentij, 1));
        }
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
int VACaMobil::isGoingToAddCar(void) {
    int addCarCount = 0;

    uint32_t upperCarValue = meanNumberOfCars+carHysteresisValue;
    uint32_t lowerCarValue = meanNumberOfCars-carHysteresisValue;
    lowerCarValue = (0 > lowerCarValue) ? 0 : lowerCarValue;

    //This two ifs are the easy part as are deterministic.
    bool conditionAchieved = false;
    if(goingDown && (carsToAchieve >= (unsigned int)onSimulationCars)) {
        conditionAchieved = true;
    } else {
        if(!goingDown && (carsToAchieve <= (unsigned int)onSimulationCars) ){
            conditionAchieved = true;
        }
    }

    if(conditionAchieved || carsToAchieve <= lowerCarValue) {
        double myMeanNumberOfCars = meanNumberOfCars + (meanNumberOfCars - (acumMedia/(double) countMedia));

        carsToAchieve = (int)(normal(myMeanNumberOfCars, carHysteresisValue/3.0)+0.5); //Stdev is carHystereis divided by three to achieve a good number of cars inside the correct values.

        carsToAchieve = (carsToAchieve > upperCarValue) ? upperCarValue : carsToAchieve;
        carsToAchieve = (carsToAchieve < lowerCarValue) ? lowerCarValue : carsToAchieve;

        int extraTime = (int)floor(simTime().dbl() - lastDowntime.dbl());
        extraTime = (extraTime < 1) ? 1 : extraTime;
        timeLimitToAdd = (int)ceil(simTime().dbl()) + extraTime;

        addCarCount = carsToAchieve - onSimulationCars;

        lastDowntime = simTime();
        goingDown = (addCarCount < 0);

        //std::cout << "Cars to Achieve " << carsToAchieve << " addCarCount "<< addCarCount << std::endl;

    } else {
        addCarCount = carsToAchieve - onSimulationCars;
    }
    //std::cout << "Cars to Achieve " << carsToAchieve << " addCarCount "<< addCarCount << " On simulation cars " << onSimulationCars << " VehicleCount "<< activeVehicleCount << " Teleported "<< teleportedVehiclesCount << std::endl;


    return addCarCount;
}

/*
int CarGeneratorManager::isGoingToAddCar(void) {
    int addCarCount = 0;

    uint32_t upperCarValue = meanNumberOfCars+carHysteresisValue;
    uint32_t lowerCarValue = meanNumberOfCars-carHysteresisValue;

    //This two ifs are the easy part as are deterministic.
    if(onSimulationCars >= upperCarValue) {
        addCarCount = 0;
    } else {
        if(onSimulationCars <= lowerCarValue) {
            addCarCount = lowerCarValue - onSimulationCars;
        } else {
            if(countMedia > 0 && meanNumberOfCars > acumMedia / countMedia){
                int carEstimatedValue = onSimulationCars - lowerCarValue;

                int probability = uniform(0,upperCarValue-lowerCarValue);

                if(probability > carEstimatedValue) {
                    addCarCount = 1;
                } else {
                    addCarCount = 0;
                }
            } else {
                addCarCount = 0;
            }
        }
    }

    return addCarCount;
}
*/

/*
 * This function adds the necessary cars at the warmup period
 */
bool VACaMobil::warmupPeriodAddCars() {

    return AddCarsUntil(warmUpSeconds, meanNumberOfCars);
}

/*
 * This function adds cars in a controlled way
 */
bool VACaMobil::AddCarsUntil(double finalTime, int carsToAddAtTheEnd) {
    bool sucess = true;

    double remainingTime = finalTime - simTime().dbl();
    int remainingCarsToAdd = carsToAddAtTheEnd - onSimulationCars;

    int carsToAdd;
    int steps;
    steps = round(remainingTime / updateInterval.dbl());
    if(steps > 0) {
        carsToAdd = round((remainingCarsToAdd)/steps);
    } else {
        carsToAdd = remainingCarsToAdd;
    }

    for( int i = 0; i < carsToAdd && sucess; i++) {
        sucess = addCarWholeMap();
    }

    return sucess;
}


/*
 *Adds a car if there is a place in the map where it can be placed
 */
bool VACaMobil::addCarWholeMap(void) {
    bool carAdded = false;

    char vehicleString[1000];
    sprintf(vehicleString,"vehicleRandom%d", ++vehicleSequenceId);

    //Get vehicleType
    std::string actualType = getVehicleType();

    std::string actualRoute = getRouteName();

    int firstRoute = lastReturnedRoute;
    do {
        std::list<std::string> lanes = getLaneNames(actualRoute);
        for(std::list<std::string>::iterator laneIterator = lanes.begin(); laneIterator != lanes.end() && !carAdded; laneIterator++) {
            carAdded = TraCIScenarioManager::commandAddVehicle(vehicleString, actualType.c_str(), actualRoute.c_str(), laneIterator->c_str(), 0.0, 0.0);
        }
        actualRoute = getNextRoute();
    }while(!carAdded && firstRoute != lastReturnedRoute);

    return carAdded;
}

/*
 * Tries to add a car.
 * Returns true or false indicating whether if its successfull or not.
 */
bool VACaMobil::addCar()
{
    char vehicleString[1000];
    sprintf(vehicleString,"vehicleRandom%d", ++vehicleSequenceId);

    //Get vehicleType
    std::string actualType = getVehicleType();

    //Get route
    std::string actualRoute = getRouteName();

    //Get lane
    std::string actualLane = getLaneName(actualRoute);

    //Add car
    return TraCIScenarioManager::commandAddVehicle(vehicleString, actualType.c_str(), actualRoute.c_str(), actualLane.c_str(), 0.0, 0.0);
}

/*
 * Retuns the string containing the ID Name of one of the different vehicle types
 * considering the defined probabilities
 */
std::string VACaMobil::getVehicleType(void)
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
 * Retuns the string containing the ID Name of one of the different routes
 */
std::string VACaMobil::getRouteName(void)
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
 * Retuns the string containing the ID Name of one lane at the first edge of the route "routeName"
 */
std::string VACaMobil::getLaneName(std::string routeName)
{
    std::list<std::string> lanes = getLaneNames(routeName);

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
std::list<std::string> VACaMobil::getLaneNames(std::string routeName) {
    std::string firstEdge = routes->find(routeName)->second.front();
    std::list<std::string> lanes = *(edges->find(firstEdge)->second);

    return lanes;
}

void VACaMobil::finish(){
    TraCIScenarioManagerLaunchd::finish();
    if(getStats){
        std::string prefix = par("statFiles").stringValue();
        std::ofstream f1(prefix.append(HEATMAP_ROADS).c_str());
        std::map<std::string, int >::iterator eit;
        for(eit = this->heatmapRoads->begin(); eit != this->heatmapRoads->end(); eit++){
            f1 << eit->first << " " << eit->second << endl;
        }
        f1.close();

        prefix = par("statFiles").stringValue();
        std::ofstream f2(prefix.append(HEATMAP_AREA).c_str());
        std::map<std::pair<int, int>, int >::iterator ait;
        for(ait= this->heatmapArea->begin(); ait != this->heatmapArea->end(); ait++){
            for(int i=0; i< ait->second; i++){
                f2 << ait->first.first << " " << ait->first.second << endl;
            }
        }
        f2.close();
    }
}

