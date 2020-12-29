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

#include "veins/visualizer/roads/RoadsCanvasVisualizer.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using veins::RoadsCanvasVisualizer;

Define_Module(veins::RoadsCanvasVisualizer);

void RoadsCanvasVisualizer::initialize(int stage)
{
    bool enabled = par("enabled");
    if (!enabled) return;

    if (!hasGUI()) return;

    if (stage == 0) {

        TraCIScenarioManager* manager = TraCIScenarioManagerAccess().get();
        ASSERT(manager);

        figures = new cGroupFigure("roads");
        double zindex = par("lineZIndex");
#if OMNETPP_VERSION >= 0x501
        figures->setZIndex(zindex);
#endif
        cCanvas* canvas = getParentModule()->getCanvas();
        canvas->addFigure(figures);

        auto onTraciInitialized = [this, manager](veins::SignalPayload<bool> payload) {
            TraCICommandInterface* traci = manager->getCommandInterface();
            ASSERT(traci);

            std::string colorStr = par("lineColor");
            auto color = cFigure::Color(colorStr.c_str());
            double width = par("lineWidth");
            bool zoom = par("lineWidthZoom");

            auto laneIds = traci->getLaneIds();
            for (auto laneId : laneIds) {
                auto coords = traci->lane(laneId).getShape();

                auto line = createLine(coords, color, width, zoom);
                figures->addFigure(line);
            }
        };
        signalManager.subscribeCallback(manager, TraCIScenarioManager::traciInitializedSignal, onTraciInitialized);
    }
}

void RoadsCanvasVisualizer::handleMessage(cMessage* msg)
{
    throw cRuntimeError("RoadsCanvasVisualizer does not handle any events");
}

void RoadsCanvasVisualizer::finish()
{
}

cPolylineFigure* RoadsCanvasVisualizer::createLine(const std::list<veins::Coord>& coords, cFigure::Color color, double width, bool zoom)
{
    std::vector<cFigure::Point> points;
    for (auto coord : coords) {
        points.push_back(cFigure::Point(coord.x, coord.y));
    }

    auto* line = new cPolylineFigure();
    line->setPoints(points);
    line->setLineColor(color);
    line->setLineWidth(width);
    line->setZoomLineWidth(zoom);

    return line;
}
