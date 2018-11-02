//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef MAC80211PTOPHY11PINTERFACE_H_
#define MAC80211PTOPHY11PINTERFACE_H_

#include "veins/base/phyLayer/MacToPhyInterface.h"

namespace Veins {

/**
 * @brief
 * Interface of PhyLayer80211p exposed to Mac1609_4.
 *
 * @author Christopher Saloman
 * @author David Eckhoff
 *
 * @ingroup phyLayer
 */
class Mac80211pToPhy11pInterface {
public:
    enum BasePhyMessageKinds {
        CHANNEL_IDLE,
        CHANNEL_BUSY,
    };

public:
    virtual void changeListeningFrequency(double freq) = 0;
    virtual void setCCAThreshold(double ccaThreshold_dBm) = 0;
    virtual void notifyMacAboutRxStart(bool enable) = 0;
    virtual void requestChannelStatusIfIdle() = 0;
    virtual ~Mac80211pToPhy11pInterface(){};
};

} // namespace Veins

#endif /* MAC80211PTOPHY11PINTERFACE_H_ */
