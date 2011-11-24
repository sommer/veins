/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRStochasticPathlossModel.cc
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this AnalogueModel models free-space pathloss
 ***************************************************************************/

#include "UWBIRStochasticPathlossModel.h"

#include "BaseWorldUtility.h"
#include "AirFrame_m.h"

//const double UWBIRStochasticPathlossModel::Gtx = 0.9, UWBIRStochasticPathlossModel::Grx = 0.9, UWBIRStochasticPathlossModel::ntx = 0.9, UWBIRStochasticPathlossModel::nrx = 0.9;
const double UWBIRStochasticPathlossModel::Gtx = 1, UWBIRStochasticPathlossModel::Grx = 1, UWBIRStochasticPathlossModel::ntx = 1, UWBIRStochasticPathlossModel::nrx = 1;
const double UWBIRStochasticPathlossModel::fc = 4492.8; // mandatory band 3, center frequency, MHz
const double UWBIRStochasticPathlossModel::d0 = 1;

const double UWBIRStochasticPathlossModel::s_mu = 1.6, UWBIRStochasticPathlossModel::s_sigma = 0.5;
const double UWBIRStochasticPathlossModel::kappa = 1;

double UWBIRStochasticPathlossModel::n1_limit = 1.25;
double UWBIRStochasticPathlossModel::n2_limit = 2;

double UWBIRStochasticPathlossModel::simtruncnormal(double mean, double stddev, double a, int rng) {
    double res = a + 1;
    while(res > a || res < -a) {
      res = normal(mean, stddev, 0);
    }
    return res;
}

void UWBIRStochasticPathlossModel::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos)
{
	if (isEnabled) {
		Signal& signal = frame->getSignal();
		// Initialize objects and variables
		TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();
		Argument arg;

		// Generate channel.
		// assume channel coherence during whole frame

		n1 = simtruncnormal(0, 1, n1_limit, 1);
		n2 = simtruncnormal(0, 1, n2_limit, 2);
		n3 = simtruncnormal(0, 1, n2_limit, 3);

		gamma = mu_gamma + n1 * sigma_gamma;
		sigma = mu_sigma + n3 * sigma_sigma;
		S = n2 * sigma;

		// Determine distance between sender and receiver
		double distance    = receiverPos.distance(sendersPos);
		/*
		 srcPosX.record(senderPos.x);
		 srcPosY.record(senderPos.y);
		 dstPosX.record(receiverPos.x);
		 dstPosY.record(receiverPos.y);
		 distances.record(distance);
		 */
		// Compute pathloss
		double attenuation = getGhassemzadehPathloss(distance);
		pathlosses.record(attenuation);
		//attenuation = attenuation / (4*PI*pow(distance, gamma));
		// Store scalar mapping
		attMapping->setValue(arg, attenuation);
		signal.addAttenuation(attMapping);
	}
}

double UWBIRStochasticPathlossModel::getNarrowBandFreeSpacePathloss(double fc, double distance) {
    double attenuation = 4*M_PI*distance*fc/BaseWorldUtility::speedOfLight;
    return 1/(attenuation*attenuation);
}

double UWBIRStochasticPathlossModel::getGhassemzadehPathloss(double distance) {
    double attenuation = PL0;
    if(distance < d0) {
      distance = d0;
    }
    attenuation = attenuation - 10*gamma*log10(distance/d0);
    if (shadowing) {
      attenuation = attenuation - S;
    }
    attenuation = pow(10, attenuation/10); // from dB to mW
    return attenuation;
}

double UWBIRStochasticPathlossModel::getFDPathloss(double freq, double distance) {
    return 0.5*PL0*ntx*nrx*pow(freq/fc, -2*(kappa+1))/pow(distance/d0, pathloss_exponent);
}


