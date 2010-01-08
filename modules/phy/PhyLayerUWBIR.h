/* -*- mode:c++ -*- ********************************************************
 * file:        PhyLayerUWBIR.h
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
//
// Physical layer that models an Ultra Wideband Impulse Radio transceiver.
// See the following publication for more information.
//  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//

#ifndef UWBIR_PHY_LAYER_H
#define UWBIR_PHY_LAYER_H

#include "PhyLayerBattery.h"
#include "RadioUWBIR.h"
#include "UWBIRStochasticPathlossModel.h"
#include "UWBIRIEEE802154APathlossModel.h"
#include "HostState.h"
#include "MacToPhyControlInfo.h"
#include "BaseUtility.h"

/*
 * @brief This class implements an Ultra Wideband Impulse Radio physical layer
 * and uses an implementation of the IEEE 802.15.4A channel model, a Burst Position
 * Modulation transmitter (at the MAC level), and a non-coherent energy-detection receiver.
 */


class DeciderUWBIRED;
class DeciderUWBIREDSyncOnAddress;
class DeciderUWBIREDSync;

#include "DeciderUWBIRED.h"
#include "DeciderUWBIREDSyncOnAddress.h"
#include "DeciderUWBIREDSync.h"

class PhyLayerUWBIR : public BasePhyLayer
{
	friend class DeciderUWBIRED;

public:
	void initialize(int stage);
        PhyLayerUWBIR() : uwbpathloss(0), ieee802154AChannel(0) {}

    void finish();

    virtual BaseUtility* getUtility() { return utility; };

protected:

    UWBIRStochasticPathlossModel* uwbpathloss;
    UWBIRIEEE802154APathlossModel* ieee802154AChannel;
    DeciderUWBIRED* uwbdecider;

    virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

    AnalogueModel* createUWBIRStochasticPathlossModel(ParameterMap & params);
    AnalogueModel* createUWBIRIEEE802154APathlossModel(ParameterMap & params);
    AnalogueModel* createIntensityModel(ParameterMap & params);
    virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);
    virtual Radio* initializeRadio();

    RadioUWBIR* uwbradio;
    /**
     * called by Blackboard to inform of changes
     */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

    virtual void handleAirFrame(cMessage* msg);

    virtual void switchRadioToRX() {
    	Enter_Method_Silent();
    	uwbradio->startReceivingFrame(simTime());
    	setRadioCurrent(uwbradio->getCurrentState());
    }

    virtual void switchRadioToSync() {
    	Enter_Method_Silent();
    	uwbradio->finishReceivingFrame(simTime());
    	setRadioCurrent(radio->getCurrentState());
    }


    virtual simtime_t setRadioState(int rs);

	/** @brief Number of power consuming activities (accounts).*/
	int numActivities;

	/** @brief The different currents in mA.*/
	double sleepCurrent, rxCurrent, decodingCurrentDelta, txCurrent, syncCurrent;

	/** @brief The differnet switching state currents in mA.*/
	double setupRxCurrent, setupTxCurrent, rxTxCurrent, txRxCurrent;

	/**
	 * @brief Defines the power consuming activities (accounts) of
	 * the NIC. Should be the same as defined in the decider.
	 */
	enum Activities {
		SLEEP_ACCT=0,
		RX_ACCT,  		//1
		TX_ACCT,  		//2
		SWITCHING_ACCT, //3
		SYNC_ACCT,		//4
	};

	virtual void setRadioCurrent(int rs);

	virtual void setSwitchingCurrent(int from, int to);

	/**
	 * @brief Captures changes in host state.
	 *
	 * Note: Does not yet cancel any ongoing transmissions if the
	 * state changes to off.
	 */
	virtual void handleHostState(const HostState& state);

	/**
	 * @brief Captures radio switches to adjust power consumption.
	 */
	virtual void finishRadioSwitching();

};

#endif
