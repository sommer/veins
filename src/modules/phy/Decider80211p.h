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

/*
 * Based on Decider80211.h from Karl Wessel
 * and modifications by Christopher Saloman
 */

#ifndef DECIDER80211_H_
#define DECIDER80211_H_

#include <BaseDecider.h>
#include <Consts80211p.h>

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << myPath

class Decider80211p: public BaseDecider
{
	public:
		enum Decider80211ControlKinds {
			NOTHING = 22100,
			BITERROR,       //the phy has recognized a bit error in the packet
			LAST_DECIDER_80211_CONTROL_KIND
		};
	protected:
		// threshold value for checking a SNR-map (SNR-threshold)
		double snrThreshold;

		/** @brief The center frequency on which the decider listens for signals */
		double centerFrequency;

		double myBusyTime;
		double myStartTime;
		std::string myPath;

	protected:

		/**
		 * @brief Checks a mapping against a specific threshold (element-wise).
		 *
		 * @return	true	, if every entry of the mapping is above threshold
		 * 			false	, otherwise
		 *
		 *
		 */
		virtual DeciderResult* checkIfSignalOk(AirFrame* frame);

		virtual simtime_t processNewSignal(AirFrame* frame);

		/**
		 * @brief Processes a received AirFrame.
		 *
		 * The SNR-mapping for the Signal is created and checked against the Deciders
		 * SNR-threshold. Depending on that the received AirFrame is either sent up
		 * to the MAC-Layer or dropped.
		 *
		 * @return	usually return a value for: 'do not pass it again'
		 */
		virtual simtime_t processSignalEnd(AirFrame* frame);

		/** @brief computes if packet is ok or has errors*/
		bool packetOk(double snirMin, int lengthMPDU, double bitrate);

		/**
		 * @brief Calculates the RSSI value for the passed ChannelSenseRequest.
		 *
		 * This method is called by BaseDecider when it answers a ChannelSenseRequest
		 * and can be overridden by sub classing Deciders.
		 *
		 * Returns the maximum RSSI value inside the ChannelSenseRequest time
		 * interval and the channel the Decider currently listens to.
		 */
		virtual double calcChannelSenseRSSI(simtime_t_cref min, simtime_t_cref max);

	public:

		/**
		 * @brief Initializes the Decider with a pointer to its PhyLayer and
		 * specific values for threshold and sensitivity
		 */
		void changeFrequency(double freq);

		Decider80211p(DeciderToPhyInterface* phy,
		              double sensitivity,
		              double centerFrequency,
		              int myIndex = -1,
		              bool debug = false):
			BaseDecider(phy, sensitivity, myIndex, debug),
			centerFrequency(centerFrequency),
			myBusyTime(0),
			myStartTime(simTime().dbl()) {

		}

		void setPath(std::string myPath) {
			this->myPath = myPath;
		}

		virtual ~Decider80211p();
};

#endif /* DECIDER80211_H_ */
