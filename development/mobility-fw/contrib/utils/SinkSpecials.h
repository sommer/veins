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
 *	file:		$RCSfile: SinkSpecials.h,v $
 *
 *      last modified:	$Date: 2007/03/27 07:03:32 $
 *      by:		$Author: tf $
 *
 *      information:	-
 *
 *	changelog:   	$Revision: 1.11 $
 *			$Log: SinkSpecials.h,v $
 *			Revision 1.11  2007/03/27 07:03:32  tf
 *			- added additional statistic (total frame rate)
 *			
 *			Revision 1.10  2007/03/18 13:35:43  tf
 *			- extended data sink with RECORD_FRAGMENTLENGTH
 *			
 *			Revision 1.9  2007/02/17 11:38:07  tf
 *			- overhead rates (time and bit) are calculated now
 *			
 *			Revision 1.8  2007/02/16 16:45:08  tf
 *			- added proper data gathering (with confidence intervals)
 *			- still missing:
 *			  overhead summation in Mac80211a.cc
 *			
 *			Revision 1.7  2007/02/15 18:05:01  tf
 *			- completely redesigned measurement data gathering and output
 *			- fixed some minor bugs
 *			
 *			Revision 1.6  2007/02/15 12:41:05  tf
 *			- some more data output
 *			
 *			Revision 1.5  2007/02/12 13:46:50  tf
 *			- more data types to catch
 *			
 *			Revision 1.4  2007/02/09 13:59:30  tf
 *			- added gathering of additional data
 *			
 *			Revision 1.3  2007/02/09 09:36:28  tf
 *			- changed recording types
 *			
 *			Revision 1.2  2007/02/06 20:15:40  tf
 *			- changed sinkmessage kinds
 *			
 *			Revision 1.1  2007/02/05 17:00:28  tf
 *			- added module for recording statistics of complete network
 *			
 */

#ifndef _SINK_SPECIALS____H
#define _SINK_SPECIALS____H

enum RecordTypes {
	RECORD_PACKET_ERROR,
	RECORD_PACKET_SUCCESS,
	RECORD_PACKET_NUMFRAG,
	RECORD_PACKET_DELAY,
	RECORD_PACKET_RATE,
	RECORD_OVERHEAD_RATE,
        RECORD_CONTROL_FRAME_ERROR,
        RECORD_CONTROL_FRAME_SUCCESS,
	RECORD_DATA_FRAME_ERROR,
	RECORD_DATA_FRAME_SUCCESS,
	RECORD_DATA_FRAME_RATE,
	RECORD_TOTAL_FRAME_RATE,
	RECORD_SELECTED_MODE,
	RECORD_DESTNUM,
	RECORD_TIMEOVERHEAD_RATE,
	RECORD_BITOVERHEAD_RATE,
	RECORD_FRAGMENTLENGTH
};
#endif
