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

#ifndef PHYLAYER80211P_H_
#define PHYLAYER80211P_H_

#include "veins/base/phyLayer/BasePhyLayer.h"
#include "veins/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h"
#include "veins/modules/phy/Decider80211p.h"
#include "veins/modules/analogueModel/SimplePathlossModel.h"
#include "veins/modules/analogueModel/LogNormalShadowing.h"
#include "veins/modules/phy/SNRThresholdDecider.h"
#include "veins/modules/analogueModel/JakesFading.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/modules/phy/Decider80211pToPhy80211pInterface.h"
#include "veins/base/utils/Move.h"

using Veins::AirFrame;

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath() << " "

/**
 * @brief
 * Adaptation of the PhyLayer class for 802.11p.
 *
 * @ingroup phyLayer
 *
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class PhyLayer80211p	: 	public BasePhyLayer,
	public Mac80211pToPhy11pInterface,
	public Decider80211pToPhy80211pInterface

{
	public:
		void initialize(int stage);
		/**
		 * @brief Set the carrier sense threshold
		 * @param ccaThreshold_dBm the cca threshold in dBm
		 */
		void setCCAThreshold(double ccaThreshold_dBm);
		/**
		 * @brief Return the cca threshold in dBm
		 */
		double getCCAThreshold();
		/**
		 * @brief Enable notifications about PHY-RXSTART.indication in MAC
		 * @param val true if Mac needs to be notified about it
		 */
		void notifyMacAboutRxStart(bool enable);
		/**
		 * @brief Explicit request to PHY for the channel status
		 */
		void requestChannelStatusIfIdle();
	protected:

		/** @brief CCA threshold. See Decider80211p for details */
		double ccaThreshold;

		/** @brief enable/disable detection of packet collisions */
		bool collectCollisionStatistics;

		/** @brief allows/disallows interruption of current reception for txing
		 *
		 * See detailed description in Decider80211p
		 */
		bool allowTxDuringRx;

		enum ProtocolIds {
			IEEE_80211 = 12123
		};
		/**
		 * @brief Creates and returns an instance of the AnalogueModel with the
		 * specified name.
		 *
		 * Is able to initialize the following AnalogueModels:
		 */
		virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

		/**
		 * @brief Creates and initializes a SimplePathlossModel with the
		 * passed parameter values.
		 */
		AnalogueModel* initializeSimplePathlossModel(ParameterMap& params);

		/**
		 * @brief Creates and initializes an AntennaModel with the
		 * passed parameter values.
		 */
		AnalogueModel* initializeAntennaModel(ParameterMap& params);

		/**
		 * @brief Creates and initializes a LogNormalShadowing with the
		 * passed parameter values.
		 */
		AnalogueModel* initializeLogNormalShadowing(ParameterMap& params);

		/**
		 * @brief Creates and initializes a JakesFading with the
		 * passed parameter values.
		 */
		AnalogueModel* initializeJakesFading(ParameterMap& params);

		/**
		 * @brief Creates and initializes a BreakpointPathlossModel with the
		 * passed parameter values.
		 */
		virtual AnalogueModel* initializeBreakpointPathlossModel(ParameterMap& params);

		/**
		 * @brief Creates and initializes a SimpleObstacleShadowing with the
		 * passed parameter values.
		 */
		AnalogueModel* initializeSimpleObstacleShadowing(ParameterMap& params);

		/**
		 * @brief Creates a simple Packet Error Rate model that attenuates a percentage
		 * of the packets to zero, and does not attenuate the other packets.
		 *
		 */
		virtual AnalogueModel* initializePERModel(ParameterMap& params);

		/**
		 * @brief Creates and initializes a TwoRayInterferenceModel with the
		 * passed parameter values.
		 */
		AnalogueModel* initializeTwoRayInterferenceModel(ParameterMap& params);

        /**
         * @brief Creates and initializes a NakagamiFading with the
         * passed parameter values.
         */
        AnalogueModel* initializeNakagamiFading(ParameterMap& params);

		/**
		 * @brief Creates and returns an instance of the Decider with the specified
		 * name.
		 *
		 * Is able to initialize the following Deciders:
		 *
		 * - Decider80211p
		 * - SNRThresholdDecider
		 */
		virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

		/**
		 * @brief Initializes a new Decider80211 from the passed parameter map.
		 */
		virtual Decider* initializeDecider80211p(ParameterMap& params);

		/**
		 * @brief This function encapsulates messages from the upper layer into an
		 * AirFrame and sets all necessary attributes.
		 */
		virtual AirFrame *encapsMsg(cPacket *msg);

		virtual void changeListeningFrequency(double freq);

		virtual void handleSelfMessage(cMessage* msg);
		virtual int getRadioState();
		virtual simtime_t setRadioState(int rs);
};

#endif /* PHYLAYER80211P_H_ */
