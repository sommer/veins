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
 *	file:		$RCSfile: OFDMChannelSim.cc,v $
 *
 *      last modified:	$Date: 2007/01/31 14:48:59 $
 *      by:		$Author: tf $
 *
 *      information:	class representing channel state simulation module 
 *      		of chsim 2.1:
 *      		 - constructor realizes initialization like initialize()
 *      		 - destructor realizes freeing memory like finish()
 *
 *	changelog:   	$Revision: 1.2 $
 *			$Log: OFDMChannelSim.cc,v $
 *			Revision 1.2  2007/01/31 14:48:59  tf
 *			- OFDMChannelSim uses linear scales now
 *			
 *			Revision 1.1  2007/01/15 16:32:24  tf
 *			- added OFDMChannelSim class derived from chsim for channel state simulation
 *			
 */

#include "SnrEvalOFDM.h"
#include "OFDMChannelSim.h"

OFDMChannelSim::OFDMChannelSim() 
{
	OFDMChannelSim(299792458.,0,2.4,0,5.8,0.00000015,312500,20,5200000000.,48,1,0,1,1);
}

OFDMChannelSim::OFDMChannelSim(double pLIGHTSPEED, double pTENLOGK, double pALPHA, double pMEAN, double pSTD_DEV, double pDELAY_RMS, double pFREQUENCY_SPACING, int pFADING_PATHS, double pCENTER_FREQUENCY, int pSUBBANDS, int pCALC_PL, int pCALC_SHADOW, int pCALC_FADING, int pCORRELATED_SUBBANDS)
{
    LIGHTSPEED = pLIGHTSPEED;
    TENLOGK = pTENLOGK;
    ALPHA = pALPHA;

    MEAN = pMEAN;
    STD_DEV = pSTD_DEV;   // 250 = 1 DL, 250*500=125000 = 1 second

    FADING_PATHS = pFADING_PATHS;
    CENTER_FREQUENCY = pCENTER_FREQUENCY;
    DELAY_RMS = pDELAY_RMS;
    FREQUENCY_SPACING = pFREQUENCY_SPACING;
    SUBBANDS = pSUBBANDS;
    initFading(); // initiate fading paths

    CALCULATE_PATH_LOSS = pCALC_PL;
    CALCULATE_SHADOWING = pCALC_SHADOW;
    CALCULATE_FADING = pCALC_FADING;

    CORRELATED_SUBBANDS = pCORRELATED_SUBBANDS;
}

/*// implementation of the module class:
void channelStateCalc::handleMessage(cMessage * msg)
{
    channelStateMsg *channelStatemsg = (channelStateMsg *) msg;

    double speed = channelStatemsg->getSpeed();
    double distance = channelStatemsg->getDistance();
    double pathLoss = 0;
    double shadowLoss = 0;
    double fadingLoss = 0;

    if(CALCULATE_PATH_LOSS) {
        pathLoss = calculatePathLoss(distance);
    }
    if(CALCULATE_SHADOWING){
        shadowLoss = calculateShadowing();
    }
    for (int sb = 0; sb < SUBBANDS; sb++) { //over subbands
        if(CALCULATE_FADING) {
            // calculate center frequency of each sub band
            double frequency = CENTER_FREQUENCY - SUBBANDS * FREQUENCY_SPACING / 2 + sb * FREQUENCY_SPACING + FREQUENCY_SPACING / 2;    // works correct for odd and even subbands
            fadingLoss = calculateFading(sb, frequency, speed); // calculate frequency dependend fading for each subband
        }
        double loss = 0 + pathLoss + shadowLoss + fadingLoss;
        if (USE_LINEAR_SCALE) loss = pow(10, (loss/10));    // convert to linear scale if selected
        channelStatemsg->setChannelState(sb, loss);
    }
    send(channelStatemsg, "out");
}*/

// destructor
OFDMChannelSim::~OFDMChannelSim()
{
    for (int s = 0; s < SUBBANDS; s++)
        delete []angle_of_arrival[s];
    delete []angle_of_arrival;

    for (int s = 0; s < SUBBANDS; s++)
        delete []delay[s];
    delete []delay;
}

/* Determined by the distance and medium-specific parameters the
 * path loss from the terminal to the base station is calculated
 */
double OFDMChannelSim::calculatePathLoss(double distance)
{
    return TENLOGK - 10 * ALPHA * log10(distance);
}

// calculates shadowling loss based on a normal gaussian function
double OFDMChannelSim::calculateShadowing()
{
    return -1 * normal(MEAN, STD_DEV);
}

