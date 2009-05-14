/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRMac.cc
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008-2009 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: All MAC designed for use with UWB-IR should derive from this
 * 				class. It provides the necessary functions to build UWBIR
 * 				packets and to receive them.
 ***************************************************************************/

#include "UWBIRMac.h"
#include <iostream>

using namespace std;

Define_Module(UWBIRMac);

void UWBIRMac::initialize(int stage) {
	BaseMacLayer::initialize(stage);
	if (stage == 0) {
		debug = par("debug").boolValue();
		stats = par("stats").boolValue();
		trace = par("trace").boolValue();
		packetsAlwaysValid = par("packetsAlwaysValid");
		myMacAddr = par("MACAddr");
		rsDecoder = par("RSDecoder").boolValue();
		phy = FindModule<MacToPhyInterface*>::findSubModule(
				this->getParentModule());
		initCounters();
		catPacket = utility->getCategory(&packet);
	}
}

void UWBIRMac::initCounters() {
	totalRxBits = 0;
	errRxBits = 0;
	nbReceivedPacketsRS = 0;
	nbReceivedPacketsNoRS = 0;
	nbSentPackets = 0;
	nbSymbolErrors = 0;
	packetsBER.setName("packetsBER");
	meanPacketBER.setName("meanPacketBER");
	dataLengths.setName("dataLengths");
	sentPulses.setName("sentPulses");
	receivedPulses.setName("receivedPulses");
	erroneousSymbols.setName("nbErroneousSymbols");
}

void UWBIRMac::finish() {
	if (stats) {
		recordScalar("Erroneous bits", errRxBits);
		recordScalar("nbSymbolErrors", nbSymbolErrors);
		recordScalar("Total received bits", totalRxBits);
		recordScalar("Average BER", errRxBits / totalRxBits);
		recordScalar("nbReceivedPacketsRS", nbReceivedPacketsRS);
		recordScalar("nbReceivedPacketsnoRS", nbReceivedPacketsNoRS);
		if (rsDecoder) {
			recordScalar("nbReceivedPackets", nbReceivedPacketsRS);
		} else {
			recordScalar("nbReceivedPackets", nbReceivedPacketsNoRS);
		}
		recordScalar("nbSentPackets", nbSentPackets);
	}
}

void UWBIRMac::prepareData(UWBIRMacPkt* packet) {
	// generate signal
	int nbSymbols = packet->getByteLength() * 8 + 92; // to move to ieee802154a.h
	EV << "prepare Data for a packet with " << packet->getByteLength() << " data bytes. Requesting " << nbSymbols << " symbols." << endl;
	IEEE802154A::setPSDULength(packet->getByteLength());
	IEEE802154A::signalAndData res = IEEE802154A::generateIEEE802154AUWBSignal(
			simTime());
	Signal* theSignal = res.first;
	vector<bool>* data = res.second;
	if (trace) {
		int nbItems = 0;
		Mapping* power = theSignal->getTransmissionPower();
		ConstMappingIterator* iter = power->createConstIterator();
		iter->jumpToBegin();
		while (iter->hasNext()) {
			nbItems++;
			sentPulses.recordWithTimestamp(simTime()
					+ iter->getPosition().getTime(), iter->getValue());
			iter->next();
			simtime_t t = simTime() + iter->getPosition().getTime();
			//EV << "nbItemsTx=" << nbItems << ", t= " << t <<  ", value=" << iter->getValue() << "." << endl;
		}
	}

	// save bit values
	//packet->setBitValuesArraySize(data->size());
	for (int pos = 0; pos < nbSymbols; pos = pos + 1) {
		packet->setBitValues(pos, data->at(pos));
	}
	delete data;

	packet->setNbSymbols(nbSymbols);

	// attach control info
	MacToPhyControlInfo* macPhycInfo = new MacToPhyControlInfo(theSignal);
	packet->setControlInfo(macPhycInfo);

}

bool UWBIRMac::validatePacket(UWBIRMacPkt *mac) {
	if (!packetsAlwaysValid) {
		PhyToMacControlInfo * phyToMac = dynamic_cast<PhyToMacControlInfo*> (mac->getControlInfo());
		UWBIRDeciderResult * res = dynamic_cast<UWBIRDeciderResult*>(phyToMac->getDeciderResult());
		const std::vector<bool> * decodedBits = res->getDecodedBits();
		int bitsToDecode = mac->getNbSymbols();
		int nbBitErrors = 0;
		int pktSymbolErrors = 0;
		bool currSymbolError = false;

		for (int i = 0; i < bitsToDecode; i++) {
			// Start of a new symbol
			if (i % IEEE802154A::RSSymbolLength == 0) {
				currSymbolError = false;
			}
			// bit error
			if (decodedBits->at(i) != mac->getBitValues(i)) {
				nbBitErrors++;
				EV<< "Found an error at position " << i << "." << endl;
				// symbol error
				if(!currSymbolError) {
					currSymbolError = true;
					pktSymbolErrors = pktSymbolErrors + 1;
				}
			}
		}
		EV << "Found " << nbBitErrors << " bit errors in MAC packet." << endl;
		double packetBER = static_cast<double>(nbBitErrors)/static_cast<double>(bitsToDecode);
		packetsBER.record(packetBER);
		meanBER.collect(packetBER);
		meanPacketBER.record(meanBER.getMean());
		if(trace) {
			erroneousSymbols.record(pktSymbolErrors);
		}

		if(stats) {
			totalRxBits += bitsToDecode;
			errRxBits += nbBitErrors;
			nbSymbolErrors += pktSymbolErrors;
		}

		// ! If this condition is true then the next one will also be true
		if(nbBitErrors == 0) {
			nbReceivedPacketsNoRS++;
			packet.setNbPacketsReceivedNoRS(packet.getNbPacketsReceivedNoRS()+1);
		}

		if (pktSymbolErrors <= IEEE802154A::RSMaxSymbolErrors) {
			nbReceivedPacketsRS++;
			packet.setNbPacketsReceived(packet.getNbPacketsReceived()+1);
			utility->publishBBItem(catPacket, &packet, -1);
		}

		// validate message
		bool success = false;

		success = (nbBitErrors == 0 || (rsDecoder && nbSymbolErrors <= IEEE802154A::RSMaxSymbolErrors) );

		return success;
	}
	return true;
}

void UWBIRMac::handleLowerMsg(cPacket *msg) {
	UWBIRMacPkt *mac = static_cast<UWBIRMacPkt *> (msg);

	if (validatePacket(mac)) {
		int dest = mac->getDestAddr();
		int src = mac->getSrcAddr();
		if ((dest == myMacAddr)) {
			coreEV<< "message with mac addr " << src
			<< " for me (dest=" << dest
			<< ") -> forward packet to upper layer\n";
			sendUp(decapsMsg(mac));
		} else {
			coreEV << "message with mac addr " << src
			<< " not for me (dest=" << dest
			<< ") -> delete (my MAC=" << myMacAddr << ")\n";
			delete mac;
		}
	} else {
		EV << "Errors in message ; dropping mac packet." << endl;
		delete mac;
	}
}

