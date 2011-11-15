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

#ifndef __CSMA802154_H__
#define __CSMA802154_H__

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "csma.h"

/**
 * @brief Extends "csma" module by some statistics send
 * up to the upper layer.
 *
 * @ingroup macLayer
 * @ingroup csma
 * @ingroup ieee802154
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 */
class MIXIM_API CSMA802154 : public csma
{
protected:
    virtual void initialize(int stage);

    virtual cPacket *decapsMsg(MacPkt * macPkt);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);
};

#endif
