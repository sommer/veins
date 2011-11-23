//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "PhyLayerBattery.h"

#include "Decider80211MultiChannel.h"
#include "MacToPhyControlInfo.h"
#include "MacPkt_m.h"

Define_Module(PhyLayerBattery);

void PhyLayerBattery::initialize(int stage) {
	PhyLayer::initialize(stage);
	if (stage == 0) {
		numActivities = hasPar("numActivities") ? par("numActivities").longValue() : 5;

		/* parameters belong to the NIC, not just phy layer
		 *
		 * if/when variable transmit power is supported, txCurrent
		 * should be specified as an xml table of available transmit
		 * power levels and corresponding txCurrent */
		sleepCurrent = rxCurrent = decodingCurrentDelta = txCurrent = 0;
		setupRxCurrent = setupTxCurrent = rxTxCurrent = txRxCurrent = 0;
		sleepCurrent = getParentModule()->par( "sleepCurrent" );
		rxCurrent = getParentModule()->par( "rxCurrent" );
		decodingCurrentDelta = getParentModule()->par( "decodingCurrentDelta" );
		txCurrent = getParentModule()->par( "txCurrent" );
		setupRxCurrent = getParentModule()->par( "setupRxCurrent" );
		setupTxCurrent = getParentModule()->par( "setupTxCurrent" );
		rxTxCurrent = getParentModule()->par( "rxTxCurrent" );
		txRxCurrent = getParentModule()->par( "txRxCurrent" );
	} else {
		registerWithBattery("physical layer", numActivities);
		setRadioCurrent(radio->getCurrentState());
	}
}

Decider* PhyLayerBattery::getDeciderFromName(std::string name, ParameterMap& params) {
	if(name == "Decider80211Battery") {
		return initializeDecider80211Battery(params);
	}
	else if(name == "Decider80211MultiChannel") {
		return initializeDecider80211MultiChannel(params);
	}

	return PhyLayer::getDeciderFromName(name, params);
}

Decider* PhyLayerBattery::initializeDecider80211Battery(ParameterMap& params) {
	double threshold = params["threshold"];
	return new Decider80211Battery(this,
								   threshold,
								   sensitivity,
								   radio->getCurrentChannel(),
								   decodingCurrentDelta,
								   findHost()->getIndex(),
								   coreDebug);
}

Decider* PhyLayerBattery
			::initializeDecider80211MultiChannel(ParameterMap& params)
{
	double threshold = params["threshold"];
	return new Decider80211MultiChannel(this,
								   threshold,
								   sensitivity,
								   decodingCurrentDelta,
								   radio->getCurrentChannel(),
								   findHost()->getIndex(),
								   coreDebug);
}

void PhyLayerBattery::drawCurrent(double amount, int activity) {
	if(radio->getCurrentState() == Radio::RX) {
		if(amount != 0.0) {
			BatteryAccess::drawCurrent(rxCurrent + amount, DECIDER_ACCT + activity);
		} else {
			BatteryAccess::drawCurrent(rxCurrent, RX_ACCT);
		}
	} else {
		opp_warning("Decider wanted to change power consumption while radio not in state RX.");
	}
}

void PhyLayerBattery::handleUpperMessage(cMessage* msg) {
	if (battery && battery->getState() != HostState::ACTIVE) {
		coreEV<< "host has FAILED, dropping msg " << msg->getName() << endl;
		delete msg;
		return;
	}

	MacPkt* pkt = static_cast<MacPkt*>(msg);
	MacToPhyControlInfo* cInfo = static_cast<MacToPhyControlInfo*>(pkt->getControlInfo());

	double current = calcTXCurrentForPacket(pkt, cInfo);

	if(current > 0) {
		BatteryAccess::drawCurrent(current, TX_ACCT);
	}

	PhyLayer::handleUpperMessage(msg);
}

void PhyLayerBattery::handleAirFrame(AirFrame* frame) {
	if (battery && battery->getState() != HostState::ACTIVE) {
		coreEV<< "host has FAILED, dropping air frame msg " << frame->getName() << endl;
		delete frame;
		return;
	}
	PhyLayer::handleAirFrame(frame);
}

void PhyLayerBattery::handleHostState(const HostState& state) {
	if (state.get() != HostState::ACTIVE && radio->getCurrentState() != Radio::SLEEP) {
		coreEV<< "host is no longer in active state (maybe FAILED, SLEEP, OFF or BROKEN), force into sleep state!" << endl;
		setRadioState(Radio::SLEEP);
		// it would be good to create a radioState OFF, as well
	}
}

void PhyLayerBattery::finishRadioSwitching() {
	PhyLayer::finishRadioSwitching();

	setRadioCurrent(radio->getCurrentState());
}

void PhyLayerBattery::setSwitchingCurrent(int from, int to) {
	double current = 0;

	switch(from) {
	case Radio::RX:
		switch(to) {
		case Radio::SLEEP:
			current = rxCurrent;
			break;
		case Radio::TX:
			current = rxTxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From RX to %d", to);
			break;
		}
		break;

	case Radio::TX:
		switch(to) {
		case Radio::SLEEP:
			current = txCurrent;
			break;
		case Radio::RX:
			current = txRxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From TX to %d", to);
			break;
		}
		break;

	case Radio::SLEEP:
		switch(to) {
		case Radio::TX:
			current = setupTxCurrent;
			break;
		case Radio::RX:
			current = setupRxCurrent;
			break;
		default:
			opp_error("Unknown radio switch! From SLEEP to %d", to);
			break;
		}
		break;

	default:
		opp_error("Unknown radio state: %d", from);
		break;
	}

	BatteryAccess::drawCurrent(current, SWITCHING_ACCT);
}

void PhyLayerBattery::setRadioCurrent(int rs) {
	switch(rs) {
	case Radio::RX:
		BatteryAccess::drawCurrent(rxCurrent, RX_ACCT);
		break;
	case Radio::TX:
		BatteryAccess::drawCurrent(txCurrent, TX_ACCT);
		break;
	case Radio::SLEEP:
		BatteryAccess::drawCurrent(sleepCurrent, SLEEP_ACCT);
		break;
	default:
		opp_error("Unknown radio state: %d", rs);
		break;
	}
}

simtime_t PhyLayerBattery::setRadioState(int rs) {
	Enter_Method_Silent();
	int prevState = radio->getCurrentState();

	if (battery) {
		if (battery && battery->getState() != HostState::ACTIVE && rs != Radio::SLEEP && prevState != rs) {
			coreEV << "can not switch radio state, host is not in active state!" << endl;
			return -1;
		}
	}

	simtime_t endSwitch = PhyLayer::setRadioState(rs);

	if(endSwitch >= 0) {
		if(radio->getCurrentState() == Radio::SWITCHING) {
			setSwitchingCurrent(prevState, rs);
		}
	}

	return endSwitch;
}
