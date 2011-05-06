/*
 * RadioDetailed.h
 * Author: Jerome Rousselot <jerome.rousselot@csem.ch>
 * Copyright: (C) 2011 CSEM SA
 * Wireless Embedded Systems
 * Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 */


#ifndef RADIODETAILED_H_
#define RADIODETAILED_H_

#include "PhyUtils.h"
#include "BasePhyLayer.h"

/**
 * @brief This class extends the basic radio model to provide a more detailed FSM, and to support multichannel operation.
 *
 * This detailed radio model adds a RADIO_ON state for the radio model, and supports multiple orthogonal channels.
 * Radios must often switch as follow: sleep -> radio_on -> rx.
 *
 *
 * @ingroup ieee802154
 * @ingroup phyLayer
 */

class RadioDetailed: public Radio {

private:

public:

	enum RadioDetailedStates {
		/* receiving state*/
		 ON = Radio::NUM_RADIO_STATES,
		 DETAILED_NUM_RADIO_STATES
	};

	/* Static factory method (see Radio class in PhyUtils.h) */
	static RadioDetailed* createNewRadioDetailed(int initialState,
								 bool recordStats,
								 double minAtt = 1.0,
								 double maxAtt = 0.0, int currentChannel=0, int nbChannels=1)
	{
		return new RadioDetailed(RadioDetailed::DETAILED_NUM_RADIO_STATES,
						 recordStats,
						 initialState,
						 minAtt, maxAtt, currentChannel, nbChannels);
	}

protected:

	RadioDetailed(int numRadioStates,bool recordStats, int initialState, double minAtt = 1.0, double maxAtt = 0.0,
			int currentChannel = 0, int nbChannels = 1)
	:Radio(numRadioStates, recordStats, initialState, minAtt, maxAtt, currentChannel, nbChannels) {	}

	virtual double mapStateToAtt(int state)
	{
		if(state == RadioDetailed::ON) {
			return minAtt;
		} else {
			return Radio::mapStateToAtt(state);
		}
	}
};

#endif /* RADIODETAILED_H_ */
