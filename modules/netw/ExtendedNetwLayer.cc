/*
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2010 CSEM SA, Neuchatel, Switzerland.
 *
 * description: ExtendedNetwLayer allows to choose between ARP layers.
 *				By default, it selects ArpHost, which uses the host module index
 *				for both the mac and net addresses.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 */

#include "ExtendedNetwLayer.h"

Define_Module(ExtendedNetwLayer);



void ExtendedNetwLayer::initialize(int stage) {
	if(stage == 0) {
		arpMode = par("arpMode").stdstringValue();
	}
	BaseNetwLayer::initialize(stage);
}

BaseArp* ExtendedNetwLayer::initializeArp() {
	if(arpMode.compare("Host") == 0) {
		return ArpHostAccess().get();
	} else if (arpMode.compare("Auto") == 0) {
		return BaseNetwLayer::initializeArp();
	}
	return 0;
}
