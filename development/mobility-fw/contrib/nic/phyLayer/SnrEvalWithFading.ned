//
//	copyright:   	(C) 2006 Computer Networks Group (CN) at
//			University of Paderborn, Germany.
//	
//			This program is free software; you can redistribute it
//			and/or modify it under the terms of the GNU General Public
//			License as published by the Free Software Foundation; either
//			version 2 of the License, or (at your option) any later
//			version.
//
//			For further information see file COPYING
//			in the top level directory.
//
//			Based on Mobility Framework 2.0p2 developed at 
//			TKN (TU Berlin) and, ChSim 2.1 developed at CN 
//			(Uni Paderborn).
//
//	file:		$RCSfile: SnrEvalOFDM.ned,v $
//
//      last modified:	$Date: 2007/02/13 12:51:53 $
//      by:		$Author: tf $
//
//	information:	-
//
//	changelog:   	$Revision: 1.4 $
//			$Log: SnrEvalOFDM.ned,v $
//			Revision 1.4  2007/02/13 12:51:53  tf
//			- using parameter globalSpeed for setting speed of terminals
//			
//			Revision 1.3  2007/01/17 16:58:28  tf
//			- added some parameters to network description files
//			
//			Revision 1.2  2007/01/15 16:21:30  tf
//			- added parameters needed for channel sim initialization
//			
//


simple SnrEvalWithFading
    parameters:
	debug : bool, // debug switch
 	// if false, the RSSI will not be published 
        // during the reception of a frame
	publishRSSIAlways : bool, 
	headerLength: numeric const,
        whiteGaussianNoise: numeric const,
	subbands: numeric const,
	pilots: numeric const,
	symbolTime: numeric const,
	transmissionPower: numeric const,
	globalSpeed,
        calculatePathLoss,
        calculateShadowing,
        calculateFading,
        correlated_subbands,
        use_linear_scale,
        lightspeed,
        alpha,
        tenlogk,
        mean,
        std_dev,
        fadingPaths,
        center_frequency,
        freq_spacing,
        delay_rms;
    gates: 
        in: uppergateIn, radioIn;
        out: uppergateOut, upperControlOut;
endsimple
