/* -*- mode:c++ -*- ********************************************************
 * Energy Framework for Omnet++, version 0.9
 *
 * Author:  Laura Marie Feeney
 *
 * Copyright 2009 Swedish Institute of Computer Science.
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/
#ifndef BATTERYSTATS_H
#define BATTERYSTATS_H


#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseModule.h"
#include "BaseBattery.h"

class DeviceEntry;

/**
 * @brief collects and formates statistical data from the battery
 *
 * See BatteryStats.ned for parameters
 *
 * Generates time series information from BatteryState information
 * published by the battery and by reading its s-o-c information
 * (currently not interesting).  Also generates summary statistics
 * when the Battery module passes its DeviceEntry table at finish().
 * Note: only BatteryStats modules should access BatteryState
 * information, other modules should use Battery's estimateResidual
 * method.
 *
 * @ingroup power
 */
class MIXIM_API BatteryStats : public BaseModule
{

public:
	/** @brief Signal for the BatteryStats.*/
	const static simsignalwrap_t catBatteryStateSignal;

	virtual void initialize( int );
	virtual void handleMessage( cMessage* );
	virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
	virtual void finish();

	/** @brief invoked by the Battery Module's finish()
	 *
	 * (should not rely on BatteryStats::finish() to clean up resources
	 */
	virtual void summary( double, double, simtime_t_cref );
	/** @brief invoked by the Battery Module's at finish()
	 *
	 * (should not rely on BatteryStats::finish() to clean up resources
	 */
	virtual void detail( DeviceEntry *, int);

protected:
	int doDetail;
	/** @brief Enable tracking of output vectors?*/
	int doTimeSeries;

	/** @name Tracked statistic values.*/
	/*@{*/
	cOutVector residualVec;
	cOutVector relativeVec;
	cOutVector estimateVec;
	cOutVector estimateRelVec;
	/*@}*/

	/** @brief Pointer to the battery module.*/
	BaseBattery *battery;
};
#endif
