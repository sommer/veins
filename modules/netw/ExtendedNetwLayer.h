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

#ifndef EXTENDED_NETW_LAYER_H
#define EXTENDED_NETW_LAYER_H

#include <BaseLayer.h>
#include "BaseNetwLayer.h"
#include "ArpHost.h"
#include "BaseArp.h"
#include <string>
#include <map>

class ExtendedNetwLayer : public BaseNetwLayer {
public:
	virtual void initialize(int stage);

protected:
	virtual BaseArp* initializeArp();
	std::string arpMode;
};

#endif

