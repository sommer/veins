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
 *	file:		$RCSfile: DataSink.h,v $
 *
 *      last modified:	$Date: 2007/03/27 07:03:32 $
 *      by:		$Author: tf $
 *
 *      information:	-
 *
 *	changelog:   	$Revision: 1.12 $
 *			$Log: DataSink.h,v $
 *			Revision 1.12  2007/03/27 07:03:32  tf
 *			- added additional statistic (total frame rate)
 *			
 *			Revision 1.11  2007/03/18 13:35:43  tf
 *			- extended data sink with RECORD_FRAGMENTLENGTH
 *			
 *			Revision 1.10  2007/02/19 12:29:28  tf
 *			- added simtime output
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
 *			Revision 1.2  2007/02/06 20:14:46  tf
 *			- gathering more data
 *			- added gnuplot compatible output
 *			
 *			Revision 1.1  2007/02/05 17:00:28  tf
 *			- added module for recording statistics of complete network
 *			
 */

#ifndef _DATASINK_H__
#define _DATASINK_H__

#include <omnetpp.h>
#include "SinkSpecials.h"
#include "BasicModule.h"
#include "SinkMessage_m.h"

// class DataSink : public BasicModule { // tries to get host address, but has none
class DataSink : public BasicModule {
	public:
    virtual void initialize(int);
    virtual void handleMessage(cMessage*);
    const char* typeName(int);
    virtual void finish();
    /**
     * @brief Function to get the logging name of the host
     *
     * The logging name is the ned module name of the host (unless the
     * host ned variable loggingName is specified). It can be used for
     * logging messages to simplify debugging in TKEnv.
     */
    virtual std::string logName(void) const {
        std::ostringstream ost;
        ost << "sink";
        return ost.str();
    };    
	protected:
	private:
    bool firstData;
    double simStartTime;
    double simEndTime;
    cStdDev fragmentLengthDev;		// RECORD_FRAGMENTLENGTH
    long msgCount;
    long packetError;   		// RECORD_PACKET_ERROR
    long packetErrorBits;
    cStdDev packetErrorDev;
    long packetSuccess; 		// RECORD_PACKET_SUCCESS
    long packetSuccessBits;
    cStdDev packetRateDev;
    cStdDev packetDelayDev; 		// RECORD_PACKET_DELAY
    cStdDev numFragmentsDev; 		// RECORD_PACKET_NUMFRAG
    long controlFrameError;		// RECORD_CONTROL_FRAME_ERROR
    long controlFrameErrorBits;
    cStdDev controlFrameErrorDev;
    cStdDev controlFrameErrorBitsDev;
    long controlFrameSuccess;		// RECORD_CONTROL_FRAME_SUCCESS
    long controlFrameSuccessBits;
    long dataFrameError;		// RECORD_DATA_FRAME_ERROR
    cStdDev dataFrameErrorDev;
    cStdDev dataFrameErrorBitsDev;
    long dataFrameErrorBits;
    long dataFrameSuccess;		// RECORD_DATA_FRAME_SUCCESS
    long dataFrameSuccessBits;
    cStdDev dataFrameRateDev;		// RECORD_DATA_FRAME_RATE
    long selectedModes[8];		// RECORD_SELECTED_MODE
    cStdDev destinationNumDev;		// RECORD_DESTNUM
    double conffact_const;
    cStdDev timeOverheadRateDev;	// RECORD_TIMEOVERHEAD_RATE
    cStdDev bitOverheadRateDev;		// RECORD_BITOVERHEAD_RATE
    cStdDev totalFrameRateDev;		// RECORD_TOTAL_FRAME_RATE
};

#endif
