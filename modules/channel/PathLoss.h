/**
 * @file PathLoss.h
 * @brief A path loss model.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 **/

#ifndef __PATHLOSS_H
#define __PATHLOSS_H

#include "ChannelState.h"

/**
 * @brief Channel state implementing a path loss model.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 **/
class PathLoss : public ChannelState
{
public:
	/**
	 * @copydoc ChannelState::getChannelState()
	 **/
	virtual double getChannelState(double, double, double);

  PathLoss(cModule& m) : ChannelState(m) {}

};

#endif /* __PATHLOSS_H */
