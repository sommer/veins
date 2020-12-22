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

#include "veins/visualizer/roads/RoadsOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LineWidth>
#include <osg/Material>
#endif // ifdef WITH_OSG

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using veins::RoadsOsgVisualizer;

Define_Module(veins::RoadsOsgVisualizer);

#ifndef WITH_OSG
void RoadsOsgVisualizer::initialize(int stage)
{
}
#else
void RoadsOsgVisualizer::initialize(int stage)
{
    bool enabled = par("enabled");
    if (!enabled) return;

    if (!hasGUI()) return;

    if (stage == 0) {

        TraCIScenarioManager* manager = TraCIScenarioManagerAccess().get();
        ASSERT(manager);

        figures = new osg::Group();

        cOsgCanvas* canvas = getParentModule()->getOsgCanvas();
        ASSERT(canvas);
        osg::Group* scene = dynamic_cast<osg::Group*>(canvas->getScene());
        if (!scene) {
            scene = new osg::Group();
            canvas->setScene(scene);
        }
        ASSERT(scene);
        scene->addChild(figures);

        auto onTraciInitialized = [this, manager](veins::SignalPayload<bool> payload) {
            TraCICommandInterface* traci = manager->getCommandInterface();
            ASSERT(traci);

            std::string colorStr = par("lineColor");
            auto color = cFigure::Color(colorStr.c_str());
            double width = par("lineWidth");

            auto laneIds = traci->getLaneIds();
            for (auto laneId : laneIds) {
                auto coords = traci->lane(laneId).getShape();

                auto line = createLine(coords, color, width);
                figures->addChild(line);
            }
        };
        signalManager.subscribeCallback(manager, TraCIScenarioManager::traciInitializedSignal, onTraciInitialized);
    }
}

void RoadsOsgVisualizer::handleMessage(cMessage* msg)
{
    throw cRuntimeError("RoadsOsgVisualizer does not handle any events");
}

void RoadsOsgVisualizer::finish()
{
}

osg::Geode* RoadsOsgVisualizer::createLine(const std::list<veins::Coord>& coords, cFigure::Color color, double width)
{
    auto verts = new osg::Vec3Array();
    for (auto coord : coords) {
        verts->push_back(osg::Vec3(coord.x, coord.y, coord.z));
    }

    auto primitiveSet = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP);
    primitiveSet->setFirst(0);
    primitiveSet->setCount(verts->size());

    auto geometry = new osg::Geometry();
    geometry->setVertexArray(verts);
    geometry->addPrimitiveSet(primitiveSet);

    osg::Vec4 colorVec(color.red / 255.0, color.green / 255.0, color.blue / 255.0, 1.0);

    auto material = new osg::Material();
    material->setAmbient(osg::Material::FRONT_AND_BACK, colorVec);
    material->setDiffuse(osg::Material::FRONT_AND_BACK, colorVec);

    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(width);

    auto stateSet = new osg::StateSet();
    stateSet->setAttribute(material);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setAttribute(lineWidth);

    auto geode = new osg::Geode();
    geode->addDrawable(geometry);
    geode->setStateSet(stateSet);

    return geode;
}

#endif // ifdef WITH_OSG
