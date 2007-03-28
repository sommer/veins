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
 *	file:		$RCSfile: DataSink.cc,v $
 *
 *      last modified:	$Date: 2007/03/27 07:03:32 $
 *      by:		$Author: tf $
 *
 *      information:	-
 *
 *	changelog:   	$Revision: 1.22 $
 *			$Log: DataSink.cc,v $
 *			Revision 1.22  2007/03/27 07:03:32  tf
 *			- added additional statistic (total frame rate)
 *			
 *			Revision 1.21  2007/03/21 21:57:24  tf
 *			- added missing whitespace to statistics output
 *			
 *			Revision 1.20  2007/03/21 20:21:07  tf
 *			- fixed minor bug (missing break) in switch block
 *			
 *			Revision 1.19  2007/03/18 13:35:43  tf
 *			- extended data sink with RECORD_FRAGMENTLENGTH
 *			
 *			Revision 1.18  2007/02/21 12:46:53  tf
 *			- destnum output...
 *			
 *			Revision 1.17  2007/02/19 12:42:46  tf
 *			- semicolon removed, compiling now
 *			
 *			Revision 1.16  2007/02/19 12:39:42  tf
 *			- added linebreak
 *			
 *			Revision 1.15  2007/02/19 12:29:28  tf
 *			- added simtime output
 *			
 *			Revision 1.14  2007/02/19 11:09:17  tf
 *			- added whitespace to output
 *			
 *			Revision 1.13  2007/02/17 11:38:07  tf
 *			- overhead rates (time and bit) are calculated now
 *			
 *			Revision 1.12  2007/02/16 16:45:08  tf
 *			- added proper data gathering (with confidence intervals)
 *			- still missing:
 *			  overhead summation in Mac80211a.cc
 *			
 *			Revision 1.11  2007/02/15 18:05:01  tf
 *			- completely redesigned measurement data gathering and output
 *			- fixed some minor bugs
 *			
 *			Revision 1.10  2007/02/15 12:41:05  tf
 *			- some more data output
 *			
 *			Revision 1.9  2007/02/14 15:36:54  tf
 *			- printing sample-size of statistic modules to omnetpp.sca now
 *			
 *			Revision 1.8  2007/02/14 15:31:47  tf
 *			- added output for gnuplot
 *			
 *			Revision 1.7  2007/02/13 13:12:30  tf
 *			*** empty log message ***
 *			
 *			Revision 1.6  2007/02/12 13:46:50  tf
 *			- more data types to catch
 *			
 *			Revision 1.5  2007/02/11 21:26:26  tf
 *			- fixed some data gathering bugs
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

#include "DataSink.h"

Define_Module(DataSink);

void DataSink::initialize(int stage) {
    if (stage==0) {
        firstData=true;
	msgCount=0;
    	simStartTime=0.0;
    	simEndTime=0.0;
    	packetError=0;   		// RECORD_PACKET_ERROR
    	packetErrorBits=0;
    	packetSuccess=0; 		// RECORD_PACKET_SUCCESS
    	packetSuccessBits=0;
    	controlFrameError=0;		// RECORD_CONTROL_FRAME_ERROR
    	controlFrameErrorBits=0;
    	controlFrameSuccess=0;		// RECORD_CONTROL_FRAME_SUCCESS
    	controlFrameSuccessBits=0;
    	dataFrameError=0;		// RECORD_DATA_FRAME_ERROR
    	dataFrameErrorBits=0;
    	dataFrameSuccess=0;		// RECORD_DATA_FRAME_SUCCESS
    	dataFrameSuccessBits=0;
	conffact_const=par("ConfidenceFactor");
        for (int i=0;i<8;i++)
    	    selectedModes[i]=0;		// RECORD_SELECTED_MODE
	EV<<"Stage 0 clear"<<endl;
    }
    else {
	EV<<"Stage 1 clear"<<endl;
    }
}

