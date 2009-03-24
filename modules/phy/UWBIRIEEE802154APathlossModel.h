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

#ifndef _UWBIRIEEE802154APATHLOSSMODEL_H
#define	_UWBIRIEEE802154APATHLOSSMODEL_H

#include "AnalogueModel.h"
#include "MappingUtils.h"
#include "Signal_.h"
#include "BaseWorldUtility.h"
#include "IEEE802154A.h"
//#include "AlohaMacLayer.h"
#include "Move.h"
#include "SimpleTimeConstMapping.h"
#include <math.h>


/** @brief This class implements the IEEE 802.15.4A Channel Model[1] in the MiXiM
 *  omnet++ modeling framework.
 *  [1] "IEEE 802.15.4a channel model - final report", 2005, Andreas F. Molisch,
 *  Kannan Balakrishnan, Dajana Cassioli, Chia-Chin Chong, Shahriar Emami,
 *  Andrew Fort, Johan Karedal, Juergen Kunisch, Hans Schantz, Ulrich Schuster, Kai Siwiak
 *
 */


class UWBIRIEEE802154APathlossModel : public AnalogueModel {

public:

	/*
	 * @brief Default constructor.
	 */
    UWBIRIEEE802154APathlossModel(int _channelModel, double _threshold):
    	channelModel(_channelModel), tapThreshold(_threshold), move() {
    	// Check that this model is supported
    	assert(implemented_CMs[channelModel]);
    	// load the model parameters
    	cfg = CMconfigs[channelModel];
    }

    /*
     * @brief Applies the model to an incoming signal.
     */
    void filterSignal(Signal& s);

    /*
     * @brief This class needs to know the position of the host it is attached to.
     * This function is called by the UWBIRPhyLayer to provide this information.
     */
    void setMove(const Move newMove);

    /*@brief Utility function to use a Rayleigh random variable
     *
     */
    double Rayleigh(double param);

    /* @brief Lists implemented channel models.
     * */
    static const bool implemented_CMs[];

    struct CMconfig {
    	double PL0; 				// pathloss at 1 m distance (*not* in dB)
    	double n;					// pathloss exponent
    	double sigma_s;				// shadowing standard deviation (unused)
    	double Aant;				// antenna loss (always 3 dB)
    	double kappa;				// frequency dependence of the pathloss
		double Lmean;				// mean number of clusters
    	double Lambda;					// inter-cluster arrival rate (clusters *per second*)
		// ray arrival rates (mixed Poisson model parameters, in seconds)
    	double lambda_1, lambda_2, Beta;
    	double Gamma;				// inter-cluster decay constant
    	double k_gamma, gamma_0; 	// intra-cluster decay time constant parameters
    	double sigma_cluster;		// cluster shadowing variance (*not* in dB)
    	double m_0, k_m;			// Nakagami m factor mean
    	double var_m_0, var_k_m;	// Nakagami m factor variance
    	double strong_m_0;			// Nakagami m factor for strong components
    	// parameters for alternative PDP shape
    	double gamma_rise; double gamma_1; double xi;
    };

    /* @brief Known models configuration
     **/
    static const CMconfig CMconfigs[];


protected:

	/** @brief selected channel model
	 **/
	int channelModel;

	/* @brief Selected configuration
	     **/
	    CMconfig cfg;

    // configure cluster threshold
    double tapThreshold; // Only generates taps with at most 10 dB attenuation

    // channel statistical characterization parameters
    // (should be xml-ized)
    // First environment: Residential LOS
    static const double PL0 = 0.000040738; // -43.9 dB
    static const double pathloss_exponent = 1.79;
    static const double meanL = 3;
    static const double Lambda = 0.047E9;
    static const double lambda1 = 1.54E9;
    static const double lambda2 = 0.15E9;
    static const double Beta = 0.095;
    static const double Gamma = 22.61E-9;
    static const double k_gamma = 0;
    static const double gamma_0 = 12.53 * 0.001 * 0.001 * 0.01;
    static const double sigma_cluster = 1.883649089; // 2.75 dB

    static const double fc = 4.492E9;


    Move move;
    TimeMapping<Linear>* newTxPower;
    Mapping* txPower;
    Argument arg;
    // number of clusters
    double L;
    // start time of cluster number "cluster"
    simtime_t clusterStart;
    simtime_t gamma_l;
    double Mcluster;
    // cluster integrated energy
    double Omega_l;

    /*
     * Generates taps for the considered pulse, with the current channel parameters
     */
    void addEchoes(simtime_t pulseStart);
};

#endif	/* _UWBIRIEEE802154APATHLOSSMODEL_H */

