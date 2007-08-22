/**
 * @file LogNormalShadowing.h
 * @brief Log-normal shadowing.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 **/

#ifndef __LOGNORMALSHADOWING_H
#define __LOGNORMALSHADOWING_H

#include "ChannelState.h"

/**
 * @brief Channel state implementing log-normal shadowing.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 **/
class LogNormalShadowing : public ChannelState
{
public:
	/**
	 * @copydoc ChannelState::getChannelState()
	 **/
	virtual double getChannelState(double, double, double);

  LogNormalShadowing(cModule& m) : ChannelState(m) {}

};

#endif /* __LOGNORMALSHADOWING_H */
