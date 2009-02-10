/*
 * DeciderResult80211.h
 *
 *  Created on: 04.02.2009
 *      Author: karl
 */

#ifndef DECIDERRESULT80211_H_
#define DECIDERRESULT80211_H_

#include <Decider.h>

/**
 * @brief Defines an extended DeciderResult for the 80211 protocol
 * which stores the bit-rate of the transmission.
 */
class DeciderResult80211 : public DeciderResult{
protected:
	/** @brief Stores the bit-rate of the transmission of the packet */
	double bitrate;
public:

	/**
	 * @brief Initialises with the passed values.
	 *
	 * "bitrate" defines the bit-rate of the transmission of the packet.
	 */
	DeciderResult80211(bool isCorrect, double bitrate):
		DeciderResult(isCorrect), bitrate(bitrate) {}

	/**
	 * @brief Returns the bit-rate of the transmission of the packet.
	 */
	double getBitrate() const { return bitrate; }
};

#endif /* DECIDERRESULT80211_H_ */
