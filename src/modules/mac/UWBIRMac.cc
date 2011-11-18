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
//
// See the following publications for more information:
// [1] An Ultra Wideband Impulse Radio PHY Layer Model for Network Simulation,
// J. Rousselot, J.-D. Decotignie, Simulation: Transactions of the Society
// for Computer Simulation, 2010 (submitted).
// [2] A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//

#include "UWBIRMac.h"

#include <iostream>
#include <math.h>

#include "MacToUWBIRPhyControlInfo.h"
#include "PhyToMacControlInfo.h"
#include "DeciderResultUWBIR.h"
#include "FindModule.h"
#include "UWBIRMacPkt_m.h"
#include "UWBIRMacPkt.h"
#include "MacToPhyInterface.h"

using namespace std;

Define_Module(UWBIRMac);

void UWBIRMac::initialize(int stage) {
	BaseMacLayer::initialize(stage);
	if (stage == 0) {
		debug = par("debug").boolValue();
		stats = par("stats").boolValue();
		trace = par("trace").boolValue();
		prf   = par("PRF");
		assert(prf == 4 || prf == 16);
		packetsAlwaysValid = par("packetsAlwaysValid");
		rsDecoder          = par("RSDecoder").boolValue();
		phy = FindModule<MacToPhyInterface*>::findSubModule( this->getParentModule() );
		initCounters();
	}
}

void UWBIRMac::initCounters() {
	totalRxBits = 0;
	errRxBits = 0;
	nbReceivedPacketsRS = 0;
	nbReceivedPacketsNoRS = 0;
	nbSentPackets = 0;
	nbSymbolErrors = 0;
	nbSymbolsReceived = 0;
	nbHandledRxPackets = 0;
	nbFramesDropped = 0;
	packetsBER.setName("packetsBER");
	meanPacketBER.setName("meanPacketBER");
	dataLengths.setName("dataLengths");
	sentPulses.setName("sentPulses");
	receivedPulses.setName("receivedPulses");
	erroneousSymbols.setName("nbErroneousSymbols");
	packetSuccessRate.setName("packetSuccessRate");
	packetSuccessRateNoRS.setName("packetSuccessRateNoRS");
	ber.setName("ber");
	RSErrorRate.setName("ser");
	success.setName("success");
	successNoRS.setName("successNoRS");
}

void UWBIRMac::finish() {
	if (stats) {
		recordScalar("Erroneous bits", errRxBits);
		recordScalar("nbSymbolErrors", nbSymbolErrors);
		recordScalar("Total received bits", totalRxBits);
		recordScalar("Average BER", errRxBits / totalRxBits);
		recordScalar("nbReceivedPacketsRS", nbReceivedPacketsRS);
		recordScalar("nbFramesDropped", nbFramesDropped);
		recordScalar("nbReceivedPacketsnoRS", nbReceivedPacketsNoRS);
		if (rsDecoder) {
			recordScalar("nbReceivedPackets", nbReceivedPacketsRS);
		} else {
			recordScalar("nbReceivedPackets", nbReceivedPacketsNoRS);
		}
		recordScalar("nbSentPackets", nbSentPackets);
	}
}

void UWBIRMac::prepareData(UWBIRMacPkt* packet, IEEE802154A::config cfg) {
	// generate signal
	//int nbSymbols = packet->getByteLength() * 8 + 92; // to move to ieee802154a.h
	debugEV << "prepare Data for a packet with " << packet->getByteLength() << " data bytes." << endl;
	if(prf == 4) {
	  IEEE802154A::setConfig(IEEE802154A::cfg_mandatory_4M);
	} else if (prf == 16) {
		IEEE802154A::setConfig(IEEE802154A::cfg_mandatory_16M);
	}
	IEEE802154A::setPSDULength(packet->getByteLength());
	IEEE802154A::signalAndData res = IEEE802154A::generateIEEE802154AUWBSignal(simTime());
	Signal* theSignal = res.first;
	vector<bool>* data = res.second;
	int nbSymbols = data->size();
	if (trace) {
		int nbItems = 0;
		ConstMapping* power = theSignal->getTransmissionPower();
		ConstMappingIterator* iter = power->createConstIterator();
		iter->jumpToBegin();
		while (iter->hasNext()) {
			nbItems++;
			sentPulses.recordWithTimestamp(simTime()
					+ iter->getPosition().getTime(), iter->getValue());
			iter->next();
			//simtime_t t = simTime() + iter->getPosition().getTime();
			//debugEV << "nbItemsTx=" << nbItems << ", t= " << t <<  ", value=" << iter->getValue() << "." << endl;
		}
	}

	// save bit values
	//packet->setBitValuesArraySize(data->size());
	for (int pos = 0; pos < nbSymbols; pos = pos + 1) {
		packet->pushBitvalue(data->at(pos));
		//packet->setBitValues(pos, data->at(pos));
	}
	delete data;

	packet->setNbSymbols(nbSymbols);

	// attach control info
	MacToUWBIRPhyControlInfo::setControlInfo(packet, theSignal, IEEE802154A::getConfig());
}

