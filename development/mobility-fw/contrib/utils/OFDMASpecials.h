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
 *	file:		$RCSfile: OFDMASpecials.h,v $
 *
 *      last modified:	$Date: 2007/01/28 23:13:00 $
 *      by:		$Author: tf $
 *
 *      informatin:	-
 *
 *	changelog:   	$Revision: 1.5 $
 *			$Log: OFDMASpecials.h,v $
 *			Revision 1.5  2007/01/28 23:13:00  tf
 *			- fixed many bugs
 *			- simulation segfaults for networks with more than 2 hosts in sendDATAframe()
 *			
 *			Revision 1.4  2007/01/25 15:09:37  tf
 *			- restructured signaling information variables and types
 *			
 *			Revision 1.3  2007/01/23 16:22:11  tf
 *			- added struct for uplink signalization
 *			
 *			Revision 1.2  2007/01/22 15:50:10  tf
 *			- added correct types for OFDMA related information on PHY layer
 *			  (testbed transmitts these in ePLCP header)
 *			
 */

#ifndef __OFDMASPECIALS_H__
#define __OFDMASPECIALS_H__

#include <list>
#include <OFDMSnrList.h>
#include <AssignmentSpecials.h>

typedef std::list<int> DestAddrList;

// used for AirFrame80211
struct DynOFDMAState {
	int ModeArray[48];
	int AssigArray[48];
};

#endif
