/*
 *	copyright:   	(C) 2006 Computer Networks Group (CN) at
 *			University of Paderborn, Germany.
 *	
 *			This program is free software; you can redistribute it
 *			and/or modify it under the terms of the GNU General Public
 *			License as published by the Free Software Foundation; either
 *			version 2 of the License, or (at your option) any later
 *			version.
 *
 *			For further information see file COPYING
 *			in the top level directory.
 *
 *			Based on Mobility Framework 2.0p2 developed at 
 *			TKN (TU Berlin) and, ChSim 2.1 developed at CN 
 *			(Uni Paderborn).
 *
 *	file:		$RCSfile: OFDMChannelSim.h,v $
 *
 *      last modified:	$Date: 2007/01/15 16:32:24 $
 *      by:		$Author: tf $
 *
 *      information:	class representing channel state simulation module 
 *      		of chsim 2.1:
 *      		 - constructor realizes initialization like initialize()
 *      		 - destructor realizes freeing memory like finish()
 *
 *	changelog:   	$Revision: 1.1 $
 *			$Log: OFDMChannelSim.h,v $
 *			Revision 1.1  2007/01/15 16:32:24  tf
 *			- added OFDMChannelSim class derived from chsim for channel state simulation
 *			
 */
#ifndef _OFDMCHANNELSIM_H_
#define _OFDMCHANNELSIM_H_

#include <map>

struct ChannelState {
        double speed;
        double distance;
        double loss[48];
	double rcvdPower[48];
	double rcvdPowerMin;
	double rcvdPowerMax;
	double snr[48];
};


class OFDMChannelSim {
      public:
    OFDMChannelSim();
    OFDMChannelSim(double pLIGHTSPEED, double pTENLOGK, double pALPHA, double pMEAN, double pSTD_DEV, double pDELAY_RMS, double pFREQUENCY_SPACING, int pFADING_PATHS, double pCENTER_FREQUENCY, int pSUBBANDS, int pCALC_PL, int pCALC_SHADOW, int pCALC_FADING, int pCORRELATED_SUBBANDS);
    ChannelState calculateLoss(double distance, double speed);
    ~OFDMChannelSim();

      protected:
    /** @brief calculates path loss for given distance */
    double calculatePathLoss(double distance);

    /** @brief calculates shadowing */
    double calculateShadowing();

    /** @brief calculates fading for given subband sb */
    double calculateFading(int sb, double frequency, double mobile_speed);
    void initFading();

      protected:
    double LIGHTSPEED;

    // for path loss
    double TENLOGK;
    double ALPHA;

    // for shading loss
    double MEAN;
    double STD_DEV;

    // for fading loss
    double DELAY_RMS;               // Mean Delay Spread
    double FREQUENCY_SPACING;       // frequency sample spacing
    int FADING_PATHS;               // number of different simulated fading paths
    double CENTER_FREQUENCY;           // center frequency
    int SUBBANDS;

    // describtion of the different fading paths
    double **angle_of_arrival;
    double **delay;

    int CALCULATE_PATH_LOSS;
    int CALCULATE_SHADOWING;
    int CALCULATE_FADING;

    int CORRELATED_SUBBANDS;
    ChannelState loss;
};

/** @brief typedef for channel map, holding channel state simulators taken from
chsim */
typedef std::map<int,OFDMChannelSim*> ChannelMap;

#endif
