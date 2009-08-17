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
 * A trivial device module that has a fixed duty cycle and period, but
 * is not connected to the operation of the host.  Used for testing,
 * can also be used as e.g. a prototype sensor device.
 *
 * Alternates between ON (draws current CURRENT) and OFF state.
 * Switching from OFF to ON (draws wakeup ENERGY).  Subscribed to
 * HostState notification and ends the ON/OFF cycle on Host Failure.
 * See the file DeviceDutySimple.ned for parameters
 *
 ***************************************************************************/

#include "DeviceDutySimple.h"

Define_Module(DeviceDutySimple)
;

void DeviceDutySimple::initialize(int stage) {
	BaseModule::initialize(stage);

	if (stage == 0) {

		dutyCycle = par("dutyCycle");
		if (dutyCycle < 0 || dutyCycle > 1.0) {
			error("invalid dutyCycle");
		}
		period = par("period");

		EV<< "dutyCycle = " << dutyCycle*100 << "%, period =" << period
		<< "s" << endl;

		current = par("current");
		wakeup = par("wakeup");

		EV << "current = " << current << "mA,  wakeup = "<< wakeup
		<< "mW-s" << endl;

		// batteryState and hostState are published with Host scope
		scopeHost = (this->findHost())->getId();

		// subscription to hostState is needed because this module
		// operates the "duty cycle protocol" and should stop when the
		// battery is depleted
		HostState hs;
		hostStateCat = utility->subscribe(this, &hs, scopeHost);
	}

	else if (stage == 1) {
		registerWithBattery("DeviceDutySimple", 1);

		off = new cMessage("turn-off", OFF);
		on = new cMessage("turn-on", ON);

		scheduleAt(simTime() + 0, on);
	}
}

		// alternate between ON (current > 0) and off (current == 0)

void DeviceDutySimple::handleMessage(cMessage *msg) {

	if (msg->isSelfMessage()) {
		EV<< simTime() << " " << msg->getName() << endl;

		switch (msg->getKind()) {
		case ON:
			// ON for dutyCycle, then OFF
			scheduleAt(simTime() + (dutyCycle * period), off);

			//drawMsg = new DrawMsg ("device wakeup 0", ENERGY);
			//drawMsg->setValue(wakeup);
			//send(drawMsg, batteryGate);
			drainEnergy(wakeup, 0);

			//drawMsg = new DrawMsg ("draw current", CURRENT);
			//drawMsg->setValue(current);
			//send(drawMsg, batteryGate);
			drainCurrent(current, 0);

			break;

		case OFF:
			// OFF for remaining time, then back to dutyCycle
			scheduleAt(simTime() + ((1 - dutyCycle) * period), on);

			//drawMsg = new DrawMsg ("device off", CURRENT);
			//drawMsg->setValue(0.0);
			//send(drawMsg, batteryGate);
			drainCurrent(0.0, 0);

			break;

		default:
			error("unknown selfMsg");
		}
	}
	else {
		error("unexpected external message");
	}
}

void DeviceDutySimple::receiveBBItem(int category, const BBItem *details,
		int scopeModuleId) {
	Enter_Method_Silent();
	BaseModule::receiveBBItem(category, details, scopeModuleId);

	if (category == hostStateCat) {

		HostState::States state = ((HostState *) details)->get();

		if (state == HostState::FAILED) {

			EV<< simTime() <<
			" hostState is FAILED...stopping operations" << endl;

			// in practice, the relevant protocol modules would have
			// received the FAIL message; but here we just stop the duty
			// cycle

			if (on->isScheduled()) cancelEvent(on);
			if (off->isScheduled()) cancelEvent(off);

		}
	}
}

void DeviceDutySimple::finish() {

	cancelAndDelete(on);
	cancelAndDelete(off);

	BaseModule::finish();
}
