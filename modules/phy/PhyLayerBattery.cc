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

Define_Module(PhyLayerBattery);

void PhyLayerBattery::initialize(int stage) {
	PhyLayer::initialize(stage);
	if (stage == 0) {
		numActivities = readPar("numActivities", 4);

		/* host failure notification */
		int scopeHost = (this->findHost())->getId();
		HostState hs;
		hostStateCat = utility->subscribe(this, &hs, scopeHost);

		hostState = HostState::ON;

		/* parameters belong to the NIC, not just snrEval
		 *
		 * if/when variable transmit power is supported, txCurrent
		 * should be specified as an xml table of available transmit
		 * power levels and corresponding txCurrent */
		sleepCurrent = idleCurrent = rxCurrent = txCurrent = 0;
		sleepCurrent = getParentModule()->par( "sleepCurrent" );
		idleCurrent = getParentModule()->par( "idleCurrent" );
		rxCurrent = getParentModule()->par( "rxCurrent" );
		txCurrent = getParentModule()->par( "txCurrent" );
	} else {
		registerWithBattery("80211Nic", numActivities);

		drawCurrent(idleCurrent, IDLE_ACCT);
	}
}

Decider* PhyLayerBattery::getDeciderFromName(std::string name, ParameterMap& params) {
	if(name == "Decider80211Battery") {
		return initializeDecider80211Battery(params);
	}

	return PhyLayer::getDeciderFromName(name, params);
}

Decider* PhyLayerBattery::initializeDecider80211Battery(ParameterMap& params) {
	double threshold = params["threshold"];
	double centerFreq = params["centerFrequency"];
	return new Decider80211Battery(this,
								   threshold,
								   sensitivity,
								   centerFreq,
								   rxCurrent,
								   idleCurrent,
								   findHost()->getIndex(),
								   coreDebug);
}

//void PhyLayerBattery::drawCurrent(double amount, int activity) {
//	drawCurrent(amount, activity);
//}

void PhyLayerBattery::handleUpperMessage(cMessage* msg) {
	if (hostState == HostState::FAILED) {
		EV<< "host has FAILED, dropping msg " << msg->getName() << endl;
		delete msg;
		return;
	}

	PhyLayer::handleUpperMessage(msg);

	drawCurrent(txCurrent, TX_ACCT);
}

void PhyLayerBattery::handleSelfMessage(cMessage* msg) {
	if(msg->getKind() == TX_OVER) {
		drawCurrent(idleCurrent, IDLE_ACCT);
	}
	PhyLayer::handleSelfMessage(msg);
}

void PhyLayerBattery::handleAirFrame(cMessage* msg) {
	if (hostState == HostState::FAILED) {
		EV<< "host has FAILED, dropping msg " << msg->getName() << endl;
		delete msg;
		return;
	}

	PhyLayer::handleAirFrame(msg);
}

void PhyLayerBattery::receiveBBItem(int category, const BBItem *details,
		int scopeModuleId) {
	Enter_Method_Silent();

	PhyLayer::receiveBBItem(category, details, scopeModuleId);

	// handles only battery consumption, everything else handled above

	if (category == hostStateCat) {

		hostState = ((HostState *) details)->get();

		switch (hostState) {
		case HostState::FAILED:
			EV<< "t = " << simTime() << " host state FAILED" << endl;
			// it would be good to create a radioState OFF, as well
			break;
		default:
			break;
		}
	}
}
