/**
 * @file PathLoss.h
 * @brief A path loss model.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 *
 * The implementation of \c getChannelState() is based on ChSim
 * written by Randy Vu, James Gross, Thorsten Pawlak, and
 * Stefan Valentin,
 **/

#include "PathLoss.h"

/**
 * Determined by the distance and medium-specific parameters the
 * path loss between two nodes is calculated.
 *
 * @param d Distance between the nodes in meters.
 * @param f Frequency in Hertz.
 * @param v Relative speed between the nodes in m/s.
 **/
double PathLoss::getChannelState(double d, double f, double v)
{
	// Return value in dB.
	return parSrc.par("tenlogk").doubleValue() - 10.0 * parSrc.par("pathloss").doubleValue() * log10(d);
}
