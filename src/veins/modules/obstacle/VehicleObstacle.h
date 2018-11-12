//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#ifndef OBSTACLE_VEHICLEOBSTACLE_H
#define OBSTACLE_VEHICLEOBSTACLE_H

#include <vector>

#include "veins/base/utils/Coord.h"

/**
 * stores information about an Obstacle for ObstacleControl
 */
namespace Veins {

class ChannelAccess;

class TraCIMobility;

class VehicleObstacle {
public:
    typedef std::vector<Coord> Coords;

    VehicleObstacle(ChannelAccess* channelAccess, TraCIMobility* traciMobility, double length, double hostPositionOffset, double width, double height)
        : channelAccess(channelAccess)
        , traciMobility(traciMobility)
        , length(length)
        , hostPositionOffset(hostPositionOffset)
        , width(width)
        , height(height)
    {
    }

    void setChannelAccess(ChannelAccess* channelAccess)
    {
        this->channelAccess = channelAccess;
    }
    void setTraCIMobility(TraCIMobility* traciMobility)
    {
        this->traciMobility = traciMobility;
    }
    void setLength(double d)
    {
        this->length = d;
    }
    void setHostPositionOffset(double d)
    {
        this->hostPositionOffset = d;
    }
    void setWidth(double d)
    {
        this->width = d;
    }
    void setHeight(double d)
    {
        this->height = d;
    }

    const ChannelAccess* getChannelAccess() const
    {
        return channelAccess;
    }
    const TraCIMobility* getTraCIMobility() const
    {
        return traciMobility;
    }
    double getLength() const
    {
        return length;
    }
    double getHostPositionOffset() const
    {
        return hostPositionOffset;
    }
    double getWidth() const
    {
        return width;
    }
    double getHeight() const
    {
        return height;
    }

    Coords getShape() const;

    /**
     * return closest point (in meters) along (senderPos--receiverPos) where this obstacle overlaps, or NAN if it doesn't
     */
    double getIntersectionPoint(const Coord& senderPos, const Coord& receiverPos) const;

protected:
    ChannelAccess* channelAccess;
    TraCIMobility* traciMobility;
    double length;
    double hostPositionOffset;
    double width;
    double height;
};
} // namespace Veins

#endif
