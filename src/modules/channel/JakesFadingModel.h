/**
 * @file JakesFadingModel.h
 * @brief Jakes-like fading model.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 **/

#ifndef __JAKESFADINGMODEL_H
#define __JAKESFADINGMODEL_H

#include "ChannelState.h"

/**
 * @brief Channel state implementing a Jakes-like fading model.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 **/
class JakesFadingModel : public ChannelState
{
protected:
	/**
	 * @brief Number of the subcarrier whose fading is modeled.
	 **/
	int subcarrier;

	/**
	 * @brief Number of fading paths used.
	 **/
	int fadingPaths;

	/**
	 * @brief Angle of arrival on a fading path used for Doppler shift calculation.
	 **/
	double* angleOfArrival;

	/**
	 * @brief Delay on a fading path.
	 **/
	double* delay;

public:
	/**
	 * @copydoc ChannelState::ChannelState()
	 **/
	JakesFadingModel(cModule&, int);

	/**
	 * @copydoc ChannelState::~ChannelState()
	 **/
	virtual ~JakesFadingModel();

	/**
	 * @copydoc ChannelState::getChannelState()
	 **/
	virtual double getChannelState(double, double, double);

	/**
	 * @copydoc ChannelState::isGlobal()
	 **/
	virtual bool isGlobal() { return false; }

};

#endif /* __JAKESFADINGMODEL_H */