bool UWBIRMac::validatePacket(UWBIRMacPkt *mac) {
	nbHandledRxPackets++;
	if (!packetsAlwaysValid) {
		PhyToMacControlInfo * phyToMac = dynamic_cast<PhyToMacControlInfo*> (mac->getControlInfo());
		DeciderResultUWBIR * res = dynamic_cast<DeciderResultUWBIR*>(phyToMac->getDeciderResult());
		const std::vector<bool> * decodedBits = res->getDecodedBits();
		int bitsToDecode = mac->getNbSymbols();
		nbSymbolsReceived = nbSymbolsReceived + ceil((double)bitsToDecode / IEEE802154A::RSSymbolLength);
		int nbBitErrors = 0;
		int pktSymbolErrors = 0;
		bool currSymbolError = false;

		for (int i = 0; i < bitsToDecode; i++) {
			// Start of a new symbol
			if (i % IEEE802154A::RSSymbolLength == 0) {
				currSymbolError = false;
			}
			// bit error
			if (decodedBits->at(i) != mac->popBitValue()) {
				nbBitErrors++;
				debugEV<< "Found an error at position " << i << "." << std::endl;
				// symbol error
				if(!currSymbolError) {
					currSymbolError = true;
					pktSymbolErrors = pktSymbolErrors + 1;
				}
			}
		}

		debugEV << "Found " << nbBitErrors << " bit errors in MAC packet." << std::endl;
		double packetBER = static_cast<double>(nbBitErrors)/static_cast<double>(bitsToDecode);
		packetsBER.record(packetBER);
		meanBER.collect(packetBER);
		meanPacketBER.record(meanBER.getMean());
		if(trace) {
			erroneousSymbols.record(pktSymbolErrors);
		}

		// ! If this condition is true then the next one will also be true
		if(nbBitErrors == 0) {
			successNoRS.record(1);
			nbReceivedPacketsNoRS++;
			packet.setNbPacketsReceivedNoRS(packet.getNbPacketsReceivedNoRS()+1);
		} else {
			successNoRS.record(0);
		}

		if (pktSymbolErrors <= IEEE802154A::RSMaxSymbolErrors) {
			success.record(1);
			nbReceivedPacketsRS++;
			packet.setNbPacketsReceived(packet.getNbPacketsReceived()+1);
			emit(BaseLayer::catPacketSignal, &packet);
		} else {
			success.record(0);
			nbFramesDropped++;
		}

		if(stats) {
			totalRxBits += bitsToDecode;
			errRxBits += nbBitErrors;
			nbSymbolErrors += pktSymbolErrors;
			ber.record(errRxBits/totalRxBits);
			packetSuccessRate.record( ((double) nbReceivedPacketsRS)/nbHandledRxPackets);
			packetSuccessRateNoRS.record( ((double) nbReceivedPacketsNoRS)/nbHandledRxPackets);
			RSErrorRate.record( ((double) nbSymbolErrors) / nbSymbolsReceived);
		}

		// validate message
		bool success = false;

		success = (nbBitErrors == 0 || (rsDecoder && pktSymbolErrors <= IEEE802154A::RSMaxSymbolErrors) );

		return success;
	}
	return true;
}

void UWBIRMac::handleLowerMsg(cPacket *msg) {
	UWBIRMacPkt *mac = static_cast<UWBIRMacPkt *> (msg);

	if (validatePacket(mac)) {
		const LAddress::L2Type& dest = mac->getDestAddr();
		const LAddress::L2Type& src  = mac->getSrcAddr();
		if ((dest == myMacAddr)) {
			debugEV<< "message with mac addr " << src
			<< " for me (dest=" << dest
			<< ") -> forward packet to upper layer" << std::endl;
			sendUp(decapsMsg(mac));
		} else {
			debugEV << "message with mac addr " << src
			<< " not for me (dest=" << dest
			<< ") -> delete (my MAC=" << myMacAddr << ")" << std::endl;
			delete mac;
		}
	} else {
		debugEV << "Errors in message ; dropping mac packet." << std::endl;
		delete mac;
	}
}

