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

#ifndef __MIXIM_MULTICHANNELMAC80211_H_
#define __MIXIM_MULTICHANNELMAC80211_H_

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "Mac80211.h"

/**
 * @brief Adds multi channel support to Mac80211.
 *
 * Multi channel support enables the MAC layer to change the channel for
 * transmission and reception during simulation.
 *
 * @author Karl Wessel
 */
class MIXIM_API Mac80211MultiChannel : public Mac80211
{
protected:
    virtual void initialize(int stage);
public:
    /**
     * @brief Tells the MAC layer to switch to the passed channel.
     *
     * This method can be used by upper layers to change the channel.
     * @param channel The channel to switch to, must be 1<=channel<=14.
     */
    void switchChannel(int channel);

    /**
     * @brief Returns the currently used channel.
     * @return The currently used channel.
     */
    int getChannel();
};

#endif
