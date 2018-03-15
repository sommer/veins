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

#include <veins/modules/world/traci/trafficLight/TraCITrafficLightProgram.h>

using Veins::TraCITrafficLightProgram;

bool TraCITrafficLightProgram::Phase::isGreenPhase() const
{
	// implementation taken from SUMO MSPhaseDefinition.cc
	if (state.find_first_of("gG") == std::string::npos) {
		return false;
	}
	if (state.find_first_of("yY") != std::string::npos) {
		return false;
	}
	return true;
}


TraCITrafficLightProgram::TraCITrafficLightProgram(std::string id):
	id(id),
	logics()
{
}

void TraCITrafficLightProgram::addLogic(const Logic& logic)
{
	logics[logic.id] = logic;
}

TraCITrafficLightProgram::Logic TraCITrafficLightProgram::getLogic(const std::string& lid) const
{
	return logics.at(lid);
}

bool TraCITrafficLightProgram::hasLogic(const std::string& lid) const
{
	return logics.find(lid) != logics.end();
}
