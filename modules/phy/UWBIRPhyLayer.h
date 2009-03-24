/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRPhyLayer.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this physical layer models an ultra wideband impulse radio channel.
 ***************************************************************************/

#ifndef UWBIR_PHY_LAYER_H
#define UWBIR_PHY_LAYER_H

#include "BasePhyLayer.h"
#include "UWBIRRadio.h"
#include "UWBIRStochasticPathlossModel.h"
#include "UWBIRIEEE802154APathlossModel.h"

#include "MacToPhyControlInfo.h"
#include "BaseUtility.h"

/*
 * @brief This class implements an Ultra Wideband Impulse Radio physical layer
 * and uses an implementation of the IEEE 802.15.4A channel model, a Burst Position
 * Modulation transmitter (at the MAC level), and a non-coherent energy-detection receiver.
 */

class UWBIREnergyDetectionDeciderV2;
class UWBIREDSyncOnAddress;
class UWBIREDSync;

#include "UWBIREnergyDetectionDeciderV2.h"
#include "UWBIREDSyncOnAddress.h"
#include "UWBIREDSync.h"

class UWBIRPhyLayer : public BasePhyLayer
{
	friend class UWBIREnergyDetectionDeciderV2;

public:
	void initialize(int stage);
        UWBIRPhyLayer() : uwbpathloss(0), ieee802154AChannel(0) {}

    void finish();


protected:

	UWBIRRadio * _radio;

    UWBIRStochasticPathlossModel* uwbpathloss;
    UWBIRIEEE802154APathlossModel* ieee802154AChannel;

    virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

    AnalogueModel* createUWBIRStochasticPathlossModel(ParameterMap & params);
    AnalogueModel* createUWBIRIEEE802154APathlossModel(ParameterMap & params);
    virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);
    /**
     * called by Blackboard to inform of changes
     */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

    virtual void switchRadioToRX() {
    	Enter_Method_Silent();
    	_radio->startReceivingFrame(simTime());
    }

    virtual void switchRadioToSync() {
    	Enter_Method_Silent();
    	_radio->finishReceivingFrame(simTime());
    }

};

#endif
