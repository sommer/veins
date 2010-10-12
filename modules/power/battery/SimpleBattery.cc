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
/*
 * A simple linear model of battery consumption.  Simple Battery
 * receives DrawMsg's from one or more devices, updates residual
 * capacity (total current * voltage * time), publishes HostState
 * notification on battery depletion, and provides time series and
 * summary information to Battery Stats module.
 */

#include "FWMath.h"
#include "SimpleBattery.h"
#include "BatteryStats.h"

Define_Module(SimpleBattery)
;

void SimpleBattery::initialize(int stage) {
	BaseBattery::initialize(stage);

	if (stage == 0) {

		voltage = par("voltage");
		nominalCapmAh = par("nominal");
		if (nominalCapmAh <= 0) {
			error("invalid nominal capacity value");
		}
		capmAh = par("capacity");

		// Publish capacity to BatteryStats every publishTime (if > 0) and
		// whenever capacity has changed by publishDelta (if < 100%).
		publishTime = 0;
		publishTime = par("publishTime");
		if (publishTime > 0) {
			publish = new cMessage("publish", PUBLISH);
			publish->setSchedulingPriority(2000);
			scheduleAt(simTime() + publishTime, publish);
		}
		publishDelta = 1;
		publishDelta = par("publishDelta");
		if (publishDelta < 0 || publishDelta > 1) {
			error("invalid publishDelta value");
		}

		resolution = par("resolution");

		EV<< "capacity = " << capmAh << "mA-h (nominal = " << nominalCapmAh <<
		") at " << voltage << "V" << endl;
		EV << "publishDelta = " << publishDelta * 100 << "%, publishTime = "
		<< publishTime << "s, resolution = " << resolution << "sec"
		<< endl;

		capacity = capmAh * 60 * 60 * voltage; // use mW-sec internally
		nominalCapacity = nominalCapmAh * 60 * 60 * voltage;
		batteryState = new BatteryState(nominalCapacity);

		residualCapacity = lastPublishCapacity = capacity;

		lifetime = -1; // -1 means not dead

		// DISable by default (use BatteryStats for data collection)
		residualVec.disable();

		residualVec.setName("residualCapacity");
		residualVec.record(residualCapacity);

		timeout = new cMessage("auto-update", AUTO_UPDATE);
		timeout->setSchedulingPriority(500);
		scheduleAt(simTime() + resolution, timeout);

		// publish battery depletion on hostStateCat
		scopeHost = (this->findHost())->getId();
		hostStateCat = utility->getCategory(&hostState);

		// periodically publish residual capacity on batteryCat
		if (publishDelta < 1 || publishTime> 0)
		batteryCat = utility->getCategory(batteryState);

		numDevices = hasPar("numDevices") ? par("numDevices") : 0;
		if (numDevices == 0) {
			EV << "Warning: no devices attached to battery\n";
		}
		registeredDevices = 0;

		devices = new DeviceEntry[numDevices];
		lastUpdateTime = simTime();
	}

	if (stage == 1) {
		hostState.set(HostState::ACTIVE);
		utility->publishBBItem(hostStateCat, &hostState, scopeHost);

		if (publishDelta < 1 || publishTime> 0 ) {
			batteryState->set(residualCapacity);
			utility->publishBBItem(batteryCat, batteryState, scopeHost);
		}
	}
}

int SimpleBattery::registerDevice(const std::string& name, int numAccts)
{
	int deviceID = registeredDevices++;

	if(registeredDevices > numDevices) {
		error("To much devices registered with battery. Please adjust the Batteries numDevices parameter!");
	}

	if (!devices[deviceID].name.empty()) {
		error("device already registered!");
	}

	devices[deviceID].name = name;

	if (numAccts < 1) {
		error("number of activities must be at least 1");
	}
	devices[deviceID].numAccts = numAccts;

	double *accts = new double[numAccts];
	for (int i = 0; i < numAccts; i++) {
		accts[i] = 0.0;
	}
	devices[deviceID].accts = accts;

	simtime_t *times = new simtime_t[numAccts];
	for (int i = 0; i < numAccts; i++) {
		times[i] = 0.0;
	}
	devices[deviceID].times = times;

	EV<< "initialized device " << devices[deviceID].name << " as device "
	<< deviceID << " with " << devices[deviceID].numAccts <<
	" accounts" << endl;

	return deviceID;
}

void SimpleBattery::draw(int deviceID, DrawAmount& amount, int activity)
{
	if(deviceID < 0 || deviceID > registeredDevices) {
		error("Unknown device ID!");
	}

	if (amount.getType() == DrawAmount::CURRENT) {

		if (devices[deviceID].name.empty()) {
			error("drawMsg from unregistered device");
		}

		double current = amount.getValue();
		if (activity < 0 && current != 0)
			error("invalid CURRENT message");

		EV << simTime() << " device " << deviceID <<
		" (" << devices[deviceID].name << ") draw current " << current <<
		"mA, activity = " << activity << endl;

		// update the residual capacity (finish previous current draw)
		deductAndCheck();

		// set the new current draw in the device vector
		devices[deviceID].draw = current;
		devices[deviceID].currentActivity = activity;
	}

	else if (amount.getType() == DrawAmount::ENERGY) {

		if (devices[deviceID].name.empty()) {
			error("drawMsg from unregistered device");
		}
		double energy = amount.getValue();
		if (!(activity >=0 && activity < devices[deviceID].numAccts)) {
			error("invalid activity specified");
		}

		EV << simTime() << " device " << deviceID <<
		" (" << devices[deviceID].name << ") deduct " << energy <<
		" mW-s, activity = " << activity << endl;

		// deduct a fixed energy cost
		devices[deviceID].accts[activity] += energy;
		residualCapacity -= energy;

		// update the residual capacity (ongoing current draw), mostly
		// to check whether to publish (or perish)
		deductAndCheck();
	}
	else {
		error("Unknown power type!");
	}
}

void SimpleBattery::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {

		switch (msg->getKind()) {
		case AUTO_UPDATE:
			// update the residual capacity (ongoing current draw)
			scheduleAt(simTime() + resolution, timeout);
			deductAndCheck();
			break;

		case PUBLISH:
			// publish the state to the BatteryStats module
			utility->publishBBItem(batteryCat, batteryState, scopeHost);
			lastPublishCapacity = residualCapacity;

			scheduleAt(simTime() + publishTime, publish);
			break;

		default:
			error("battery receives mysterious timeout");
			break;
		}
	} else {
		error("unexpected message");
		delete msg;
	}
}

void SimpleBattery::handleHostState(const HostState& state)
{
	//does nothing yet
}

void SimpleBattery::deductAndCheck() {
	// already depleted, devices should have stopped sending drawMsg,
	// but we catch any leftover messages in queue
	if (batteryState->getAbs() == 0) {
		return;
	}

	simtime_t now = simTime();

	// If device[i] has never drawn current (e.g. because the device
	// hasn't been used yet or only uses ENERGY) the currentActivity is
	// still -1.  If the device is not drawing current at the moment,
	// draw has been reset to 0, so energy is also 0.  (It might perhaps
	// be wise to guard more carefully against fp issues later.)

	for (int i = 0; i < numDevices; i++) {
		int currentActivity = devices[i].currentActivity;
		if (currentActivity > -1) {
			double energy = devices[i].draw * voltage * (now - lastUpdateTime).dbl();
			if (energy > 0) {
				devices[i].accts[currentActivity] += energy;
				devices[i].times[currentActivity] += (now - lastUpdateTime);
				residualCapacity -= energy;
			}
		}
	}

	lastUpdateTime = now;

	EV<< simTime() << ": residual capacity = "
	<< residualCapacity << endl;

	// battery is depleted
	if (residualCapacity <= 0.0 ) {

		EV << "battery depleted at t = " << now << "s" << endl;

		lifetime = now;

		// announce hostState
		hostState.set(HostState::FAILED);
		utility->publishBBItem(hostStateCat, &hostState, scopeHost);

		// final battery level announcement
		if (publishDelta < 1 || publishTime> 0) {
			batteryState->set(0);
			utility->publishBBItem(batteryCat, batteryState, scopeHost);
			// cancelEvent(publish);
		}

		// no more resolution-based timeouts
		cancelEvent(timeout);
	}

	// battery is not depleted, continue
	else {

		batteryState->set(residualCapacity);

		// publish the battery capacity if it changed by more than delta
		if ((lastPublishCapacity - residualCapacity)/capacity >= publishDelta) {
			utility->publishBBItem(batteryCat, batteryState, scopeHost);
			lastPublishCapacity = residualCapacity;
		}
	}
	residualVec.record(residualCapacity);
}

	// the three functions below should be supported in all battery
	// modules.  in SimpleBattery, they're trivial.  a more accurate model
	// would require substantially more complex functionality here

	// Return open circuit battery voltage.  in SimpleBattery, the open
	// circuit voltage is fixed

double SimpleBattery::getVoltage() {
	Enter_Method_Silent();
	return voltage;
}

// Return host's own state-of-charge estimate of residual capacity
// (absolute value in mW-s).  In SimpleBattery, the state-of-charge
// estimate is the same as the residual capacity.

// Use getAbs() rather than residualCapacity, which can become negative

double SimpleBattery::estimateResidualAbs() {
	Enter_Method_Silent();
	double value = batteryState->getAbs();
	return value;
}

// Return host's own estimate of residual capacity (relative to
// nominal capacity).

double SimpleBattery::estimateResidualRelative() {
	Enter_Method_Silent();
	double value = batteryState->getAbs();
	return value / nominalCapacity;
}

void SimpleBattery::finish() {
	// do a final update of battery capacity
	deductAndCheck();

	for (int i = 0; i < numDevices; i++) {

		double total = 0;
		for (int j = 0; j < devices[i].numAccts; j++) {
			total += devices[i].accts[j];
		}
		EV<< "device " << i << " (" << devices[i].name << ") consumed "
		<< total << " mW-s at" << endl;

		for (int j = 0; j < devices[i].numAccts; j++) {
			EV << "activity " << j << ": " << devices[i].accts[j] << " mWs and "
			<< devices[i].times[j] << "sec" << endl;
		}

		// check that total time in all states matches simulation time
		simtime_t sum = 0;
		for (int j = 0; j < devices[i].numAccts; j++)
		sum += devices[i].times[j];
		if (FWMath::round(sum.dbl() * 1000000) - FWMath::round(simTime().dbl() * 1000000) != 0)
		{
			EV << "WARNING: device " << devices[i].name << " total time " << sum
			<< " != sim time " << simTime() << " (may not matter)" << endl;
		}
	}

	cModule *statsModule = getParentModule()->getSubmodule("batteryStats");
	if (statsModule) {
		BatteryStats *batteryStats = check_and_cast<BatteryStats *>(statsModule);
		batteryStats->summary(capacity, residualCapacity, lifetime);
		batteryStats->detail(devices, numDevices);
	}
	else {
		error("No batteryStats module found, please check your Host.ned");
	}

	BaseBattery::finish();
}

SimpleBattery::~SimpleBattery() {
	cancelAndDelete(timeout);
	if (publishTime> 0)
		cancelAndDelete(publish);

	delete [] devices; // it's ok, batteryStats is done with device info

	delete batteryState;
}

