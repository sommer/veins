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

#define WANT_WINSOCK2
#include <platdep/sockets.h>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <algorithm>
#include <functional>

#include "veins_libsumo/LibsumoConnection.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"

using namespace veins::TraCIConstants;

namespace veins {

struct traci2omnet_functor : public std::unary_function<TraCICoord, Coord> {
    traci2omnet_functor(const LibsumoConnection& owner)
        : owner(owner)
    {
    }

    Coord operator()(const TraCICoord& coord) const
    {
        return owner.traci2omnet(coord);
    }

    const LibsumoConnection& owner;
};

LibsumoConnection::LibsumoConnection(cComponent* owner)
    : HasLogProxy(owner)
{
}

LibsumoConnection::~LibsumoConnection()
{
}

LibsumoConnection* LibsumoConnection::connect(cComponent* owner)
{
    EV_STATICCONTEXT;
    EV_INFO << "TraCIScenarioManager connecting to TraCI server" << endl;

    return new LibsumoConnection(owner);
}

std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf)
{
    if (sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length() > 0xFF) {
        uint32_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + buf.str().length();
        return (TraCIBuffer() << static_cast<uint8_t>(0) << len << commandId).str() + buf.str();
    }
    uint8_t len = sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length();
    return (TraCIBuffer() << len << commandId).str() + buf.str();
}

void LibsumoConnection::setNetbounds(TraCICoord netbounds1, TraCICoord netbounds2, int margin)
{
    coordinateTransformation.reset(new TraCICoordinateTransformation(netbounds1, netbounds2, margin));
}

Coord LibsumoConnection::traci2omnet(TraCICoord coord) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->traci2omnet(coord);
}

std::list<Coord> LibsumoConnection::traci2omnet(const std::list<TraCICoord>& list) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->traci2omnet(list);
}

TraCICoord LibsumoConnection::omnet2traci(Coord coord) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->omnet2traci(coord);
}

std::list<TraCICoord> LibsumoConnection::omnet2traci(const std::list<Coord>& list) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->omnet2traci(list);
}

Heading LibsumoConnection::traci2omnetHeading(double heading) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->traci2omnetHeading(heading);
}

double LibsumoConnection::omnet2traciHeading(Heading heading) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->omnet2traciHeading(heading);
}

} // namespace veins
