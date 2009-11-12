/*
 * DeciderResult80211.h
 *
 *  Created on: 04.02.2009
 *      Author: karl
 */

#ifndef DECIDERRESULT802154NARROW_H_
#define DECIDERRESULT802154NARROW_H_

#include <Decider.h>

/**
 * @brief Defines an extended DeciderResult for the 802.15.4 protocol.
 *
 * @ingroup decider
 */
class DeciderResult802154Narrow : public DeciderResult {
protected:
	/** @brief Stores the bit-rate of the transmission of the packet */
	double bitrate;

	/** @brief Stores the minimum signal to noise ratio of the transmission */
	double snr;

	double ber;
	double rssi;
public:

	/**
	 * @brief Initialises with the passed values.
	 */
	DeciderResult802154Narrow(bool isCorrect, double bitrate, double snr, double ber, double rssi):
		DeciderResult(isCorrect),
		bitrate(bitrate),
		snr(snr),
		ber(ber),
		rssi(rssi)
	{}

	/**
	 * @brief Returns the bit-rate of the transmission of the packet.
	 */
	double getBitrate() const { return bitrate; }

	/**
	 * @brief Returns the signal to noise ratio of the transmission.
	 */
	double getSnr() const { return snr; }

	double getBER() const { return ber; }
	double getRSSI() const { return rssi; }
};

#endif /* DECIDERRESULT80211_H_ */
