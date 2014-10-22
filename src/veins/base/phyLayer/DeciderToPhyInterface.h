/* -*- mode:c++ -*- ********************************************************
 *
 *
 *
 */

#ifndef DECIDER_TO_PHY_INTERFACE_H_
#define DECIDER_TO_PHY_INTERFACE_H_

#include <vector>
#include <list>
#include <omnetpp.h>

#include "veins/base/utils/MiXiMDefs.h"

namespace Veins {
class AirFrame;
}
using Veins::AirFrame;

class BaseWorldUtility;
class ConstMapping;

/**
 * See Decider.h for definition of DeciderResult
 */
class DeciderResult;

/**
 * @brief This class is the interface for a Decider to the BasePhyLayer.
 *
 * The Decider can do the following things using it:
 *
 *		- get the current simulation time
 * 		- get the list of AirFrames that intersect with a specific time interval (to
 * 			calculate SNR)
 * 		- tell the BasePhyLayer to hand an AirFrame up to the MACLayer
 * 		- tell the BasePhyLayer to send a control message to the MACLayer (to answer
 * 			a ChannelSenseRequest)
 *
 * @ingroup decider
 */
class MIXIM_API DeciderToPhyInterface
{
public:
	/**
	 * @brief Type for container of AirFrames.
	 *
	 * Used as out-value in "getChannelInfo" method.
	 */
	typedef std::list<AirFrame*> AirFrameVector;

	virtual ~DeciderToPhyInterface() {}

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to]
	 */
	virtual void getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) = 0;

	/**
	 * @brief Returns a Mapping which defines the thermal noise in
	 * the passed time frame (in mW).
	 *
	 * The implementing class of this method keeps ownership of the
	 * Mapping.
	 */
	virtual ConstMapping* getThermalNoise(simtime_t_cref from, simtime_t_cref to) = 0;

	/**
	 * @brief Called by the Decider to send a control message to the MACLayer
	 *
	 * This function can be used to answer a ChannelSenseRequest to the MACLayer
	 *
	 */
	virtual void sendControlMsgToMac(cMessage* msg) = 0;

	/**
	 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
	 *
	 * When a packet is completely received and not noise, the Decider
	 * call this function to send the packet together with
	 * the corresponding DeciderResult up to MACLayer
	 *
	 */
	virtual void sendUp(AirFrame* packet, DeciderResult* result) = 0;

	/**
	 * @brief Returns the current simulation time
	 *
	 */
	virtual simtime_t getSimTime() = 0;

	/**
	 * @brief Tells the PhyLayer to cancel a scheduled message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it doesn't need to handle an AirFrame or
	 * ControlMessage again anymore.
	 */
	virtual void cancelScheduledMessage(cMessage* msg) = 0;

	/**
	 * @brief Tells the PhyLayer to reschedule a message (AirFrame or
	 * ControlMessage).
	 *
	 * Used by the Decider if it has to handle an AirFrame or an control message
	 * earlier than it has returned to the PhyLayer the last time the Decider
	 * handled that message.
	 */
	virtual void rescheduleMessage(cMessage* msg, simtime_t_cref t) = 0;

	/**
	 * @brief Enables the Decider to draw Power from the
	 * phy layers power accounts.
	 *
	 * Does nothing if no Battery module in simulation is present.
	 */
	virtual void drawCurrent(double amount, int activity) = 0;

	/**
	 * @brief Returns a pointer to the simulations world-utility-module.
	 */
	virtual BaseWorldUtility* getWorldUtility() = 0;

	/**
	 * @brief Utility method to enable a Decider, which isn't an OMNeT-module, to
	 * use the OMNeT-method 'recordScalar' with the help of and through its interface to BasePhyLayer.
	 *
	 * The method-signature is taken from OMNeTs 'ccomponent.h' but made pure virtual here.
	 * See the original method-description below:
	 *
	 * Records a double into the scalar result file.
	 */
	virtual void recordScalar(const char *name, double value, const char *unit=NULL) = 0;

	/** @brief Returns the channel currently used by the radio. */
	virtual int getCurrentRadioChannel() = 0;
};

#endif /*DECIDER_TO_PHY_INTERFACE_H_*/
