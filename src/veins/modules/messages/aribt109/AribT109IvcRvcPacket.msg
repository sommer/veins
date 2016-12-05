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
#include <stdlib.h>    
#include <vector>
#include "veins/modules/mac/aribt109/SynchronizationStatus.h"
#include "veins/modules/mac/aribt109/RoadsidePeriodInformation.h"

typedef std::vector<RoadsidePeriodInformation> RoadsidePeriodInformationVector;
}}

class SynchronizationStatus extends void;
class RoadsidePeriodInformationVector extends void;

packet AribT109IvcRvcPacket {
	// protocol version field
    // Not needed because not really used.
    // Just bit 0 and bit 1 - 15 is reserved for future use.

    // type field
    // Determines the type of source.
    // Just uses bit 3, 0 - 2 not used.
    bool isFromBaseStation;

    // synchronization information field
    // Determines the synchronization status of the sender.
    bool synchronized;

    // Determines the kind of synchronization. Only valid if the sender was synchronized.
    SynchronizationStatus mobileStationSynchronizationStatus;

    // reserved field not needed

    // timestamp field
    // Indicated the transmission time which is the value of the one second cycle timer in microseconds
    // when the beginning of the PLCP preamble is transmitted from the antenna.
    // Range is from 0 to 999999.
    // Accuracy shall be not worse than +- 2us.
    simtime_t aribTimeStamp;

    // rvc period information field
    // Array shall hold exactly 16 entries.
    RoadsidePeriodInformationVector roadsidePeriodInformationArray;

    // enhanced field not needed
    
    // for evaluation purpose
    simtime_t ivcLayerArivalTimeStamp;
    
    // for evaluation purpose
    simtime_t csmaStartTimeStamp;
}
