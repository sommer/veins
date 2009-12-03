#ifndef MACTOPHYINTERFACE_H_
#define MACTOPHYINTERFACE_H_

#include <omnetpp.h>

#include "Signal_.h"
#include "ChannelState.h"
#include "PhyUtils.h"


/**
 * @brief Defines the methods provided by the phy to the mac layer.
 *
 * @ingroup macLayer
 * @ingroup phyLayer
 */
class MacToPhyInterface
{
public:
	enum BasePhyMessageKinds {
		/** Indicates the end of a send transmission. */
		TX_OVER = 22000,
		/** Indicates the end of a radio switch. */
		RADIO_SWITCHING_OVER,
		/** Channel sense control message between Mac and Phy.*/
		CHANNEL_SENSE_REQUEST,
		/** AirFrame kind */
		AIR_FRAME,
		/** Stores the id on which classes extending BasePhy should
		 * continue their own kinds.*/
		LAST_BASE_PHY_KIND,
	};

public:

	virtual ~MacToPhyInterface() {}
	/**
	 * @brief Returns the current state the radio is in. See RadioState
	 * for possible values.
	 *
	 * This method is mainly used by the mac layer.
	 */
	virtual int getRadioState() = 0;

	/**
	 * @brief Tells the BasePhyLayer to switch to the specified
	 * radio state.
	 *
	 * The switching process can take some time depending on the
	 * specified switching times in the ned file.
	 * The return value is the time needed to switch to the
	 * specified state, or smaller zero if the radio could
	 * not be switched (propably because it is already switching.
	 */
	virtual simtime_t setRadioState(int rs) = 0;

	/**
	 * @brief Returns the current state of the channel. See ChannelState
	 * for details.
	 */
	virtual ChannelState getChannelState() = 0;

	/**
	 * @brief Returns the length of the phy header in bits.
	 *
	 * Since the MAC layer has to create the signal for
     * a transmission it has to know the total length of
     * the packet and therefore needs the length of the
     * phy header.
	 */
	virtual int getPhyHeaderLength() = 0;
};

#endif /*MACTOPHYINTERFACE_H_*/
