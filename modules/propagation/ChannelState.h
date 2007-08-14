#ifndef __CHANNELSTATE_H
#define __CHANNELSTATE_H

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
public:
	/**
	 * @brief Channel state hook. 
	 * @return Returns the channel state in dBm.
	 **/
	virtual double getChannelState() = 0;
};

#endif /* __CHANNELSTATE_H */
