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

/**
 * @brief Extends PhyLayer by adding power consumption for tx, rx and idle.
 *
 * Does two things, first before sending messages to the channel or
 * receiving messages from the channel it checks the hosts current state.
 * If the host is not able to send or the receive (e.g. no power) the
 * messages are dropped.
 * Draws tx current from battery during transmissions (starting on
 * reception of message from upper layer until reception of TX_OVER
 * message). And idle current any time else.
 * Does also provide battery access to the Decider by implementing
 * DeciderToPhyInterfaces "drawCurrent()"-method.
 *
 * Defines initialization for "Decider80211Battery".
 *
 * @ingroup power
 * @ingroup decider
 */
class PhyLayerBattery : public PhyLayer{
protected:
	/** @brief Number of power consuming activities (accounts).*/
	int numActivities;

	/** @brief Hosts current state.*/
	int hostState;

	/** @brief Blackboard category for HostStates.*/
	int hostStateCat;

	/** @brief The different currents in mA.*/
	double sleepCurrent, idleCurrent, rxCurrent, txCurrent;

	/**
	 * @brief Defines the power consuming activities (accounts) of
	 * the NIC. Should be the same as defined in the decider.
	 */
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

	/**
	 * @brief Checks the hosts state and draws txCurrent from
	 * battery.
	 *
	 * Prevents sending of AirFrames if the hosts state is not on.
	 * Sets current power consumption for TX mode.
	 * Calls the base classes overriden method for normal handling.
	 */
	virtual void handleUpperMessage(cMessage* msg);

	/**
	 * @brief Checks if the the host state is on and prevents
	 * reception of AirFrames if not.
	 *
	 * Calls the base classes overriden method for normal handling.
	 */
	virtual void handleAirFrame(cMessage* msg);

	/**
	 * @brief Catches TX over messages to set current power consumption
	 * back for idle mode.
	 *
	 * Calls the base classes overriden method for normal handling.
	 */
	virtual void handleSelfMessage(cMessage* msg);

	/**
	 * @brief Provides ability to draw power for the Decider.
	 *
	 * Method is defined in "DeciderToPhyInterface".
	 */
	virtual void drawCurrent(double amount, int activity);

	/**
	 * @brief Captures changes in host state.
	 *
	 * Note: Does not yet cancel any ongoing transmissions if the
	 * state changes to off.
	 */
	void receiveBBItem(int category, const BBItem *details,
						int scopeModuleId);
};

#endif /* PHYLAYERBATTERY_H_ */
