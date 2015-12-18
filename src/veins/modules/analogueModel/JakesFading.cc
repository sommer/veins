//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "veins/modules/analogueModel/JakesFading.h"

#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/messages/AirFrame_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"

using Veins::AirFrame;
using Veins::ChannelAccess;

double JakesFadingMapping::getValue(const Argument& pos) const {
	double f = model->carrierFrequency;
	double v = relSpeed;
	simtime_t t = pos.getTime();
	double re_h = 0;
	double im_h = 0;

	// Compute Doppler shift.
	double doppler_shift = v * f / BaseWorldUtility::speedOfLight();

	for (int i = 0; i < model->fadingPaths; i++) {
		// Some math for complex numbers:
		//
		// Cartesian form: z = a + ib
		// Polar form:     z = p * e^i(phi)
		//
		// a = p * cos(phi)
		// b = p * sin(phi)
		// z1 * z2 = p1 * p2 * e^i(phi1 + phi2)

		// Phase shift due to Doppler => t-selectivity.
		double phi_d = model->angleOfArrival[i] * doppler_shift;
		// Phase shift due to delay spread => f-selectivity.
		double phi_i = SIMTIME_DBL(model->delay[i]) * f;
		// Calculate resulting phase due to t-selective and f-selective fading.
		double phi = 2.00 * M_PI * (phi_d * SIMTIME_DBL(t) - phi_i);

		// One ring model/Clarke's model plus f-selectivity according to Cavers:
		// Due to isotropic antenna gain pattern on all paths only a^2 can be received on all paths.
		// Since we are interested in attenuation a:=1, attenuation per path is then:
		double attenuation = (1.00 / sqrt(static_cast<double>(model->fadingPaths)));

		// Convert to cartesian form and aggregate {Re, Im} over all fading paths.
		re_h = re_h + attenuation * cos(phi);
		im_h = im_h - attenuation * sin(phi);
	}

	// Output: |H_f|^2 = absolute channel impulse response due to fading.
	// Note that this may be >1 due to constructive interference.
	return re_h * re_h + im_h * im_h;
}


JakesFading::JakesFading(int fadingPaths, simtime_t_cref delayRMS,
						 double carrierFrequency, simtime_t_cref interval):
	fadingPaths(fadingPaths),
	carrierFrequency(carrierFrequency),
	interval(interval)
{
	angleOfArrival = new double[fadingPaths];
	delay = new simtime_t[fadingPaths];

	for (int i = 0; i < fadingPaths; i++) {
		angleOfArrival[i] = cos(RNGCONTEXT uniform(0, M_PI));
		delay[i] = (RNGCONTEXT exponential(delayRMS));
	}
}

JakesFading::~JakesFading() {
	delete[] delay;
	delete[] angleOfArrival;
}

void JakesFading::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos)
{
	Signal&                signal           = frame->getSignal();
	ChannelMobilityPtrType senderMobility   = dynamic_cast<ChannelAccess *>(frame->getSenderModule())->getMobilityModule();
	ChannelMobilityPtrType receiverMobility = dynamic_cast<ChannelAccess *>(frame->getArrivalModule())->getMobilityModule();
	const double           relSpeed         = (senderMobility->getCurrentSpeed() - receiverMobility->getCurrentSpeed()).length();

	signal.addAttenuation(new JakesFadingMapping(this, relSpeed,
	                                             Argument(signal.getReceptionStart()),
	                                             interval,
	                                             Argument(signal.getReceptionEnd())));
}
