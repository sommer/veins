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

#ifndef TRANSMISSIONCONTROL_H_
#define TRANSMISSIONCONTROL_H_

#include "veins/modules/utility/ConstsAribT109.h"

/**
 * Represents Roadside-to-Vehicle transmission control information. (RTC)
 * Used for transmission control to determine at which times a particular base station actually transmits.
 *
 * EDIT: It is also used in mobile stations to determine the actual times the vehicle is allowed to use the channel (@see MobileStationAribT109MacLayer::initialize(int) for furhter information)
 *
 * @author Julian Heinovski
 */
struct TransmissionControl {
    /**
     * aka transmissionTiming
     * Determines the time when the transmission begins.
     * In us.
     * TST
     */
    int transmissionPeriodBegin;

    /**
     * Determines the duration of the transmission period.
     * TRP
     * In us.
     *
     * Note that the value should be within the range of the @see RvcPeriodInformation.periodDuration.
     */
    int transmissionPeriodDuration;

    /**
     * not in standard
     * In us.
     */
//        int transmissionPeriodEnd = transmissionPeriodBegin + transmissionPeriodDuration;
};

#endif /* TRANSMISSIONCONTROL_H_ */
