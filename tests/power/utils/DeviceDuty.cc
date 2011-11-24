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
 *  A trivial device module that has a fixed two-phase duty cycle, but
 *  is not actually connected to the operation of the host.  Is useful
 *  only for testing multiple accounts.
 *
 *  Alternates between ON (draws CURRENT current0 or current1) and OFF
 *  state twice in each period.  Switching from OFF to ON (draws
 *  wakeup ENERGY).  The energy consumed in each of the two ON periods
 *  and by the transitions are assigned to accounts DUTY0, DUTY1, and
 *  WAKE respectively.  See the file DeviceDuty.ned for parameters.
 *
 ***************************************************************************/
#include "DeviceDuty.h"

Define_Module(DeviceDuty)
;

void DeviceDuty::initialize(int stage) {
	BatteryAccess::initialize(stage);

	if (stage == 0) {

		dutyCycle0 = par("dutyCycle0");
		if (dutyCycle0 < 0 || dutyCycle0 > 1.0) {
			error("invalid dutyCycle0");
		}
		gap = par("gap");
		dutyCycle1 = par("dutyCycle1");
		if (dutyCycle1 < 0 || dutyCycle1 > 1.0) {
			error("invalid dutyCycle1");
		}

		if ((dutyCycle0 + gap + dutyCycle1) > 1.0)
			error("duty cycles are greater than the period");

		period = par("period");

		EV<< "dutyCycle0 = " << dutyCycle0*100 << "%, gap =" << gap
		<< "%, dutyCycle1 = " << dutyCycle1*100 << "%, period ="
		<< period << "s" << endl;

		current0 = par("current0");
		current1 = par("current1");
		wakeup = par("wakeup");
		EV << "current0 = " << current0 << "mA, current1 = " << current1
		<< "mA, wakeup = "<< wakeup << "mW-s" << endl;

	}

	else if (stage == 1) {

/*
		 batteryGate = findGate("battery");

		 RegisterMsg *registerMsg;
		 registerMsg = new RegisterMsg("register", REGISTER);
		 registerMsg->setDeviceName("DeviceDuty");
		 registerMsg->setNumAccounts(3);
		 send(registerMsg, batteryGate);
*/
		registerWithBattery("DeviceDuty", 3);


		off = new cMessage("turn-off", OFF);
		on0 = new cMessage("turn-on0", ON0);
		gap01 = new cMessage("turn-off (gap)", GAP);
		on1 = new cMessage("turn-on1", ON1);

		scheduleAt(simTime() + 0, on0);
	}
}

		// alternate between ON (current > 0) and off (current == 0)

void DeviceDuty::handleMessage(cMessage *msg) {

	if (msg->isSelfMessage()) {
		EV<< simTime() << " " << msg->getName() << endl;

		switch (msg->getKind()) {
			case ON0:
//			drawMsg = new DrawMsg ("device wakeup 0", ENERGY);
//			drawMsg->setValue(wakeup);
//			drawMsg->setActivity(WAKE);
//			send(drawMsg, batteryGate);
			drawEnergy(wakeup, WAKE);

//			drawMsg = new DrawMsg ("draw current", CURRENT);
//			drawMsg->setValue(current0);
//			drawMsg->setActivity(DUTY0);
//			send(drawMsg, batteryGate);
			drawCurrent(current0, DUTY0);

			// ON for dutyCycle0, then the gap
			scheduleAt(simTime() + (dutyCycle0 * period), gap01);
			break;

			case GAP:
			//drawMsg = new DrawMsg ("device off(gap)", CURRENT);
			//drawMsg->setValue(0.0);
			// energy is 0 so account doesn't matter
			//send(drawMsg, batteryGate);
			drawCurrent(0.0, 0);

			// OFF for gap, then dutyCycle1
			scheduleAt(simTime() + (gap * period), on1);
			break;

			case ON1:
			//drawMsg = new DrawMsg ("device wakeup1", ENERGY);
			//drawMsg->setValue(wakeup);
			//drawMsg->setActivity(WAKE);
			//send(drawMsg, batteryGate);
				drawEnergy(wakeup, WAKE);

			//drawMsg = new DrawMsg ("draw current", CURRENT);
			//drawMsg->setValue(current1);
			//drawMsg->setActivity(DUTY1);
			//send(drawMsg, batteryGate);
				drawCurrent(current1, DUTY1);

			// ON for DutyCycle1, then off
			scheduleAt(simTime() + (dutyCycle1 * period), off);
			break;

			case OFF:
			{
				//drawMsg = new DrawMsg ("device off", CURRENT);
				//drawMsg->setValue(0.0);
				// energy is 0 so account doesn't matter
				//send(drawMsg, batteryGate);
				drawCurrent(0.0, 0);

				// OFF for remaining time, then back to dutyCycle0
				double timeLeft = period * (1.0 - (dutyCycle0 + gap + dutyCycle1));
				scheduleAt(simTime() + timeLeft, on0);
				break;
			}
			default:
			error("unknown selfMsg");
		}
	}
	else {
		error("unexpected external message");
	}
}

void DeviceDuty::handleHostState(const HostState& hostState) {


	HostState::States state = hostState.get();

	if (state == HostState::FAILED) {

		EV<< simTime() <<
		" hostState is FAILED...stopping operations" << endl;

		// in practice, the relevant protocol modules would have
		// received the FAIL message; but here we just stop the duty
		// cycle

		if (on0->isScheduled()) cancelEvent(on0);
		if (gap01->isScheduled()) cancelEvent(gap01);
		if (on1->isScheduled()) cancelEvent(on1);
		if (off->isScheduled()) cancelEvent(off);

	}
}

DeviceDuty::~DeviceDuty() {
}

void DeviceDuty::finish() {
	cComponent::finish();
	cancelAndDelete(on0);
	cancelAndDelete(gap01);
	cancelAndDelete(on1);
	cancelAndDelete(off);
}
