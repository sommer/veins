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

#ifndef BASEWAVEAPPLLAYER_H_
#define BASEWAVEAPPLLAYER_H_

#include <map>
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath() << " "

/**
 * @brief
 * WAVE application layer base class.
 *
 * @author David Eckhoff
 *
 * @ingroup applLayer
 *
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class BaseWaveApplLayer : public BaseApplLayer {

	public:
		~BaseWaveApplLayer();
		virtual void initialize(int stage);
		virtual void finish();

		virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details);

		enum WaveApplMessageKinds {
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			SEND_BEACON_EVT
		};

	protected:

		static const simsignalwrap_t mobilityStateChangedSignal;

		/** @brief handle messages from below */
		virtual void handleLowerMsg(cMessage* msg);
		/** @brief handle self messages */
		virtual void handleSelfMsg(cMessage* msg);

		virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
		virtual void sendWSM(WaveShortMessage* wsm);
		virtual void onBeacon(WaveShortMessage* wsm) = 0;
		virtual void onData(WaveShortMessage* wsm) = 0;

		virtual void handlePositionUpdate(cObject* obj);

	protected:
		int beaconLengthBits;
		int beaconPriority;
		bool sendData;
		bool sendBeacons;
		simtime_t individualOffset;
		int dataLengthBits;
		bool dataOnSch;
		int dataPriority;
		Coord curPosition;
		int mySCH;
		int myId;

		cMessage* sendBeaconEvt;

		WaveAppToMac1609_4Interface* myMac;
};

#endif /* BASEWAVEAPPLLAYER_H_ */
