
#ifndef PHYLAYERDETAILED_H_
#define PHYLAYERDETAILED_H_

#include "PhyLayerBattery.h"
#include "RadioDetailed.h"
#include "MacToPhyDetailedInterface.h"


class PhyLayerDetailed : public PhyLayerBattery,
						 public MacToPhyDetailedInterface {

protected:
	double onCurrent, setupOnCurrent;
	RadioDetailed* radioDetailed;
	virtual Radio* initializeRadio(); 
	virtual void setSwitchingCurrent(int from, int to);

	/**
	 * @brief Creates and returns an instance of the Decider with the specified
	 * name.
	 *
	 * Is able to initialize the following Deciders:
	 *
	 * - Decider80211
	 * - Decider80211Battery
	 * - Decider80211MultiChannel
	 * - SNRThresholdDecider
	 */
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	/**
	 * @brief Initializes a new Decider80211MultiChannel from the passed parameter map.
	 */
	virtual Decider* initializeDecider80211MultiChannel(ParameterMap& params);

public:
	/** @brief Sets the channel currently used by the radio. */
	virtual void setCurrentRadioChannel(int newRadioChannel);
	/** @brief Returns the channel currently used by the radio. */
	virtual int getCurrentRadioChannel();
	/** @brief Returns the number of channels available on this radio. */
	virtual int getNbRadioChannels();

	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to] and that match the currently selected channel.
	 */
	//virtual void getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out);

	virtual AirFrame *encapsMsg(cPacket *macPkt);
};

#endif