void OFDMChannelSim::initFading()
{
    angle_of_arrival = new double *[SUBBANDS];
    for (int s = 0; s < SUBBANDS; s++)
        angle_of_arrival[s]  = new double [FADING_PATHS];

    delay = new double *[SUBBANDS];
    for (int s = 0; s < SUBBANDS; s++)
        delay[s]  = new double [FADING_PATHS];

    for (int s = 0; s < SUBBANDS; s++) {
        for (int i = 0; i < FADING_PATHS; ++i) {
            //angle of arrival on path i, used for doppler_shift calculation
            //might be also subband independent, i.e. per s
            angle_of_arrival[s][i] = cos(uniform(0,M_PI));
            //delay on path i
            //might be also subband independent, i.e. per s
            delay[s][i] = (double)exponential(DELAY_RMS);
        }
    }
}

/*
 * Jakes-like  method, Frequency in Megahertz
 * With OFDM subbands (numbered from low to high f):
 * frequency = CENTER_FREQUENCY - SUBBANDS * FREQUENCY_SPACING / 2 + sb * FREQUENCY_SPACING + FREQUENCY_SPACING / 2
 */
double OFDMChannelSim::calculateFading(int sb, double frequency, double mobile_speed)
{
  double phi_d = 0;
  double phi_i = 0;
  double phi = 0;
  double phi_sum = 0;

  double re_h = 0;
  double im_h = 0;

  if (CORRELATED_SUBBANDS) sb = 0;  // sub bands are correleted -> same fading 'profile'

  double doppler_shift = mobile_speed * frequency / LIGHTSPEED ;
//  ev << " Maximum Doppler Frequency is: "<< doppler_shift << " at speed " << mobile_speed << endl;

  for(int i = 0; i < FADING_PATHS; i++)
  {
    //some math for complex numbers:
    //z = a + ib        cartesian form
    //z = p * e ^ i(phi)    polar form
    //a = p * cos(phi)
    //b = p * sin(phi)
    //z1 * z2 = p1 * p2 * e ^ i(phi1 + phi2)

    phi_d = angle_of_arrival[sb][i] * doppler_shift;    // phase shift due to doppler => t-selectivity
    phi_i = delay[sb][i] * frequency;                   // phase shift due to delay spread => f-selectivity

    phi = 2.00 * M_PI * (phi_d * simulation.simTime() - phi_i);    // calculate resulting phase due to t-and f-selective fading

    //one ring model/Clarke's model plus f-selectivity according to Cavers:
    //due to isotropic antenna gain pattern on all paths only a^2 can be received on all paths
    //since we are interested in attenuation a:=1, attenuation per path is then:
    double attenuation = (1.00/sqrt(FADING_PATHS));

    //convert to cartesian form and aggregate {Re, Im} over all fading paths
    re_h = re_h + attenuation * cos(phi);
    im_h = im_h - attenuation * sin(phi);
  }
  //output: |H_f|^2= absolute channel impulse response due to fading in dB
  //note that this may be >0dB due to constructive interference
  return 10 * log10(re_h * re_h + im_h * im_h);
}

ChannelState OFDMChannelSim::calculateLoss(double distance, double speed) 
{
    ChannelState cs;
    double pathLoss = 0;
    double shadowLoss = 0;
    double fadingLoss = 0;

    cs.speed = speed;
    cs.distance = distance;

    if (CALCULATE_PATH_LOSS) {
        pathLoss = calculatePathLoss(distance);
    }
    if (CALCULATE_SHADOWING) {
        shadowLoss = calculateShadowing();
    }
    for (int sb = 0; sb < SUBBANDS; sb++) { //over subbands
        if (CALCULATE_FADING) {
            // calculate center frequency of each sub band
            double frequency = CENTER_FREQUENCY - SUBBANDS * FREQUENCY_SPACING / 2 + sb * FREQUENCY_SPACING + FREQUENCY_SPACING / 2;    // works correct for odd and even subbands
            fadingLoss = calculateFading(sb, frequency, speed); // calculate frequency dependend fading for each subband
        }
        cs.loss[sb] = 0 + pathLoss + shadowLoss + fadingLoss;
	cs.loss[sb] = pow(10, (cs.loss[sb]/10));
    }
    return cs; 
}


/*
  {
    phi_sum = phi_sum + phi;    // sum over all fading paths
  }
  double gain_re = 1.00/sqrt(FADING_PATHS) * cos(phi_sum); // convert from polar to cartesian form because we want the absolute value
  double gain_im = 1.00/sqrt(FADING_PATHS) * sin(phi_sum); // convert from polar to cartesian form because we want the absolute value
  double gain = (10 * log10(gain_re * gain_re + gain_im * gain_im));    //output attenuation^2 (power) in dB
  return gain;
}
*/
