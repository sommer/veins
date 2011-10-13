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

#include "Mac80211MultiChannel.h"
#include "BasePhyLayer.h"

Define_Module(Mac80211MultiChannel);



void Mac80211MultiChannel::initialize(int stage)
{
    Mac80211::initialize(stage);

    if(stage == 0) {
    }
    else if(stage == 1) {
    }
}

void Mac80211MultiChannel::switchChannel(int channel) {
	if(!(1 <= channel && channel <= 14)) {
		opp_error("Invalid channel %d. Mac tried to switch to a channel not"
				  " supported by this protocoll.", channel);
	}
	phy->setCurrentRadioChannel(channel);

	centerFreq = CENTER_FREQUENCIES[channel];
}

int Mac80211MultiChannel::getChannel() {
	return phy->getCurrentRadioChannel();
}
