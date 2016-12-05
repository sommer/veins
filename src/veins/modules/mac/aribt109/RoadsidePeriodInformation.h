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

#ifndef ROADSIDEPERIODINFORMATION_H_
#define ROADSIDEPERIODINFORMATION_H_

/**
 * Is used to determine a limit for transmitting the RVC period information
 */
enum TransmissionCount {
    NO_TRANSFER = 0, TRANSFER_ONCE = 1, TRANSFER_TWICE = 2, TRANSFER_THRICE = 3
};

inline TransmissionCount decrementTransmissionCount(TransmissionCount tc) {
    switch (tc) {
    case TRANSFER_ONCE:
        return NO_TRANSFER;
    case TRANSFER_TWICE:
        return TRANSFER_ONCE;
    case TRANSFER_THRICE:
        return TRANSFER_TWICE;
    default:
        return NO_TRANSFER;
    }
}

inline std::ostream& operator<<(std::ostream &out, TransmissionCount tc) {
    switch (tc) {
    case 1:
        return (out << "ONCE");
    case 2:
        return (out << "TWICE");
    case 3:
        return (out << "THRICE");
    default:
        return (out << "NO");
    }
}

/**
 * Represents the information of a Roadside-to-Veicle Period.
 *
 * Is used by @see BaseStationAribT109MacLayer to store the roadside period information in roadsidePeriodInformationArray. (RRC)
 *
 * Is used by @see MobileStationAribT109MacLayer to store the received roadside period information. (ORT.ENT.TRC & ORT.ENT.RCP)
 *
 * @author Julian Heinovski
 */
struct RoadsidePeriodInformation {

    /**
     * Determines how often this message has to be transferred.
     *
     * Assumption:
     * Base stations estimate the channel and then determine how often a mobile station in the system has to send packets in a specific period such that the data is not compromised.
     * Different base stations can determine different transmission counts. Hence, a mobile station has to decide which received transmission count for each period is used.
     *
     * RRC.TRC
     */
    TransmissionCount transmissionCount;

    /**
     * Represents the length of the period in us or 0 if there is no period.
     * RRC.RCP
     *
     * Every 6240us a new rvc period has to start
     * so there is no definition for the start times.
     * Instead, there is a definition for the duration
     * which is between 0 and 3024us (incl.)
     *
     * NOTE: all base stations have to have the same periods
     * determines the length of each period in us
     * example: 1000,3000,580,1456,792,1324,887,1542,0,120,3012,545,325,2565,10,0
     */
    int periodDuration;
};

#endif /* ROADSIDEPERIODINFORMATION_H_ */
