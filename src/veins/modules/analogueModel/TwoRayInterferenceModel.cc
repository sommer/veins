//
// Copyright (C) 2011 Stefan Joerer <stefan.joerer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;

#define debugEV EV << "PhyLayer(TwoRayInterferenceModel): "

void TwoRayInterferenceModel::filterSignal(AirFrame *frame, const Coord& senderPos, const Coord& receiverPos) {
	Signal& s = frame->getSignal();

	const Coord senderPos2D(senderPos.x, senderPos.y);
	const Coord receiverPos2D(receiverPos.x, receiverPos.y);

	ASSERT(senderPos.z > 0); // make sure send antenna is above ground
	ASSERT(receiverPos.z > 0); // make sure receive antenna is above ground

	double d = senderPos2D.distance(receiverPos2D);
	double ht = senderPos.z, hr = receiverPos.z;

	debugEV << "(ht, hr) = (" << ht << ", " << hr << ")" << endl;

	double d_dir = sqrt( pow (d,2) + pow((ht - hr),2) ); // direct distance
	double d_ref = sqrt( pow (d,2) + pow((ht + hr),2) ); // distance via ground reflection
	double sin_theta = (ht + hr)/d_ref;
	double cos_theta = d/d_ref;

	double gamma = (sin_theta - sqrt(epsilon_r - pow(cos_theta,2)))/
		(sin_theta + sqrt(epsilon_r - pow(cos_theta,2)));

	//is the signal defined to attenuate over frequency?
	bool hasFrequency = s.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency());
	debugEV << "Signal contains frequency dimension: " << (hasFrequency ? "yes" : "no") << endl;

	assert(hasFrequency);

	debugEV << "Add TwoRayInterferenceModel attenuation (gamma, d, d_dir, d_ref) = (" << gamma << ", " << d << ", " << d_dir << ", " << d_ref << ")" << endl;

        s.addAttenuation(new TwoRayInterferenceModel::Mapping(gamma, d, d_dir, d_ref, debug));
}

double TwoRayInterferenceModel::Mapping::getValue(const Argument& pos) const {

	assert(pos.hasArgVal(Dimension::frequency()));
	double freq = pos.getArgValue(Dimension::frequency());
	double lambda = BaseWorldUtility::speedOfLight() / freq;
	double phi =  ( 2*M_PI/lambda * (d_dir - d_ref) );
	double att = pow(4 * M_PI * (d/lambda) *
				1/(sqrt(
					(pow((1 + gamma * cos(phi)),2)
					+ pow(gamma,2) * pow(sin(phi),2))
				))
			, 2);
	debugEV << "Add attenuation for (freq, lambda, phi, gamma, att) = (" << freq << ", " << lambda << ", " << phi << ", " << gamma << ", " << (1/att) << ", " << FWMath::mW2dBm(att) << ")" << endl;
	return 1/att;
}

