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

cplusplus {{
#include "veins/modules/messages/BasicSafetyMessage_m.h"
}}
class BasicSafetyMessage;
enum TrafficLightAtrributeType {
    NONE = 0;
    LOGICID = 1;
    PHASEID = 2;
    SWITCHTIME = 3;
    STATE = 4;
};

enum TrafficLightChangeSource {
    UNKNOWN = 0;
    SUMO = 1;
    LOGIC = 2;
    RSU = 3;//If an RSU tries to change the values
};
enum BsmTrafficLight {
    TFA = 0; //Announcement
};

// NOTE: Currently only supports changes of the IDs (due to variation in field types)
message TraCITrafficLightMessage extends BasicSafetyMessage {
    // traffic light id
    string tlId;
    // what field/attrbute of the traffic light changed?
    int changedAttribute @enum(TrafficLightAtrributeType);
    // value before the change
    string oldValue;
    // value that is to be set / was newly set
    string newValue;
    // where did the change originate
    int changeSource @enum(TrafficLightChangeSource);
};
