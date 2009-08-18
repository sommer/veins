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

#ifndef PHYLAYERBATTERY_H_
#define PHYLAYERBATTERY_H_

#include "PhyLayer.h"
#include "Decider80211Battery.h"
#include "HostState.h"

class PhyLayerBattery : public PhyLayer{
protected:
	int numActivities;
	int hostState;
	int hostStateCat;
	double sleepCurrent, idleCurrent, rxCurrent, txCurrent;

	enum Activities {
		SLEEP_ACCT=0,
		IDLE_ACCT=1,
		RX_ACCT=2,
		TX_ACCT=3
	};

protected:
	/**
	 * @brief Creates and returns an instance of the Decider with the specified
	 * name.
	 *
	 * Is able to initialize the following Deciders:
	 *
	 * - Decider80211
	 * - Decider80211Battery
	 * - SNRThresholdDecider
	 */
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	/**
	 * @brief Initializes a new Decider80211Battery from the passed parameter map.
	 */
	virtual Decider* initializeDecider80211Battery(ParameterMap& params);

public:
	virtual void initialize(int stage);

	virtual void handleUpperMessage(cMessage* msg);

	virtual void handleAirFrame(cMessage* msg);

	virtual void handleSelfMessage(cMessage* msg);

	virtual void drawCurrent(double amount, int activity);

	void receiveBBItem(int category, const BBItem *details,
						int scopeModuleId);
};

#endif /* PHYLAYERBATTERY_H_ */
