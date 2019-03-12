//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRAFFICPARTICIPANT_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRAFFICPARTICIPANT_H_

#include <string>

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class TrafficParticipant
 *
 * Provides a basic interface to variables that each traffic
 * participant shares. Extend this to add variables
 * for specific participants like cars, bikes, pedestrians etc.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class TrafficParticipant {

public:

    /**
     * Constructor.
     *
     * @param x coordinate to initialize this participant with.
     * @param y coordinate to initialize this participant with.
     * @param edgeID to initialize this participant with.
     * @param speed to initialize this participant with.
     * @param angle to initialize this participant with.
     * @param id to initialize this participant with.
     * @param typeID to initialize this participant with.
     */
    TrafficParticipant(double x, double y, std::string edgeID, double speed, double angle, std::string id, std::string typeID);

    /**
     * Default destructor.
     */
    virtual ~TrafficParticipant() = default;

    /**
     * Get the TraCI position of this participant.
     *
     * @param x [out] this will contain the x coordinate.
     * @param y [out] this will contain the y coordinate.
     */
    void getPosition(double& x, double& y);

    /**
     * Get the edge id that this participant is travelling on.
     *
     * @return std::string: edge id.
     */
    std::string getEdgeID();

    /**
     * Get speed value of this participant.
     *
     * @return double: speed.
     */
    double getSpeed();

    /**
     * Get the traci angle of this participant.
     *
     * @return double: angle.
     */
    double getAngle();

    /**
     * Get the traci id of this traffic participant.
     *
     * @return std::string: the id.
     */
    std::string getID();

    /**
     * Get the traci type id of this traffic participant.
     *
     * @return std::string: the type id.
     */
    std::string getTypeID();

private:

    /**
     * Stores the x coordinate of this participant.
     */
    double mX;

    /**
     * Stores the y coordinate of this participant.
     */
    double mY;

    /**
     * Stores the edge id of this participant.
     */
    std::string mEdgeID;

    /**
     * Stores the speed of this participant.
     */
    double mSpeed;

    /**
     * Stores the angle of this participant.
     */
    double mAngle;

    /**
     * Stores the id of this person.
     */
    std::string mID;

    /**
     * Stores the type id of this person.
     */
    std::string mTypeID;

};

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRAFFICPARTICIPANT_H_ */
