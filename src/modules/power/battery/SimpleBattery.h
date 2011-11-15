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
#ifndef BATTERY_H
#define BATTERY_H

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseBattery.h"

#include "BatteryState.h"
#include "HostState.h"

#include "DeviceEntry.h"

/**
 * @brief A simple linear model of battery consumption.
 *
 * See SimpleBattery.ned for parameters
 *
 * Simple Battery receives DrawMsg's from one or more devices, updates
 * residual capacity (total current * voltage * time), publishes
 * HostState notification on battery depletion, and provides time
 * series and summary information to Battery Stats module.
 *
 * @ingroup power
 * @author Laura Marie Feeney
 * @author Karl Wessel (port for MiXiM)
 */

class MIXIM_API SimpleBattery: public BaseBattery {
public:
	SimpleBattery();
	virtual ~SimpleBattery();
	virtual void initialize(int);
	virtual void handleMessage(cMessage*);
	virtual void handleHostState(const HostState& state);
	virtual void finish();

	/**
	 * @name State-of-charge interface
	 *
	 * @brief Other host modules should use these interfaces to obtain
	 *  the state-of-charge of the battery.  Do NOT use BatteryState
	 *  interfaces, which should be used only by Battery Stats modules.
	 */
	/*@{*/
	/** @brief get voltage (future support for non-voltage regulated h/w */
	double getVoltage() const;
	/** @brief current state of charge of the battery, relative to its
	 * rated nominal capacity [0..1]
	 */
	double estimateResidualRelative() const;
	/** @brief current state of charge of the battery (mW-s) */
	double estimateResidualAbs() const;
	/*@}*/

	/**
	 * @brief Registers a power device by creating a new DeviceEntry for it.
	 */
	virtual int registerDevice(const std::string& name, int numAccounts);

	/**
	 * @brief Draws either a certain amount of energy in mWs or
	 * a defined current in mA over time, depending on passed DrawAmount.
	 */
	virtual void draw(int drainID, DrawAmount& amount, int activity);

	virtual HostState::States getState() const {
		return hostState.get();
	}

protected:

	/** @brief The maximum amount of different power drawing devices.*/
	int numDevices;

	/** @name battery parameters*/
	/*@{*/
	/** @brief Actual capacity.*/
	double capmAh;
	/** @brief Nominal capacity.*/
	double nominalCapmAh;
	/** @brief Voltage*/
	double voltage;
	/*@}*/

	/** @brief Debit battery at least once every resolution seconds.*/
	simtime_t resolution;
	cMessage *timeout;

	/** @name publishing of capacity to BatteryStats via the BB. */
	/*@{*/
	cMessage *publish;
	double publishDelta;
	simtime_t publishTime;

	/** @brief Holds the state of the battery.*/
	BatteryState *batteryState;
	/*@}*/

	/** @name publish of host failure notification
	 * @brief everyone should subscribe to this*/
	/*@{*/
	HostState hostState;
	/*@}*/

	/** @name INTERNAL state*/
	/*@{*/
	double capacity;
	double nominalCapacity;
	double residualCapacity;
	double lastPublishCapacity;
	simtime_t lifetime;
	/*@}*/

	/** @brief Ouput vector tracking the residual capacity.*/
	cOutVector residualVec;

	/** @brief Array of different power consuming devices.*/
	DeviceEntry *devices;
	/** @brief Amount of currently registered devices.*/
	int registeredDevices;

	/** @brief Self message kinds used by the battery.*/
	enum msgType {
		AUTO_UPDATE,
		PUBLISH,
	};

	simtime_t lastUpdateTime;
	/** @brief Pointer to host module */
	cModule* 	host;
	virtual void deductAndCheck();
};

#endif
