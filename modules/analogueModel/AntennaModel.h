#ifndef ANTENNAMODEL_H
#define ANTENNAMODEL_H

#include "SimplePathlossModel.h"
#include "ChannelAccess.h"

class AntennaModel: public AnalogueModel
{
protected:
	// store antenna radiation pattern here

    ChannelAccess* destinationChannelAccess;

    Coord relativeAntennaDirection;
    double relativeAntennaOrientation;

public:
	AntennaModel(double directionX, double directionY);  // initial direction

	/**
	 * @brief Filters a specified Signal by adding an attenuation
	 * over time to the Signal.
	 */
	virtual void filterSignal(Signal& s, bool isActiveAtOrigin);

	virtual bool isActiveAtDestination() { return true; }

	virtual bool isActiveAtOrigin() { return true; }

	virtual void setDestinationChannelAccess(ChannelAccess* ca) {
		destinationChannelAccess = ca;
	}

	/** @brief Returns the antenna gain on the requested angle (expressed in rad). */
	virtual double getAntennaGain(double angle);


};
#endif
