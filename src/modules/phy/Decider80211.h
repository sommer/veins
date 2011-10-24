/*
 * Decider80211.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef DECIDER80211_H_
#define DECIDER80211_H_

#include "MiXiMDefs.h"
#include "BaseDecider.h"

/**
 * @brief Decider for the 802.11 modules
 *
 * Depending on the minimum of the snr included in the PhySDU this
 * module computes a bit error probability. The header (1 Mbit/s) is
 * always modulated with DBQPSK. The PDU is normally modulated either
 * with DBPSK (1 and 2 Mbit/s) or CCK (5.5 and 11 Mbit/s). CCK is not
 * easy to model, therefore it is modeled as DQPSK with a 16-QAM for
 * 5.5 Mbit/s and a 256-QAM for 11 Mbit/s.
 *
 *
 * @ingroup decider
 * @ingroup ieee80211
 * @author Marc L�bbers, David Raguin, Karl Wessel(port for MiXiM)
 */
class MIXIM_API Decider80211: public BaseDecider {
public:
	/** @brief Control message kinds used by this Decider.*/
	enum Decider80211ControlKinds {
		NOTHING = 22100,
		/** @brief The decider has recognized a bit error in the packet.*/
		BITERROR,
		/** @brief Packet lost due to collision.*/
		COLLISION,
		/** @brief Sub-classing deciders should begin their own kinds at this value.*/
		LAST_DECIDER_80211_CONTROL_KIND
	};
protected:
	/** @brief threshold value for checking a SNR-map (SNR-threshold)*/
	double snrThreshold;

	/** @brief The center frequency on which the decider listens for signals */
	double centerFrequency;

protected:

	/**
	 * @brief Checks if the passed completed AirFrame was received correctly.
	 *
	 * Returns the result as a DeciderResult
	 *
	 * @return	The result of the decider for the passed AirFrame.
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
	 * @brief Calculates the RSSI value for the passed interval.
	 *
	 * This method is called by BaseDecider when it answers a ChannelSenseRequest
	 * or calculates the current channel state.
	 *
	 * Returns the maximum RSSI value inside the passed time
	 * interval and the channel the Decider currently listens to.
	 */
	virtual double calcChannelSenseRSSI(simtime_t_cref start, simtime_t_cref end);

public:

	/**
	 * @brief Initializes the Decider with a pointer to its PhyLayer and
	 * specific values for threshold and sensitivity
	 */
	Decider80211(DeciderToPhyInterface* phy,
				double threshold,
				double sensitivity,
				int channel,
				int myIndex = -1,
				bool debug = false);

	virtual ~Decider80211() {};
};

#endif /* DECIDER80211_H_ */
