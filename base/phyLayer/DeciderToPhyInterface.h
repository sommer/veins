/* -*- mode:c++ -*- ********************************************************
 * 
 * 
 * 
 */

#ifndef DECIDER_TO_PHY_INTERFACE_H_
#define DECIDER_TO_PHY_INTERFACE_H_

#include <vector>
#include <omnetpp.h>

#include "AirFrame.h"

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
 */
class DeciderToPhyInterface
{
	public:
		typedef std::vector<AirFrame*> AirFrameVector;
		
		/**
		 * @brief Fills the passed AirFrameVector with all AirFrames that intersect 
		 * with the time interval [from, to]
		 */
		virtual void getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out) = 0;
		
		/** 
		 * @brief Called by the Decider to send a control message to the MACLayer
		 * 
		 * This function can be used to answer a ChannelSenseRequest to the MACLayer
		 * 
		 */
		virtual void sendControlMsg(cMessage* msg) = 0;
		
		/** 
		 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
		 * 
		 * When a packet is completely received and not noise, the Decider
		 * call this function to send the packet together with
		 * the corresponding DeciderResult up to MACLayer
		 * 
		 */
		virtual void sendUp(AirFrame* packet, DeciderResult result) = 0;
		
		/**
		 * @brief Returns the current simulation time
		 * 
		 */
		virtual simtime_t getSimTime() = 0;
};

#endif /*DECIDER_TO_PHY_INTERFACE_H_*/
