
#ifndef PHYLAYERDETAILED_H_
#define PHYLAYERDETAILED_H_

#include "PhyLayerBattery.h"
#include "RadioDetailed.h"
#include "MacToPhyDetailedInterface.h"


class PhyLayerDetailed : public PhyLayerBattery, MacToPhyDetailedInterface {

protected:
	double onCurrent, setupOnCurrent;
	RadioDetailed* radioDetailed;
	virtual Radio* initializeRadio(); 
	virtual void setSwitchingCurrent(int from, int to);
	/** @brief duration of PHY header (after which we can decide if we can synchronize on the frame). */
	double  phyHeaderDuration;
	cOutVector radioChannel;

public:
	/** @brief Sets the channel currently used by the radio. */
	virtual void setCurrentRadioChannel(int newRadioChannel);
	/** @brief Returns the channel currently used by the radio. */
	virtual int getCurrentRadioChannel();
	/** @brief Returns the number of channels available on this radio. */
	virtual int getNbRadioChannels();

	/** @brief Loads analogue models. */
	AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	/** @brief Initializes PER Model */
	AnalogueModel* initializePERModel(ParameterMap& params);
	/**
	 * @brief Fills the passed AirFrameVector with all AirFrames that intersect
	 * with the time interval [from, to] and that match the currently selected channel.
	 */
	virtual void getChannelInfo(simtime_t from, simtime_t to, AirFrameVector& out);

	virtual AirFrame *encapsMsg(cPacket *macPkt);
};

#endif
