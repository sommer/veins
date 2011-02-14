#ifndef MACTOPHYDETAILEDINTERFACE_H
#define MACTOPHYDETAILEDINTERFACE_H

#include "MacToPhyInterface.h"


/**
 * @brief Extends the MacToPhyInterface to support multichannel radios.
 *
 * @ingroup macLayer
 * @ingroup phyLayer
 */
class MacToPhyDetailedInterface {
public:
	/** @brief Sets the channel currently used by the radio. */
	virtual void setCurrentRadioChannel(int newRadioChannel) = 0;
	/** @brief Returns the channel currently used by the radio. */
	virtual int getCurrentRadioChannel() = 0;
	/** @brief Returns the number of channels available on this radio. */
	virtual int getNbRadioChannels() = 0;
};

#endif // MACTOPHYDETAILEDINTERFACE_H

