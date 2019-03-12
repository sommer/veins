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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACIVEHICLE_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACIVEHICLE_H_

#include "veins/modules/mobility/traci/subscriptionManagement/TrafficParticipant.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class TraCIVehicle
 *
 * Extends the TrafficParticipant definition by a couple of properties
 * of the vehicle like length, height and width.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class TraCIVehicle: public TrafficParticipant {

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
     * @param sginals to initialize this vehicle with.
     * @param length to initialize this vehicle with.
     * @param height to initialize this vehicle with.
     * @param width to initialize this vehicle with.
     */
    TraCIVehicle(double x, double y, std::string edgeID, double speed,
            double angle, std::string id, std::string typeID, int signals, double length,
            double height, double width);

    /**
     * Default destructor.
     */
    virtual ~TraCIVehicle() = default;

    /**
     * Get the signals of this vehicle.
     */
    int getSignals();

    /**
     * Get the length of this vehicle.
     */
    double getLength();

    /**
     * Get the height of this vehicle.
     */
    double getHeight();

    /**
     * Get the width of this vehicle.
     */
    double getWidth();

private:

    /**
     * Stores the signals of this vehicle.
     */
    int mSignals;

    /**
     * Stores the length of this vehicle.
     */
    double mLength;

    /**
     * Stores the height of this vehicle.
     */
    double mHeight;

    /**
     * Stores the width of this vehicle.
     */
    double mWidth;
};

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACIVEHICLE_H_ */

