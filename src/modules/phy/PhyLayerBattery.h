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

#include "MiXiMDefs.h"
#include "PhyLayer.h"
#include "HostState.h"

class MacToPhyControlInfo;
class MacPkt;

/**
 * @brief Extends PhyLayer by adding power consumption for tx, rx and idle.
 *
 * Does two things, first before sending messages to the channel or
 * receiving messages from the channel it checks the hosts current state.
 * If the host is not able to send or the receive (e.g. no power) the
 * messages are dropped.
 *
 * Further it draws a current depending on the current radio state.
 * For this it captures every call to "setRadioState()" and changes
 * the current to previously set default values according to the
 * new radios state. There are different currents for TX, RX, SLEEP
 * and SWITCHING state.
 * Its also possible to override "calcTXCurrentForPacket()" to
 * calculate the current used during TX depending on the actual
 * TX power instead of using the default value.
 *
 * Does also provide battery access to the Decider by implementing
 * DeciderToPhyInterfaces "drawCurrent()"-method. Using this method
 * the Decider can define a current delta which is applied to the RX
 * current during this radio state. This is meant to be used if the
 * Decider wants to define different currents for "listening" and
 * "decoding" phases in RX state.
 *
 * Defines initialization for "Decider80211Battery".
 *
 * @ingroup power
 * @ingroup phyLayer
 */
class MIXIM_API PhyLayerBattery : public PhyLayer{
protected:
	/** @brief Number of power consuming activities (accounts).*/
	int numActivities;

	/** @name The different currents in mA.*/
	/*@{*/
	double sleepCurrent, rxCurrent, decodingCurrentDelta, txCurrent;
	/*@}*/

	/** @name The different switching state currents in mA.*/
	/*@{*/
	double setupRxCurrent, setupTxCurrent, rxTxCurrent, txRxCurrent;
	/*@}*/

	/**
	 * @brief Defines the power consuming activities (accounts) of
	 * the NIC. Should be the same as defined in the decider.
	 */
	enum Activities {
		SLEEP_ACCT=0,
		RX_ACCT,
		TX_ACCT,
		SWITCHING_ACCT,
		DECIDER_ACCT,
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
	 * - Decider80211MultiChannel
	 * - SNRThresholdDecider
	 */
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	/**
	 * @brief Initializes a new Decider80211Battery from the passed parameter map.
	 */
	virtual Decider* initializeDecider80211Battery(ParameterMap& params);

	/**
	 * @brief Initializes a new Decider80211MultiChannel from the passed parameter map.
	 */
	virtual Decider* initializeDecider80211MultiChannel(ParameterMap& params);

	/**
	 * @brief Calculates the current needed for the transmission of the
	 * passed MacPkt.
	 *
	 * A return value below or equal zero is ignored and the default txCurrent
	 * is used instead.
	 *
	 * Sub classing modules can override this method if they don't want to use
	 * a default TX-current for every transmission but a current depending on
	 * the actual used TX power for a packet.
	 */
	virtual double calcTXCurrentForPacket(MacPkt* pkt, MacToPhyControlInfo* cInfo)
	{ return -1.0; }

	/** @brief Updates the actual current drawn for the passed state.*/
	virtual void setRadioCurrent(int rs);

	/** @brief Updates the actual current drawn for switching between
	 * the passed states.*/
	virtual void setSwitchingCurrent(int from, int to);

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
	virtual void handleAirFrame(AirFrame* frame);

	/**
	 * @brief Captures changes in host state.
	 *
	 * Note: Does not yet cancel any ongoing transmissions if the
	 * state changes to off.
	 */
	virtual void handleHostState(const HostState& state);

	/**
	 * @brief Captures radio switches to adjust power consumption.
	 */
	virtual void finishRadioSwitching();

public:
	virtual void initialize(int stage);

	/**
	 * @brief Provides ability to draw power for the Decider.
	 *
	 * Method is defined in "DeciderToPhyInterface".
	 *
	 * Note: This method should only be used by the Decider to
	 * draw power. The phy layer itself should call instead its
	 * protected method BatteryAccess::drawCurrent()!
	 */
	virtual void drawCurrent(double amount, int activity);

	/**
	 * @brief Captures radio switches to adjust power consumption.
	 */
	virtual simtime_t setRadioState(int rs);
};

#endif /* PHYLAYERBATTERY_H_ */
