/*
 * Decider80211pToPhy80211pInterface.h
 *
 *  Created on: Mar 28, 2012
 *      Author: eckhoff
 */

#ifndef DECIDER80211PTOPHY80211PINTERFACE_H_
#define DECIDER80211PTOPHY80211PINTERFACE_H_

class Decider80211pToPhy80211pInterface {
public:
	virtual ~Decider80211pToPhy80211pInterface() {};
	virtual int getRadioState()=0;
};

#endif /* DECIDER80211PTOPHY80211PINTERFACE_H_ */
