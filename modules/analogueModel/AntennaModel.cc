#include "AntennaModel.h"

AntennaModel::AntennaModel(double directionX, double directionY)
{
	// get antenna model from PHY
	// initialize antenna orientation
	relativeAntennaOrientation = atan2(directionY, directionX);  // in rad
}

void AntennaModel::filterSignal(Signal& s, bool isActiveAtOrigin)
{
	/** Get start of the signal */
	simtime_t sStart = s.getSignalStart();
	simtime_t sEnd = s.getSignalLength() + sStart;

	/** claim the Move pattern of the sender from the Signal */
	Coord senderPos = s.getMove().getPositionAt(sStart);
	Coord rxPos =  destinationChannelAccess->getMove().getPositionAt(sStart);

	double sqrDistance = rxPos.sqrdist(senderPos);

	Coord signalDirection = rxPos - senderPos;
	signalDirection = signalDirection / signalDirection.length();  // normalize

	// obtain "absolute" orientation of this antenna:
	// host direction + antenna direction, normalized
	Coord hostDirection;

	if(isActiveAtOrigin) {
		hostDirection = s.getMove().getDirection();
	} else {
		hostDirection = destinationChannelAccess->getMove().getDirection();
	}
	double hostOrientation = atan2(hostDirection.getY(), hostDirection.getX());  // in rad

	double antennaOrientation = hostOrientation + relativeAntennaOrientation;  // in rad


	if(!isActiveAtOrigin) {
		// we are modeling a receiving antenna
		// revert the direction
		signalDirection = Coord(-signalDirection.getX(), -signalDirection.getY());
	}

	double signalOrientation = atan2(signalDirection.getY(), signalDirection.getX());
	double attenuation = getAntennaGain(signalOrientation - antennaOrientation);
	// combine absolute antenna orientation and interpolated radiation pattern

	TimeMapping<Linear>* attMapping = new TimeMapping<Linear>();
	Argument arg;
	attMapping->setValue(arg, attenuation);
	s.addAttenuation(attMapping);
}

double AntennaModel::getAntennaGain(double angle)
{
	double gain = 0;
	// clean up angle value
	while(angle > PI) {
		angle = angle - 2*PI;
	}
	while(angle < -PI) {
		angle = angle + 2*PI;
	}
	assert(angle <= PI && angle >= -PI);
	if(angle > -PI/4 && angle < PI/4) {
		gain = 1;
	} else if ( angle >= 3*PI/4 || angle <= -3*PI/4) {
		gain = 1;
	}
	return gain;
}
