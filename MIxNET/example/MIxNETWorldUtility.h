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
#include <string>
#include <map>

class MIxNETWorldUtility: public BaseWorldUtility {
protected:

	/**
	 * @brief Mapping from INET-MAC-addresses to MiXiM-MAC-addresses
	 */
	std::map<std::string, int> inetToMiximMACAddr;

	/**
	 * @brief Mapping from MiXiM-MAC-addresses to INET-MAC-addresses
	 */
	std::map<int, std::string> miximToInetMACAddr;


public:
	MIxNETWorldUtility();
	virtual ~MIxNETWorldUtility();

	/**
	 * @brief Stores a pair of corresponding MAC-addresses
	 */
	virtual void addMACAddrPair(const MACAddress& inetAddr, int miximAddr);

	/**
	 * @brief Returns the MiXiM-MAC-address that corresponds to the passed
	 * INET-MAC-address
	 *
	 * @return Returns the corresponding MiXiM-MAC-address if an entry to
	 * the passed INET-MAC-address exists, else returns an error value.
	 */
	virtual int getMiximMACAddr(const MACAddress& inetAddr) const;
};

#endif /* MIXNETWORLDUTILITY_H_ */
