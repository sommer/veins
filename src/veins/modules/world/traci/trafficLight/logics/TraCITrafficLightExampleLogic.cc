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

#include "TraCITrafficLightExampleLogic.h"

using Veins::TraCITrafficLightExampleLogic;

Define_Module(Veins::TraCITrafficLightExampleLogic);

TraCITrafficLightExampleLogic::TraCITrafficLightExampleLogic() {

}

TraCITrafficLightExampleLogic::~TraCITrafficLightExampleLogic() {

}

void TraCITrafficLightExampleLogic::initialize()
{

}


void TraCITrafficLightExampleLogic::finish()
{/*
    if(customLog.is_open()) {
        customLog.close();
    }*/
}

void TraCITrafficLightExampleLogic::handleApplMsg(cMessage *msg)
{

}
void TraCITrafficLightExampleLogic::handleTlIfMsg(TraCITrafficLightMessage *tlMsg)
{

}

void TraCITrafficLightExampleLogic::handlePossibleSwitch()
{

}

void TraCITrafficLightExampleLogic::setNewState(/*const signalScheme& signal, const phasetime_t& greenTime*/)
{

}
bool TraCITrafficLightExampleLogic::isSuperposition(/*const signalScheme& major, const signalScheme& minor*/) const
{

}
void TraCITrafficLightExampleLogic::highlightScheduledConfig() {

}
