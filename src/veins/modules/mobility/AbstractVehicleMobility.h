#ifndef VEINS_MOBILITY_ABSTRACTVEHICLEMOBILITY_H
#define VEINS_MOBILITY_ABSTRACTVEHICLEMOBILITY_H

#include <string>

#include "veins/base/utils/Coord.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"

namespace Veins {

/*
 * @brief Abstract base class for remove synchronized vehicle mobilities
 *
 * Assumptions:
 *  - Vehicle is synchronized to an external simulator (in terms of position and status, at least partially).
 *  - The Mobility implementation stores the state of the vehicle, so that access to the state is const.
 *  - An external entity within Veins updates the vehicle state to maintain synchronization.
 *  - Vehicle has an id with the external simulator (that can be represented as a string).
 *  - Vehicle has a scalar speed on the road.
 *  - Vehicle has a road (that can be identified by a string) on which is resides.
 *  - Vehicle has an orienation in the plane (azimuth).
 *  - Vehicle has a set of indicators/signals that can be toggled individually.
 *  - More specific vehicle attributes should be covered by derivates of this abstract class and their implementations.
 *
 * Open Points:
 *  - X/Y positions
 *  - Elevation
 *  - Parking state
 *  - Antenna and its position offset (relevant to this interface?)
 *
 * Delibereately left out:
 *  - Accidents (should be in the application layer).
 *  - Statistics (not important for accessing the vehicle mobility).
 *  - Access to ScenarioManager/CommandInterface (contradicts the abstractness of this interface).
 *  - OMNeT++ interface implementations (e.g., initialize; is part of the mobility implementation).
 */
class AbstractVehicleMobility {
public:
    enum VehicleSignal {
        // TODO: move this here from TraciScenarioManager class
        // possibly change to enum class (for name encapsulation)
        VEH_SIGNAL_UNDEF = -1,
        VEH_SIGNAL_NONE = 0,
        VEH_SIGNAL_BLINKER_RIGHT = 1,
        VEH_SIGNAL_BLINKER_LEFT = 2,
        VEH_SIGNAL_BLINKER_EMERGENCY = 4,
        VEH_SIGNAL_BRAKELIGHT = 8,
        VEH_SIGNAL_FRONTLIGHT = 16,
        VEH_SIGNAL_FOGLIGHT = 32,
        VEH_SIGNAL_HIGHBEAM = 64,
        VEH_SIGNAL_BACKDRIVE = 128,
        VEH_SIGNAL_WIPER = 256,
        VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
        VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
        VEH_SIGNAL_EMERGENCY_BLUE = 2048,
        VEH_SIGNAL_EMERGENCY_RED = 4096,
        VEH_SIGNAL_EMERGENCY_YELLOW = 8192
    };

public:
    // every base class needs a virtual destructor
    virtual ~AbstractVehicleMobility() = default;

    // set up the initial position and state of the vehicle (called externaly)
    virtual void preInitialize(std::string external_id, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1) = 0;
    // change position and state of the vehicle (called externaly)
    virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, TraCIScenarioManager::VehicleSignal signals = TraCIScenarioManager::VEH_SIGNAL_UNDEF) = 0;

    // access to vehicle's id within synchronized simulator
    virtual std::string getExternalId() const = 0;
    virtual void setExternalId(std::string external_id) = 0;

    // access to the id of the road the vehicle resides on within synchronized simulator
    virtual std::string getRoadId() const = 0;

    // access to the speed along the road of the vehicle
    virtual double getSpeed() const = 0;

    // access to the indicators/signals of the vehicle
    virtual TraCIScenarioManager::VehicleSignal getSignals() const = 0;

    // access to the vehicle oriantation in the plane (azimuth)
    // returns angle in rads, 0 being east, with -M_PI <= angle < M_PI.
    virtual double getAngleRad() const = 0;
};

} // namespace Veins

#endif /* VEINS_MOBILITY_ABSTRACTVEHICLEMOBILITY_H */