void DataSink::handleMessage(cMessage* data) {
     SinkMessage* sinkData = static_cast<SinkMessage*>(data);
     msgCount++;
     if (msgCount>10000) {
     	ev<<"Simulated time: "<<simTime();
	msgCount=0;
     }
     if (firstData) {
         simStartTime=simTime();
	 firstData=false;
     }
     EV<<"Recording dataSink msg of kind: "<<typeName(sinkData->kind());
     switch (sinkData->kind()) {
     	case RECORD_PACKET_ERROR:
	    packetError++;
	    packetErrorDev.collect(1);
	    packetErrorBits+=sinkData->getLongValue();
	    break;
	case RECORD_PACKET_SUCCESS:
	    packetSuccess++;
	    packetErrorDev.collect(0);
	    packetSuccessBits+=sinkData->getLongValue();
	    break;
	case RECORD_PACKET_DELAY:
	    packetDelayDev.collect(sinkData->getDoubleValue());
	    break;
	case RECORD_PACKET_RATE:
	    packetRateDev.collect(sinkData->getDoubleValue());
	    break;
	case RECORD_PACKET_NUMFRAG:
	    numFragmentsDev.collect(sinkData->getLongValue());
	    break;
        case RECORD_CONTROL_FRAME_ERROR:
	    controlFrameError++;
	    controlFrameErrorDev.collect(1);
	    controlFrameErrorBits+=sinkData->getLongValue();
	    break;
        case RECORD_CONTROL_FRAME_SUCCESS:
	    controlFrameSuccess++;
	    controlFrameErrorDev.collect(0);
	    controlFrameErrorBits+=sinkData->getLongValue();
	    break;
	case RECORD_DATA_FRAME_ERROR:
	    dataFrameError++;
	    dataFrameErrorDev.collect(1);
	    dataFrameErrorBits+=sinkData->getLongValue();
	    break;
	case RECORD_DATA_FRAME_SUCCESS:
	    dataFrameSuccess++;
	    dataFrameErrorDev.collect(0);
	    dataFrameSuccessBits+=sinkData->getLongValue();
	    break;
	case RECORD_DATA_FRAME_RATE:
	    dataFrameRateDev.collect(sinkData->getLongValue());
	    break;
	case RECORD_SELECTED_MODE:
	    selectedModes[sinkData->getIndex()]+=sinkData->getLongValue();
	    break;
	case RECORD_DESTNUM:
	    destinationNumDev.collect(sinkData->getLongValue());
	    EV<<"destnum is: "<<sinkData->getLongValue();
	    break;
	case RECORD_TIMEOVERHEAD_RATE:
	    timeOverheadRateDev.collect(sinkData->getDoubleValue());
	    break;
	case RECORD_BITOVERHEAD_RATE:
	    bitOverheadRateDev.collect(sinkData->getDoubleValue());
	    break;
	case RECORD_FRAGMENTLENGTH:
	    fragmentLengthDev.collect(sinkData->getLongValue());
	    break;
	case RECORD_TOTAL_FRAME_RATE:
	    totalFrameRateDev.collect(sinkData->getDoubleValue());
	    break;
	default:
	    error("Recording type not implemented: %d",sinkData->kind());
     }
     delete sinkData;
}

