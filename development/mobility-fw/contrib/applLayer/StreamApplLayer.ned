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
//	file:		$RCSfile: StreamApplLayer.ned,v $
//
//      last modified:	$Date: 2007/01/31 09:45:05 $
//      by:		$Author: tf $
//
//	information:	-
//
//	changelog:   	$Revision: 1.2 $
//			$Log: StreamApplLayer.ned,v $
//			Revision 1.2  2007/01/31 09:45:05  tf
//			- added parameter streamRate to set rate of stream generation
//			
//			Revision 1.1  2007/01/19 09:57:07  tf
//			- added new Applayer for tests
//				- params
//					length of frames
//				- sends a broadcast which is answered by all receiving stations
//				  receiving stations reply and will be scheduled as stream dest.
//			
//


simple StreamApplLayer
    parameters:
	debug : bool, // debug switch
	headerLength: numeric const, // length of the application message header (in bits)
	initBurst: numeric const, // station initiating burst
	streamLength: numeric const, // how many packets to generate
	streamRate: numeric const;
    gates:
	in: lowergateIn, // from network layer
	    lowerControlIn;
	out: lowergateOut; // to network layer
endsimple
