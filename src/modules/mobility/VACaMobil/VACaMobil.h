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

#ifndef __INET_CARGENERATORMANAGER_H_
#define __INET_CARGENERATORMANAGER_H_

#include <omnetpp.h>

#include "TraCIScenarioManagerLaunchd.h"

typedef struct _typerate{
    std::string type;
    double rate;
}Typerate;

class Comp {
    public:
        bool operator()(Typerate s1, Typerate s2){
            if(s1.rate < s2.rate ){
                return true;
            } else {
                return false;
            }
        }
};

/**
 * Constant car generator.
 */
class VACaMobil : public TraCIScenarioManagerLaunchd
{
  protected:
    virtual int numInitStages() const { return std::max(TraCIScenarioManagerLaunchd::numInitStages(), 2); }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
  private:
    int meanNumberOfCars;
    int carHysteresisValue;
    int warmUpSeconds;
    int vehicleSequenceId;
    int addCarRetryValue;
    int lastReturnedRoute;
    bool doNothing;
    bool getStats;

    //Variables needed for normal adding cars function
    uint32_t carsToAchieve;
    int timeLimitToAdd;
    simtime_t lastDowntime;
    bool goingDown;

    const char *vRates;

    bool initialized;

    int acumMedia;
    int countMedia;

    simsignal_t onSimulationCarsSignal;
    int onSimulationCars;

    std::vector<std::string> routeNames;
    std::map<std::string, std::list<std::string> > *routes;
    std::map<std::string, std::list<std::string>* > *edges;
    std::set<Typerate, Comp> vehicles;

    std::vector<Coord> rsusLocation;
    std::vector<std::string> rsusNames;
    std::string namePrefix;
    uint nRandomRsu;
    bool rsuInitialized;

    std::map<std::pair<int, int>, int > * heatmapArea;
    std::map<std::string, int > * heatmapRoads;

    void retrieveInitialInformation();
    void retrieveVehicleInformation();

    std::list<std::string> commandGetRouteIds();
    std::list<std::string> commandGetVehicleIds();
    Coord commandGetPosition(std::string nodeId);

    virtual int isGoingToAddCar(void);
    virtual bool warmupPeriodAddCars(void);
    bool AddCarsUntil(double finalTime, int carsToAddAtTheEnd);

    virtual bool addCar(void);
    virtual bool addCarWholeMap(void);
    virtual std::string getVehicleType(void);
    virtual std::string getRouteName(void);
    virtual std::string getNextRoute(void);
    virtual std::string getLaneName(std::string routeName);
    virtual std::list<std::string> getLaneNames(std::string routeName);

    void parseRsu();
    void createRsu(Coord pos, std::string name);
    void generateRandomRsus(uint n);
    void placeRsus();

    virtual void updateHeatmaps();


};
#endif
