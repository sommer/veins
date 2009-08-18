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

#ifndef DECIDER80211BATTERY_H_
#define DECIDER80211BATTERY_H_

#include "Decider80211.h"

class Decider80211Battery : public Decider80211 {
protected:
	double rxCurrent;
	double idleCurrent;

	enum Activities {
		SLEEP_ACCT=0,
		IDLE_ACCT=1,
		RX_ACCT=2,
		TX_ACCT=3
	};
public:
	Decider80211Battery(DeciderToPhyInterface* phy,
						double threshold,
						double sensitivity,
						double centerFrequency,
						double rxCurrent,
						double idleCurrent,
						int myIndex = -1,
						bool debug = false):
		Decider80211(phy, sensitivity, myIndex, debug, threshold, centerFrequency),
		rxCurrent(rxCurrent),
		idleCurrent(idleCurrent)
	{}

	/**
	 * @brief Changes the "isIdle"-status to the passed value.
	 *
	 * This method further checks if there are any ChannelSenseRequests
	 * which can be answered because of the idle state changed.
	 */
	virtual void setChannelIdleStatus(bool isIdle);
};

#endif /* DECIDER80211BATTERY_H_ */
