/**
 * @file ChannelState.h
 * @brief Interface for channel state models.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 **/

#ifndef __CHANNELSTATE_H
#define __CHANNELSTATE_H

#include <omnetpp.h>

/**
 * @brief Interface for channel state models.
 * @author Hermann S. Lichte
 * @date 2007-08-14
 *
 * All models that influence the state of the wireless channel
 * must implement the ChannelState interface.
 **/
class ChannelState
{
protected:
	/**
	 * @brief Reference to a module containing parameters affecting
	 * the channel state.
	 **/
	cModule& parSrc;

public:
	/**
	 * @brief Instantiate a new channel state object.
	 * @param m Reference to a module where to obtain those parameters
	 * from that are needed to determine the channel state.
	 **/
	ChannelState(cModule& m) : parSrc(m) {}

	/**
	 * @brief Destructor for channel state objects.
	 **/
	virtual ~ChannelState() {}

	/**
	 * @brief Channel state hook.
	 * @return Returns the channel state in dB.
	 **/
	virtual double getChannelState(double, double, double) = 0;

	/**
	 * @brief Check whether instances of this class are global.
	 *
	 * If a channel state is declared as global only one instance
	 * of it will be used for all channels (e.g., path loss).
	 * Otherwise, the physical layer instantiates a channel state
	 * object for any channel when it is needed for the first time.
	 * This is useful when channels are correlated but yet every
	 * channel shall be statistically independent (e.g., fading).
	 *
	 * @return Returns true if only one instance of this class
	 * shall be used for all channels.
	 **/
	virtual bool isGlobal() { return true; }

};

#endif /* __CHANNELSTATE_H */
