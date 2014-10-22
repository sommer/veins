/*
 * DeciderResult80211.h
 *
 *  Created on: 04.02.2009
 *      Author: karl
 *
 *  Modified by Michele Segata (segata@ccs-labs.org)
 */

#ifndef DECIDERRESULT80211_H_
#define DECIDERRESULT80211_H_

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/phyLayer/Decider.h"

/**
 * @brief Defines an extended DeciderResult for the 80211 protocol
 * which stores the bit-rate of the transmission.
 *
 * @ingroup decider
 * @ingroup ieee80211
 */
class MIXIM_API DeciderResult80211 : public DeciderResult{
protected:
	/** @brief Stores the bit-rate of the transmission of the packet */
	double bitrate;

	/** @brief Stores the signal to noise ratio of the transmission */
	double snr;

	/** @brief Stores the received power in dBm
	 * Please note that this is NOT the RSSI. The RSSI is an indicator
	 * of the quality of the signal which is not standardized, and
	 * different vendors can define different indicators. This value
	 * indicates the power that the frame had when received by the
	 * NIC card, WITHOUT noise floor and WITHOUT interference
	 */
	double recvPower_dBm;

	/** @brief Stores whether the uncorrect decoding was due to low power or collision */
	bool collision;
public:

	/**
	 * @brief Initialises with the passed values.
	 *
	 * "bitrate" defines the bit-rate of the transmission of the packet.
	 */
	DeciderResult80211(bool isCorrect, double bitrate, double snr, double recvPower_dBm = 0, bool collision = false):
		DeciderResult(isCorrect), bitrate(bitrate), snr(snr), recvPower_dBm(recvPower_dBm), collision(collision) {}

	/**
	 * @brief Returns the bit-rate of the transmission of the packet.
	 */
	double getBitrate() const { return bitrate; }

	/**
	 * @brief Returns the signal to noise ratio of the transmission.
	 */
	double getSnr() const { return snr; }

	/**
	 * @brief Returns whether drop was due to collision, if isCorrect is false
	 */
	bool isCollision() const { return collision; }

	/**
	 * @brief Returns the signal power in dBm.
	 */
	double getRecvPower_dBm() const { return recvPower_dBm; }
};

#endif /* DECIDERRESULT80211_H_ */
