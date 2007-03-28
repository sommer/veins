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
//	file:		$RCSfile: DataSink.ned,v $
//
//      last modified:	$Date: 2007/02/05 17:00:28 $
//      by:		$Author: tf $
//
//	information:	-
//
//	changelog:   	$Revision: 1.1 $
//			$Log: DataSink.ned,v $
//			Revision 1.1  2007/02/05 17:00:28  tf
//			- added module for recording statistics of complete network
//			
//
simple DataSink
    parameters:
        debug: bool,
	ConfidenceFactor;
    gates:
        in: toSink[];
endsimple
