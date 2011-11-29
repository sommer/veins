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

/**
 * @brief 802.11p Mac-Layer.
 *
 * @class Mac80211p
 * @ingroup Mac80211p
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 * @author Christopher Saloman (modifications for 802.11p)
 * @author David Eckhoff (rewrote 802.11p slim model)
 */

#ifndef ___MAC80211P_H_
#define ___MAC80211P_H_

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include "BaseMacLayer.h"
#include <DroppedPacket.h>
#include <Mac80211Pkt_m.h>
#include "Mac1609_4To80211pControlInfo.h"
#include <Mac80211pToMac1609_4Interface.h>
#include <WaveShortMessage_m.h>
#include "FWMath.h"
#include <Decider80211p.h>
#include <BaseArp.h>
#include <MacToPhyControlInfo.h>
#include <PhyToMacControlInfo.h>
#include <NetwToMacControlInfo.h>
#include <MacToNetwControlInfo.h>
#include "MacToPhyInterface.h"
#include <SimpleAddress.h>
#include "FindModule.h"
#include <Consts80211p.h>
#include <BaseConnectionManager.h>

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath()

class  Mac80211p : public BaseMacLayer {

	protected:
		class ContentionParameters {
			private:
				/** @name Parameters for CSMA/CA.*/
				/*@{*/
				int aifsn;
				int cwMin;
				int cwMax;
				/*@}*/

			public:
				/** @brief Constructor to fill all variables of this class*/
				ContentionParameters(int aifsn, int cwMin, int cwMax) : aifsn(aifsn), cwMin(cwMin), cwMax(cwMax) { }

				/** @brief Gets value of aifsn*/
				int getAifsn()	{
					return aifsn;
				}

				/** @brief Gets value of cwMax*/
				int getCwMax()  {
					return cwMax;
				}

				/** @brief Gets value of cwMin*/
				int getCwMin()  {
					return cwMin;
				}
		};

	public:

		~Mac80211p() { };

		/** @brief Initialization of the module and some variables*/
		virtual void initialize(int);

		/** @brief Delete all dynamically allocated objects of the module*/
		virtual void finish();

		/** @brief Handle messages from lower layer */
		virtual void handleLowerMsg(cMessage*);

		/** @brief Handle messages from upper layer */
		virtual void handleUpperMsg(cMessage*);

		/** @brief Handle self messages such as timers */
		virtual void handleSelfMsg(cMessage*);

		/** @brief Handle control messages from lower layer */
		virtual void handleLowerControl(cMessage* msg);

	protected:

		/** @name Different tracked statistics.*/
		/*@{*/
		long statsReceivedPackets;
		long statsReceivedBroadcasts;
		long statsSentPackets;
		long statsLostPackets;
		long statsNumBackoffs;
		long statsDroppedPackets;
		long statsNumTooLittleTime;
		double statsTotalBackoffDuration;
		/*@}*/

		/** @brief MAC states
		 * see states diagram.
		 */
		enum t_mac_states {
			IDLE_1=1,
			BACKOFF_2,
			CCA_3,
			TRANSMITFRAME_4,
		};

		/*************************************************************/
		/****************** TYPES ************************************/
		/*************************************************************/

		/** @brief Kinds for timer messages.*/
		enum t_mac_timer {
			TIMER_NULL=0,
			TIMER_BACKOFF,
			TIMER_CCA,
		};

		/** @name Pointer for timer messages.*/
		/*@{*/
		cMessage* backoffTimer, *ccaTimer;
		/*@}*/

		/** @brief MAC state machine events.
		 * See state diagram.*/
		enum t_mac_event {
			EV_SEND_REQUEST=1,                   // 1, 11, 20, 21, 22
			EV_TIMER_BACKOFF,                    // 2, 7, 14, 15
			EV_FRAME_TRANSMITTED,                // 4, 19
			EV_FRAME_RECEIVED,                   // 15, 26
			EV_BROADCAST_RECEIVED, 		   // 23, 24
			EV_TIMER_CCA,
		};

		enum t_mac_carrier_sensed {
			CHANNEL_BUSY=1,
			CHANNEL_FREE
		} ;

		enum t_mac_status {
			STATUS_OK=1,
			STATUS_ERROR,
			STATUS_RX_ERROR,
			STATUS_RX_TIMEOUT,
			STATUS_FRAME_TO_PROCESS,
			STATUS_NO_FRAME_TO_PROCESS,
			STATUS_FRAME_TRANSMITTED
		};

		/** @brief keep track of MAC state */
		t_mac_states macState;
		t_mac_status status;

		bool transmissionAttemptInterruptedByRx;

		int macMaxCSMABackoffs;

		/** @brief maximum number of frame retransmissions without ack */
		unsigned int macMaxFrameRetries;

		/** @brief The power (in mW) to transmit with.*/
		double txPower;

		/** @brief number of backoff performed until now for current frame */
		int currentNumBackoffs;

		/** @brief current contention window size */
		int currentCW;

		/** @brief A queue to store packets from upper layer in case another
		packet is still waiting for transmission..*/
		Mac80211Pkt* packetToSend;

		/** @brief count the number of tx attempts
		 *
		 * This holds the number of transmission attempts for the current frame.
		 */
		unsigned int txAttempts;

		/** @brief the bit rate at which we transmit */
		double bitrate;

		/** @brief Inspect reasons for dropped packets */
		DroppedPacket droppedPacket;

		/** @brief publish dropped packets nic wide */
		int myNicId;

		/** @brief This MAC layers MAC address.*/
		int myMacAddress;

		/** @brief vector that stores the borders of the contention window for different priorities in 802.11p.*/
		std::vector<ContentionParameters*> SCHcontentionParamaters;
		std::vector<ContentionParameters*> CCHcontentionParamaters;

	protected:
		// FSM functions
		void fsmError(t_mac_event event, cMessage* msg);
		void executeMac(t_mac_event event, cMessage* msg);
		void updateStatusIdle(t_mac_event event, cMessage* msg);
		void updateStatusBackoff(t_mac_event event, cMessage* msg);
		void updateStatusCCA(t_mac_event event, cMessage* msg);
		void updateStatusTransmitFrame(t_mac_event event, cMessage* msg);
		void updateStatusNotIdle(cMessage* msg);
		void updateMacState(t_mac_states newMacState);
		double calculateAIFS() const;
		void attachSignal(Mac80211Pkt* mac, simtime_t startTime, double frequency);
		Signal* createSignal(simtime_t start, simtime_t length, double power, double bitrate, double frequency);
		void startTimer(t_mac_timer timer);
		void checkBitrate(int bitrate) const;
		virtual double calculateBackoff();

		Mac80211pToMac1609_4Interface* _1609_Mac;

};

#endif // ___MAC80211P_H_
