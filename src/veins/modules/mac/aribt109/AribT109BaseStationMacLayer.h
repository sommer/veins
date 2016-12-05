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

#ifndef BASESTATIONARIBT109MAC_H_
#define BASESTATIONARIBT109MAC_H_

#include "veins/modules/mac/aribt109/AribT109AbstractMacLayer.h"

class AribT109BaseStationMacLayer: public AribT109AbstractMacLayer {
public:
    AribT109BaseStationMacLayer() {
    }
    ;

    // OMNeT functions

    /** @brief Initialize the module and variables.*/
    void initialize(int stage);
    /** @brief Delete all dynamically allocated objects of the module.*/
    void finish();

    /** @brief Handle messages from lower layer. Is called from handleMessage of super class. */
    void handleLowerMsg(cMessage* msg);
    /** @brief Handle messages from upper layer. Is called from handleMessage of super class.*/
    void handleUpperMsg(cMessage* msg);
    /** @brief Handle self messages such as timers. Is called from handleMessage of super class.*/
    void handleSelfMsg(cMessage* msg);

protected:
    void channelIdle();
    void channelBusy();
    void channelBusySelf();
    void channelIdleSelf();

private:

};

#endif /* BASESTATIONARIBT109MAC_H_ */
