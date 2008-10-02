/**
 * @file JakesFadingModel.cc
 * @brief Jakes-like fading model.
 * @author Hermann S. Lichte
 * @date 2007-08-15
 *
 * The implementation of \c getChannelState() is based on ChSim
 * written by Randy Vu, James Gross, Thorsten Pawlak, and
 * Stefan Valentin,
 **/

#include "JakesFadingModel.h"
#include "BaseWorldUtility.h"
#include "FWMath.h"

/**
 * Initialize fading for a particular subcarrier.
 *
 * @param s Number of subcarrier to initialize fading for.
 **/
JakesFadingModel::JakesFadingModel(cModule& m, int s) : ChannelState(m), subcarrier(s)
{
	fadingPaths = parSrc.par("fadingPaths");
	angleOfArrival = new double[fadingPaths];
	delay = new double[fadingPaths];

	for (int i = 0; i < fadingPaths; i++) {
		angleOfArrival[i] = cos(uniform(0, M_PI));
		delay[i] = static_cast<double>(exponential(parSrc.par("delayRMS")));
	}
}

/**
 * Clean up dynamically allocated arrays.
 **/
JakesFadingModel::~JakesFadingModel()
{
	delete[] delay;
	delete[] angleOfArrival;
}

/**
 * @param d Distance between the nodes in meters.
 * @param f Frequency in Hertz.
 * @param v Relative speed between the nodes in m/s.
 **/
double JakesFadingModel::getChannelState(double d, double f, double v)
{
	double re_h = 0;
	double im_h = 0;

	// Compute Doppler shift.
	double doppler_shift = v * f / BaseWorldUtility::speedOfLight;

	for (int i = 0; i < fadingPaths; i++) {
		// Some math for complex numbers:
		//
		// Cartesian form: z = a + ib
		// Polar form:     z = p * e^i(phi)
		//
		// a = p * cos(phi)
		// b = p * sin(phi)
		// z1 * z2 = p1 * p2 * e^i(phi1 + phi2)

		// Phase shift due to Doppler => t-selectivity.
		double phi_d = angleOfArrival[i] * doppler_shift;
		// Phase shift due to delay spread => f-selectivity.
		double phi_i = delay[i] * f;
		// Calculate resulting phase due to t-selective and f-selective fading.
		double phi = 2.00 * M_PI * (phi_d * simTime() - phi_i);

		// One ring model/Clarke's model plus f-selectivity according to Cavers:
		// Due to isotropic antenna gain pattern on all paths only a^2 can be received on all paths.
		// Since we are interested in attenuation a:=1, attenuation per path is then:
		double attenuation = (1.00 / sqrt(static_cast<double>(fadingPaths)));

		// Convert to cartesian form and aggregate {Re, Im} over all fading paths.
		re_h = re_h + attenuation * cos(phi);
		im_h = im_h - attenuation * sin(phi);
	}

	// Output: |H_f|^2 = absolute channel impulse response due to fading in dB.
	// Note that this may be >0 dB due to constructive interference.
	return FWMath::mW2dBm(re_h * re_h + im_h * im_h);
}
