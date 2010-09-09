/*
 * MIxNETWorldUtility.h
 *
 *  Created on: 09.09.2010
 *      Author: Michael Swigulski
 */

#ifndef MIXNETWORLDUTILITY_H_
#define MIXNETWORLDUTILITY_H_

#include <BaseWorldUtility.h>
#include <MACAddress.h>

class MIxNETWorldUtility: public BaseWorldUtility {
public:
	MIxNETWorldUtility();
	virtual ~MIxNETWorldUtility();

	virtual void addMACAddrPair(const MACAddress& inetAddr, int miximAddr);

	virtual int getMiximMACAddr(const MACAddress& inetAddr);
};

#endif /* MIXNETWORLDUTILITY_H_ */
