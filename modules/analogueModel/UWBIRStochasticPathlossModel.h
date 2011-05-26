/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRPathGainModel.h
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

#ifndef _UWBIRPATHLOSSMODEL_H
#define	_UWBIRPATHLOSSMODEL_H

#include "AnalogueModel.h"
#include "Mapping.h"
#include "Signal_.h"
#include "BaseWorldUtility.h"
#include <math.h>

/**
 * @brief This AnalogueModel models implements the Ghassmezadeh stochastic UWB channel models.
 *
 * Citation of the following publication is appreciated if you use the MiXiM UWB PHY model
 * for a publication of your own.
 * J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
 *
 * For more information, see also:
 *
 * [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no.
 * 1-2 82-112. http://dx.doi.org/10.1177/0037549710377767
 * [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
 * Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010.
 * http://infoscience.epfl.ch/record/147987
 * [3]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
 * for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
 * Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
 * http://portal.acm.org/citation.cfm?id=1537714
 *
 * @ingroup ieee802154a
 * @ingroup analogueModels
 */
class UWBIRStochasticPathlossModel : public AnalogueModel {

public:

    //static const double Gtx = 0.9, Grx = 0.9, ntx = 0.9, nrx = 0.9;
    static const double Gtx = 1, Grx = 1, ntx = 1, nrx = 1;
    double PL0; // 0.000008913; // -50.5 dB   0.000019953
    static const double fc = 4492.8; // mandatory band 3, center frequency, MHz
    static const double d0 = 1;
    double mu_gamma, sigma_gamma; //1.7, 0.3
    double mu_sigma, sigma_sigma;
    double gamma, S, sigma;
    double n1, n2, n3;
    static double n1_limit, n2_limit;

    static const double s_mu = 1.6, s_sigma = 0.5;
    static const double kappa = 1;

    bool isEnabled, shadowing;

    cOutVector distances;
    cOutVector srcPosX, srcPosY, dstPosX, dstPosY;

    int myIndex;
    std::string myName;

    UWBIRStochasticPathlossModel(double _PL0, double _mu_gamma, double _sigma_gamma, double _mu_sigma, double _sigma_sigma, const Move* _move, bool _enabled, bool _shadowing=true) :
    PL0(_PL0), mu_gamma(_mu_gamma), sigma_gamma(_sigma_gamma), mu_sigma(_mu_sigma), sigma_sigma(_sigma_sigma), isEnabled(_enabled), shadowing(_shadowing), frequency("frequency"),
     move(_move) {
    	distances.setName("distances");
    	srcPosX.setName("srcPosX");
    	srcPosY.setName("srcPosY");
    	dstPosX.setName("dstPosX");
    	dstPosY.setName("dstPosY");
    	pathlosses.setName("pathloss");
    }

    void filterSignal(Signal& s);

protected:
    double pathloss_exponent;
    double fading;
    Dimension frequency;
    const Move* move;
    cOutVector pathlosses;  // outputs computed pathlosses. Allows to compute Eb = Epulse*pathloss for Eb/N0 computations. (N0 is the noise sampled by the receiver)

    double getNarrowBandFreeSpacePathloss(double fc, double distance);
    double getGhassemzadehPathloss(double distance);
    double getFDPathloss(double freq, double distance);
    double simtruncnormal(double mean, double stddev, double a, int rng);

};

#endif	/* _UWBIRPATHLOSSMODEL_H */

