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

#include "WorldUtilityStats.h"
#include "Packet.h"
#include "BaseLayer.h"

Define_Module(WorldUtilityStats);

void WorldUtilityStats::initialize(int stage)
{
	BaseWorldUtility::initialize(stage);
	if(stage == 0) {
		recordVectors = par("recordVectors");
		bitrate = par("bitrate");

		bitsSent = 0;
		bitsReceived = 0;

		//register for global stats to collect
		subscribe(BaseLayer::catPacketSignal, this);

		sent.setName("Bits generated");
		rcvd.setName("Bits received");
	}
}


void WorldUtilityStats::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
	Enter_Method_Silent();
	if(signalID == BaseLayer::catPacketSignal)
	{
		const Packet* p = static_cast<const Packet*>(obj);
		double nbBitsSent = p->getNbBitsSent();
		double nbBitsRcvd = p->getNbBitsReceived();
		bitsSent += nbBitsSent;
		bitsReceived += nbBitsRcvd;

		if(recordVectors) {
			sent.record(bitsSent);
			rcvd.record(bitsReceived);
		}
	}
}

void WorldUtilityStats::finish()
{
	recordScalar("GlobalTrafficGenerated", bitsSent, "bit");
	recordScalar("GlobalTrafficReceived", bitsReceived, "bit");

	recordScalar("Traffic", bitsSent / bitrate / simTime());
	double hosts = simulation.getSystemModule()->par("numHosts");
	if(!par("bcTraffic"))
		hosts = 2;
	recordScalar("Usage", bitsReceived / bitrate / simTime() / (hosts-1));
}
