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
#include "veins/base/utils/Coord.h"
}}
class BasicSafetyMessage;
class noncobject Coord;

packet CooperativeAwarenessMessage extends BasicSafetyMessage {
    string vehicleId;
    // contents for simulation
    string currentEdge;
    int currentLandNr;
    double currentLanePosition;

    // contents for real-world data
    // senderPosition already included in BasicSafetyMessage
    Coord senderLastPos;
    // this is the absolute speed, the speed vector is available in the BasicSafetyMessage
    double speed;
    double angle;
}
