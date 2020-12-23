//
// Copyright (C) 2020 Christoph Sommer <sommer@cms-labs.org>
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

#ifdef WITH_OSG
namespace osg {
class Geode;
class Group;
}; // namespace osg
#endif // WITH_OSG

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/modules/utility/SignalManager.h"

namespace veins {

/**
 * @brief
 * Simple support module to visualize road network as received via TraCI.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer
 *
 * @see TraCIScenarioManager
 *
 */
class VEINS_API RoadsOsgVisualizer : public cSimpleModule {
public:
    void initialize(int stage) override;
#ifdef WITH_OSG
    void handleMessage(cMessage* msg) override;
    void finish() override;

protected:
    veins::SignalManager signalManager;
    osg::Group* figures;

    osg::Geode* createLine(const std::list<veins::Coord>& coords, cFigure::Color color, double width);
#endif // WITH_OSG
};

} // namespace veins
