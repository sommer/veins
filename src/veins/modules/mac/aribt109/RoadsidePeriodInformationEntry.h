//
// Copyright (C) 2016 Julian Heinovski <julian.heinovski@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
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
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef ROADSIDEPERIODINFORMATIONENTRY_H_
#define ROADSIDEPERIODINFORMATIONENTRY_H_

#include "veins/modules/mac/aribt109/RoadsidePeriodInformation.h"

/**
 * Represents a Roadside-to-vehicle period information entry. (ORT.ENT)
 *
 * Is used by @see MobileStationAribT109MacLayer to store all received roadside period information.
 *
 * @author Julian Heinovski
 */
struct RoadsidePeriodInformationEntry {
    /**
     * RVC period numbers
     *
     * Is used to distinguish between the 16 different periods. This value is necessary because we are allowed store multiple information entries for the same period.
     *
     * ORT.ENT.RCN
     * 1..16
     */
    short periodNumber;

    /**
     * RVC period information
     *
     * Is used to store the actual period's information.
     *
     * ORT.ENT. TRC & RCP
     */
    RoadsidePeriodInformation periodInformation;

    /**
     * New feature by Julian Heinovski
     *
     * The sender of the information entry is stored to distinguish between information received from different senders.
     * In case of receiving new information, only the entry from the same sender is updated.
     */
    LAddress::L2Type senderMacAddress;

    /**
     * Elapsed time
     *
     * Is used to determine how much time has passed since a particular entry has been stored.
     *
     * ORT.ENT.ELT
     */
    simtime_t storingTimeStamp;
};

static inline bool isNoTransfer(const RoadsidePeriodInformationEntry& entry) {
    return (entry.periodInformation.transmissionCount == NO_TRANSFER);
}

#endif /* ROADSIDEPERIODINFORMATIONENTRY_H_ */
