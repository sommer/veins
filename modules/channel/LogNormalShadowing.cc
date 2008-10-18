/**
 * @file LogNormalShadowing.h
 * @brief Log-normal shadowing.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 *
 * The implementation of \c getChannelState() is based on ChSim
 * written by Randy Vu, James Gross, Thorsten Pawlak, and
 * Stefan Valentin,
 **/

#include "LogNormalShadowing.h"

/**
 * Calculates shadowing loss based on a normal Gaussian function.
 *
 * @param d Distance between the nodes in meters.
 * @param f Frequency in Hertz.
 * @param v Relative speed between the nodes in m/s.
 **/
double LogNormalShadowing::getChannelState(double d, double f, double v)
{
	// Return value in dB.
	return -1.0 * normal(parSrc.par("mean"), parSrc.par("stdDev"));
}
