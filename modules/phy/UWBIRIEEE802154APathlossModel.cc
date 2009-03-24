/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRIEEE802154APathlossModel.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
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
 * description: this AnalogueModel implements the IEEE 802.15.4A Channel Model
 ***************************************************************************/

#include "UWBIRIEEE802154APathlossModel.h"

const bool UWBIRIEEE802154APathlossModel::implemented_CMs[] = {
        		false, //  There is no Channel Model 0: start at 1
        		true,  //  CM1
        		true,  //  CM2
        		true,  //  CM3
        		false, //  CM4 (requires alternate PDP)
        		true,  //  CM5
        		true,  //  CM6
        		true,  //  CM7
        		false,  //  CM8 (alternate PDP)
        		false  //  CM9 (requires alternate PDP)
        };

const UWBIRIEEE802154APathlossModel::CMconfig UWBIRIEEE802154APathlossModel::CMconfigs[] = {
		// CM0
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		// *CM1*
			//PL0, 			n, sigma_s (undefined)
		{	0.000040738, 1.79, 	0,
			//Aant, kappa, Lmean, 	Lambda
			3, 		1.12, 	3, 		0.047E9,
			//lambda_1, lambda_2, 	Beta
			1.54E9, 	0.15E9, 	0.095,
			//Gamma, k_gamma, gamma_0, sigma_cluster,
			22.61E-9, 0, 	12.53E-9, 1.883649089,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.67, 	0, 	0.28, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		// *CM2*
			//PL0, 			n, sigma_s (undefined)
		{	0.00001349, 4.58, 0,
			//Aant, kappa, Lmean, 	Lambda
			3, 		1.12, 	3.5,	0.12E9,
			//lambda_1, lambda_2, 	Beta
			1.77E9, 	0.15E9, 	0.045,
			//Gamma, k_gamma, gamma_0, sigma_cluster,
			26.27E-9, 0, 	17.5E-9,  1.963360277,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.69, 	0, 	0.32, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		// *CM3*
			//PL0, 			n, sigma_s (undefined)
		{	0.000218776, 1.63, 1.9,
			//Aant, kappa, Lmean, 	Lambda
			3, 		-3.5, 	5.4,	0.016E9,
			//lambda_1, lambda_2, 	Beta
			0.19E9, 	2.97E9, 	0.0184,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			14.6E-9, 0, 	6.4E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.42, 	0, 	0.31, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		// *CM4*
			//PL0, 			n, sigma_s (undefined)
		{	0.000007244, 3.07, 3.9,
			//Aant, kappa, Lmean, 	Lambda (undefined)
			3, 		-5.3, 	1,	0,
			//lambda_1, lambda_2, 	Beta (all undefined)
			0, 	0, 	0,
			//Gamma, k_gamma, gamma_0, sigma_cluster (all undefined),
			0, 0, 	0,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.5, 	0, 	0.25, 		0, 		0,
			//gamma_rise, gamma_1, xi
			15.21, 11.84, 0.86},

		// *CM5* Outdoor LOS
			//PL0, 			n, sigma_s
		{	0.000046881, 1.76, 0.83,
			//Aant, kappa, Lmean, 	Lambda
			3, 		-1.6, 	13.6,	0.0048E9,
			//lambda_1, lambda_2, 	Beta
			0.27E9, 	2.41E9, 	0.062,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			31.7E-9, 0, 	3.7E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.77, 	0, 	0.78, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

			// *CM6* Outdoor NLOS
				//PL0, 			n, sigma_s
		{	0.000046881, 2.5, 2,
			//Aant, kappa, Lmean, 	Lambda
			3, 		0.4, 	10.5,	0.0243E9,
			//lambda_1, lambda_2, 	Beta
			0.15E9, 	1.13E9, 	0.062,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			104.7E-9, 0, 	9.3E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			0.56, 	0, 	0.25, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

			// *CM7* Open Outdoor NLOS
				//PL0, 			n, sigma_s
		{	0.000012706, 1.58, 3.96,
			//Aant, kappa (?), Lmean, 	Lambda
			3, 		0, 	3.31,	0.0305E9,
			//lambda_1, lambda_2, 	Beta
			0.0225E9, 	0, 	0,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			56E-9, 0, 	0.92E-9,  0,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0 (undefined)
			4.1, 	0, 	2.5, 		0, 		0,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

			// *CM8* Industrial LOS
				//PL0, 			n, sigma_s
		{	0.000002138, 1.2, 6,
			//Aant, kappa (?), Lmean, 	Lambda
			3, 		-5.6, 	4.75,	0.0709E9,
			//lambda_1, lambda_2, 	Beta (all undefined)
			0, 	0, 	0,
			//Gamma, k_gamma, gamma_0, sigma_cluster (undefined),
			13.47E-9, 0.926, 	0.651E-9,  4.32,
			//m_0, k_m, var_m_0, var_k_m, strong_m_0
			0.36, 	0, 	1.13, 		0, 		12.99,
			//gamma_rise, gamma_1, xi (all undefined)
			0, 0, 0},

		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // CM9
};

void UWBIRIEEE802154APathlossModel::filterSignal(Signal& s) {
    // We create a new "fake" txPower to add multipath taps
    // and then attenuation is applied to all pulses.

	// *** Taps

    txPower = s.getTransmissionPower();
    newTxPower = dynamic_cast<TimeMapping<Linear>*> (txPower->clone());
    // number of clusters for this channel (channel coherence time > packet air time)
    L = poisson(cfg.Lmean, 0);

    // Loop on each value of the mapping and generate multipath echoes
    ConstMappingIterator* iter = txPower->createConstIterator();

    while (iter->inRange()) {
        // generate echoes for each non zero value
        if (iter->getValue() > 0) {
            addEchoes(iter->getPosition().getTime());
        }
        if (!iter->hasNext()) {
            break;
        }
        iter->next();
    }
    delete iter;
    s.setTransmissionPower(newTxPower);

    //*** Pathloss
    //TODO: refactor to pathloss.h or ieee....h
    //double Gtx = 1, Grx = 1;
    double ntx = 1, nrx = 1;
    //double fc = 4492.8; // mandatory band 3, center frequency, MHz
    double d0 = 1;

    // compute distance
    Move srcMove = s.getMove();
    Coord srcCoord, rcvCoord;
    double distance = 0;
    srcCoord = srcMove.getPositionAt(s.getSignalStart());
    rcvCoord = move.getPositionAt(s.getSignalStart());
    distance = rcvCoord.distance(srcCoord);

    // compute attenuation
    //double attenuation = ntx * nrx * cfg.PL0 / pow(distance / d0, cfg.n);


    double attenuation = 0.5 * ntx * nrx * cfg.PL0 / pow(distance / d0, cfg.n);
    //attenuation = attenuation / pow(fc, 2*(cfg.kappa+1));
    //attenuation = attenuation * (pow(fc+0.25E9,1-2*(cfg.kappa+1))-pow(fc-0.25E9,1-2*(cfg.kappa+1)) );
    // create mapping
    SimpleTimeConstMapping* attMapping = new SimpleTimeConstMapping(
    		attenuation, s.getSignalStart(), s.getSignalStart()+s.getSignalLength());

    s.addAttenuation(attMapping);

}

void UWBIRIEEE802154APathlossModel::addEchoes(simtime_t pulseStart) {
    // loop control variables
    bool moreTaps = true;

    simtime_t tau_kl = 0;
    simtime_t fromClusterStart = 0;
    // start time of cluster number "cluster"
    clusterStart = 0;
    gamma_l = cfg.gamma_0;
    Mcluster = normal(0, cfg.sigma_cluster, 1);
    Omega_l = pow(10, Mcluster / 10);
    double tapEnergy = sqrt( Omega_l / ( cfg.gamma_0 * ( (1-cfg.Beta)*cfg.lambda_1 + cfg.Beta*cfg.lambda_2 + 1 ) ) );

    for (int cluster = 0; cluster < L; cluster++) {
        while (moreTaps) {
            // modify newTxPower
            // we add three points for linear interpolation
        	// TODO: add phase information
            arg.setTime(pulseStart + clusterStart + tau_kl);
            newTxPower->setValue(arg, newTxPower->getValue(arg));
            arg.setTime(pulseStart + clusterStart + tau_kl + IEEE802154A::mandatory_pulse / 2);
            newTxPower->setValue(arg, tapEnergy * IEEE802154A::maxPulse + newTxPower->getValue(arg));
            arg.setTime(pulseStart + clusterStart + tau_kl + IEEE802154A::mandatory_pulse);
            newTxPower->setValue(arg, newTxPower->getValue(arg));

            // Update values for next iteration
            double mix1 = exponential(1 / cfg.lambda_1);
            double mix2 = exponential(1 / cfg.lambda_2);
            tau_kl += mix1 * cfg.Beta + (1 - cfg.Beta) * mix2;
            //fromClusterStart += tau_kl;
            tapEnergy = gamma_l.dbl() * ((1 - cfg.Beta) * cfg.lambda_1 + cfg.Beta * cfg.lambda_2 + 1);
            tapEnergy = Omega_l / tapEnergy;
            simtime_t expArg = -tau_kl / gamma_l ;
            tapEnergy = tapEnergy * exp( expArg.dbl() );
            tapEnergy = sqrt(tapEnergy);
            if (tapEnergy < tapThreshold) {
                moreTaps = false;
            }
        }
        clusterStart += exponential(1 / cfg.Lambda); // sum(x_n) over n=1..cluster
        gamma_l = cfg.k_gamma * clusterStart + cfg.gamma_0;
        Mcluster = normal(0, cfg.sigma_cluster, 2);
        simtime_t expArg = -clusterStart / cfg.Gamma;
        Omega_l = pow(10, (10 * log(exp(expArg.dbl())) + Mcluster) / 10);
        moreTaps = true;
    }
}

double UWBIRIEEE802154APathlossModel::Rayleigh(double param) {
    return weibull(2, sqrt(2) * param, 3);
}

void UWBIRIEEE802154APathlossModel::setMove(const Move newMove) {
    move = newMove;
}
