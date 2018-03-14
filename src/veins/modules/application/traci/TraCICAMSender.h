//
// Copyright (C) 2015 Dominik Buse <dbuse@mail.uni-paderborn.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//


#ifndef TRACICAMSENDER_H_
#define TRACICAMSENDER_H_

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;

class TraCICAMSender : public BaseWaveApplLayer {
public:
	~TraCICAMSender();
	virtual void initialize(int stage);
protected:
	virtual void handleSelfMsg(cMessage* msg);
private:
	virtual void sendCAM();

	TraCIMobility* mobility;
	TraCICommandInterface* traci;
	TraCICommandInterface::Vehicle* traciVehicle;
	double camInterval;
	cMessage* camTimer;
};

#endif /* TRACICAMSENDER_H_ */
