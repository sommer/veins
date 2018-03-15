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

#ifndef SRC_VEINS_MODULES_WORLD_TRACI_TRAFFICLIGHT_TRACITRAFFICLIGHTPROGRAM_H_
#define SRC_VEINS_MODULES_WORLD_TRACI_TRAFFICLIGHT_TRACITRAFFICLIGHTPROGRAM_H_

#include <string>
#include <vector>
#include <map>
namespace Veins {
class TraCITrafficLightProgram {
public:
	struct Phase {
		int32_t duration;
		int32_t minDuration;
		int32_t maxDuration;
		std::string state;

		bool isGreenPhase() const;
	};
	struct Logic {
		std::string id;
		int32_t currentPhase;
		std::vector<Phase> phases;
		int32_t type;       // currently unused, just 0
		int32_t parameter;  // currently unused, just 0
	};

	TraCITrafficLightProgram(std::string id = "");

	void addLogic(const Logic& logic);
	TraCITrafficLightProgram::Logic getLogic(const std::string& lid) const;
	bool hasLogic(const std::string& lid) const;
   /* TraCITrafficLightProgram();
	virtual ~TraCITrafficLightProgram();*/
private:
	std::string id;
	std::map<std::string, TraCITrafficLightProgram::Logic> logics;

};


struct TraCITrafficLightLink {
	std::string incoming;
	std::string outgoing;
	std::string internal;
};

}
#endif /* SRC_VEINS_MODULES_WORLD_TRACI_TRAFFICLIGHT_TRACITRAFFICLIGHTPROGRAM_H_ */
