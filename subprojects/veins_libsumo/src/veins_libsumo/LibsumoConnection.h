//
// Copyright (C) 2006 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

#pragma once

#include <stdint.h>
#include <memory>

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Heading.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace veins {

class VEINS_API LibsumoConnection : public HasLogProxy {
public:
    static LibsumoConnection* connect(cComponent* owner);
    void setNetbounds(TraCICoord netbounds1, TraCICoord netbounds2, int margin);
    ~LibsumoConnection();

    /**
     * convert TraCI heading to OMNeT++ heading (in rad)
     */
    Heading traci2omnetHeading(double heading) const;

    /**
     * convert OMNeT++ heading (in rad) to TraCI heading
     */
    double omnet2traciHeading(Heading heading) const;

    /**
     * convert TraCI coordinates to OMNeT++ coordinates
     */
    Coord traci2omnet(TraCICoord coord) const;
    std::list<Coord> traci2omnet(const std::list<TraCICoord>&) const;

    /**
     * convert OMNeT++ coordinates to TraCI coordinates
     */
    TraCICoord omnet2traci(Coord coord) const;
    std::list<TraCICoord> omnet2traci(const std::list<Coord>&) const;

private:
    LibsumoConnection(cComponent* owner);

    std::unique_ptr<TraCICoordinateTransformation> coordinateTransformation;
};

/**
 * returns byte-buffer containing a TraCI command with optional parameters
 */
std::string makeLibsumoCommand(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

} // namespace veins