void DataSink::finish() {
     simEndTime=simTime();
     //	RECORD_PACKET_ERROR,
     //	RECORD_PACKET_SUCCESS,
     recordScalar("simulated time",simEndTime-simStartTime);
     double packetRateDelta=(conffact_const*packetRateDev.stddev())/sqrt(packetRateDev.samples());
     ev<<"GNUPLOT_PACKETBITRATE "<<packetRateDev.mean()
       <<" "<<packetRateDev.mean()+packetRateDelta
       <<" "<<packetRateDev.mean()-packetRateDelta
       <<" "<<packetRateDev.samples()
       <<" "<<packetSuccessBits/(simEndTime-simStartTime)
       <<endl;
     recordScalar("packet rate",packetRateDev.mean());
     recordScalar("confidence delta",packetRateDelta);
     recordScalar("number of samples",packetRateDev.samples());
     double packetErrorDelta=(conffact_const*packetErrorDev.stddev())/sqrt(packetErrorDev.samples());
     ev<<"GNUPLOT_PACKETERRORRATE "<<packetErrorDev.mean()
       <<" "<<packetErrorDev.mean()+packetErrorDelta
       <<" "<<packetErrorDev.mean()-packetErrorDelta
       <<" "<<packetErrorDev.samples()
       <<endl;
     recordScalar("packet errror rate",packetErrorDev.mean());
     recordScalar("confidence delta",packetErrorDelta);
     recordScalar("number of samples",packetErrorDev.samples());
     // RECORD_CONTROL_FRAME_ERROR,
     // RECORD_CONTROL_FRAME_SUCCESS,
     double controlFrameErrorDelta=(conffact_const*controlFrameErrorDev.stddev())/sqrt(controlFrameErrorDev.samples());
     ev<<"GNUPLOT_CONTROLFRAMEERRORRATE "<<controlFrameErrorDev.mean()<<" "<<controlFrameErrorDev.mean()+controlFrameErrorDelta<<" "<<controlFrameErrorDev.mean()-controlFrameErrorDelta<<" "<<controlFrameErrorDev.samples()<<endl;
     recordScalar("control frame error rate",controlFrameErrorDev.mean());
     recordScalar("confidence delta",controlFrameErrorDelta);
     recordScalar("number of samples",controlFrameErrorDev.samples());
     //	RECORD_DATA_FRAME_ERROR,
     //	RECORD_DATA_FRAME_SUCCESS,
     double dataFrameErrorDelta=(conffact_const*dataFrameErrorDev.stddev())/sqrt(dataFrameErrorDev.samples());
     ev<<"GNUPLOT_DATAFRAMEERRORRATE "<<dataFrameErrorDev.mean()
       <<" "<<dataFrameErrorDev.mean()+dataFrameErrorDelta
       <<" "<<dataFrameErrorDev.mean()-dataFrameErrorDelta
       <<" "<<dataFrameErrorDev.samples()
       <<endl;
     recordScalar("data frame error rate",dataFrameErrorDev.mean());
     recordScalar("confidence delta",dataFrameErrorDelta);
     recordScalar("number of samples",dataFrameErrorDev.samples());
     //	RECORD_PACKET_NUMFRAG,
     double numFragmentsDelta=(conffact_const*numFragmentsDev.stddev())/sqrt(numFragmentsDev.samples());
     ev<<"GNUPLOT_PACKETNUMFRAG "<<numFragmentsDev.mean()
       <<" "<<numFragmentsDev.mean()+numFragmentsDelta
       <<" "<<numFragmentsDev.mean()-numFragmentsDelta
       <<" "<<numFragmentsDev.samples()
       <<endl;
     recordScalar("mean number of fragments per frame",numFragmentsDev.mean());
     recordScalar("confidence delta",numFragmentsDelta);
     recordScalar("number of samples",numFragmentsDev.samples());
     //	RECORD_PACKET_DELAY,
     double packetDelayDelta=(conffact_const*packetDelayDev.stddev())/sqrt(packetDelayDev.samples());
     ev<<"GNUPLOT_PACKETDELAY "<<packetDelayDev.mean()
       <<" "<<packetDelayDev.mean()+packetDelayDelta
       <<" "<<packetDelayDev.mean()-packetDelayDelta
       <<" "<<packetDelayDev.samples()
       <<endl;
     recordScalar("mean delay of packet",packetDelayDev.mean());
     recordScalar("confidence delta",packetDelayDelta);
     recordScalar("number of samples",packetDelayDev.samples());
     //	RECORD_DATA_FRAME_RATE,
     double dataFrameRateDelta=(conffact_const*dataFrameRateDev.stddev())/sqrt(dataFrameRateDev.samples());
     ev<<"GNUPLOT_DATAFRAMERATE "<<dataFrameRateDev.mean()
       <<" "<<dataFrameRateDev.mean()+dataFrameRateDelta
       <<" "<<dataFrameRateDev.mean()-dataFrameRateDelta
       <<" "<<dataFrameRateDev.samples()
       <<endl;
     recordScalar("mean rate of data frames",dataFrameRateDev.mean());
     recordScalar("confidence delta",dataFrameRateDelta);
     recordScalar("number of samples",dataFrameRateDev.samples());
     //	RECORD_SELECTED_MODE,
     ev<<"GNUPLOT_SELECTEDMODES ";
     for (int j=0;j<8;j++) {
     	 ev<<selectedModes[j]<<" ";
	 recordScalar("mode x selected",selectedModes[j]);
     }
     ev<<endl;
     //	RECORD_DESTNUM
     double destinationNumDelta=(conffact_const*destinationNumDev.stddev())/sqrt(destinationNumDev.samples());
     ev<<"GNUPLOT_DESTNUM "<<destinationNumDev.mean()
       <<" "<<destinationNumDev.mean()+destinationNumDelta
       <<" "<<destinationNumDev.mean()-destinationNumDelta
       <<" "<<destinationNumDev.samples()
       <<endl;
     recordScalar("mean rate of data frames",destinationNumDev.mean());
     recordScalar("confidence delta",destinationNumDelta);
     recordScalar("number of samples",destinationNumDev.samples());

     // RECORD_BITOVERHEAD_RATE
     double bitOverheadRateDelta=(conffact_const*bitOverheadRateDev.stddev())/sqrt(bitOverheadRateDev.samples());
     ev<<"GNUPLOT_BITOVERHEAD_RATE "<<bitOverheadRateDev.mean()
       <<" "<<bitOverheadRateDev.mean()+bitOverheadRateDelta
       <<" "<<bitOverheadRateDev.mean()-bitOverheadRateDelta
       <<" "<<bitOverheadRateDev.samples()
       <<endl;
     recordScalar("mean bit overhead per packet",bitOverheadRateDev.mean());
     recordScalar("confidence delta",bitOverheadRateDelta);
     recordScalar("number of samples",bitOverheadRateDev.samples());

     // RECORD_TIMEOVERHEAD_RATE
     double timeOverheadRateDelta=(conffact_const*timeOverheadRateDev.stddev())/sqrt(timeOverheadRateDev.samples());
     ev<<"GNUPLOT_TIMEOVERHEAD_RATE "<<timeOverheadRateDev.mean()
       <<" "<<timeOverheadRateDev.mean()+timeOverheadRateDelta
       <<" "<<timeOverheadRateDev.mean()-timeOverheadRateDelta
       <<" "<<timeOverheadRateDev.samples()
       <<endl;
     recordScalar("mean time overhead per packet",timeOverheadRateDev.mean());
     recordScalar("confidence delta",timeOverheadRateDelta);
     recordScalar("number of samples",timeOverheadRateDev.samples());
     // RECORD_FRAGMENTLENGTH
     double fragmentLengthDelta=(conffact_const*fragmentLengthDev.stddev())/sqrt(fragmentLengthDev.samples());
     ev<<"GNUPLOT_FRAGMENTLENGTH "<<fragmentLengthDev.mean()
       <<" "<<fragmentLengthDev.mean()+fragmentLengthDelta
       <<" "<<fragmentLengthDev.mean()-fragmentLengthDelta
       <<" "<<fragmentLengthDev.samples()
       <<endl;
     recordScalar("mean fragment length",fragmentLengthDev.mean());
     recordScalar("confidence delta",fragmentLengthDelta);
     recordScalar("number of samples",fragmentLengthDev.samples());
     // RECORD_TOTALFRAMERATE
     double totalFrameRateDelta=(conffact_const*totalFrameRateDev.stddev())/sqrt(totalFrameRateDev.samples());
     ev<<"GNUPLOT_TOTALFRAMERATE "<<totalFrameRateDev.mean()
       <<" "<<totalFrameRateDev.mean()+totalFrameRateDelta
       <<" "<<totalFrameRateDev.mean()-totalFrameRateDelta
       <<" "<<totalFrameRateDev.samples()
       <<endl;
     recordScalar("mean total chosen rate",totalFrameRateDev.mean());
     recordScalar("confidence delta",totalFrameRateDelta);
     recordScalar("number of samples",totalFrameRateDev.samples());
}

const char *DataSink::typeName(int state)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (state)
    {
    	CASE(RECORD_PACKET_ERROR);
	CASE(RECORD_PACKET_SUCCESS);
	CASE(RECORD_PACKET_NUMFRAG);
	CASE(RECORD_PACKET_DELAY);
        CASE(RECORD_CONTROL_FRAME_ERROR);
        CASE(RECORD_CONTROL_FRAME_SUCCESS);
	CASE(RECORD_DATA_FRAME_ERROR);
	CASE(RECORD_DATA_FRAME_SUCCESS);
	CASE(RECORD_DATA_FRAME_RATE);
	CASE(RECORD_SELECTED_MODE);
	CASE(RECORD_DESTNUM);
    }
    return s;
#undef CASE
}

