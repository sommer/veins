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
 * @brief Module that manages timeslots for CCH and SCH listening and sending.
 *
 * @class Mac1609_4
 * @author Christopher Saloman
 * @author David Eckhoff : rewrote complete model
 */

#ifndef ___MAC1609_4_H_
#define ___MAC1609_4_H_

#include <assert.h>
#include <omnetpp.h>
#include <queue>
#include <BaseLayer.h>
#include <Mac80211pToPhy11pInterface.h>
#include <Mac1609_4To80211pControlInfo.h>
#include <MacToPhyInterface.h>
#include <NetwToMacControlInfo.h>
#include <Mac80211pToPhy11pInterface.h>
#include <Mac80211pToMac1609_4Interface.h>
#include <WaveAppToMac1609_4Interface.h>
#include <Consts80211p.h>
#include "FindModule.h"
#include <Mac80211Pkt_m.h>
#include <WaveShortMessage_m.h>

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath()

class Mac1609_4 :	public BaseLayer,
	public Mac80211pToMac1609_4Interface,
	public WaveAppToMac1609_4Interface {
	public:
		~Mac1609_4() { };

		void changeServiceChannel(int channelNumber);

	protected:
		/** @brief States of the channel selecting operation.*/
		enum ChannelSelectorState {
			SCH=0,
			CCH=1
		};

	protected:
		/** @brief Initialization of the module and some variables.*/
		virtual void initialize(int);

		/** @brief Delete all dynamically allocated objects of the module.*/
		virtual void finish();

		/** @brief Handle messages from lower layer.*/
		virtual void handleLowerMsg(cMessage*);

		/** @brief Handle messages from upper layer.*/
		virtual void handleUpperMsg(cMessage*);

		/** @brief Handle control messages from upper layer.*/
		virtual void handleUpperControl(cMessage* msg);


		/** @brief Handle self messages such as timers.*/
		virtual void handleSelfMsg(cMessage*);

		/** @brief Handle control messages from lower layer.*/
		virtual void handleLowerControl(cMessage* msg);

		/** @brief Set a state for the channel selecting operation.*/
		void setCsState(ChannelSelectorState state);

		double timeLeft() const;
		double timeLeftTillGuardOver() const;
		bool guardActive() const;

		void attachAndSend(cMessage* msg, ChannelSelectorState channel, t_channel channelType, double frequency);

	protected:
		/** @brief Self message to indicate that the current channel shall be switched.*/
		cMessage* channel_switch;

		/** @brief Self message to indicate that the guard interval is over.*/
		cMessage* guard_over;

		/** @brief Current state of the channel selecting operation.*/
		ChannelSelectorState csState;
		ChannelSelectorState lastPacketCameFrom;

		/** @brief Interfaces in order to control both instances of upperMac.*/
		std::queue<cMessage*> schQueue;
		std::queue<cMessage*> cchQueue;

		/** @brief Interface in order to change listening and sending frequency of phy layer*/
		Mac80211pToPhy11pInterface* phy11p;

		/** @brief Physical parameters as defined in 802.11p*/
		double dot4SyncTolerance;
		double dot4MaxChSwitchTime;
		double dot4CchInterval;
		double dot4SchInterval;

		/** @brief Stores the frequencies in Hz, that are associated to the channel numbers.*/
		std::map<int,double> frequency;

		int packets_arrived;
		int packets_received;
		int packets_successfully_sent;
		int intervalTooShort;

		int headerLength;

		int mySCH;
};

#endif /* ___MAC1609_4_H_*/
