/*
 * MIxNETWorldUtility.cc
 *
 *  Created on: 09.09.2010
 *      Author: Michael Swigulski
 */

#include "MIxNETWorldUtility.h"

Define_Module(MIxNETWorldUtility);

MIxNETWorldUtility::MIxNETWorldUtility() {
	// TODO Auto-generated constructor stub

}

MIxNETWorldUtility::~MIxNETWorldUtility() {
	// TODO Auto-generated destructor stub
}

void MIxNETWorldUtility::addMACAddrPair(const MACAddress& inetAddr,
										int miximAddr)
{
	// convert INET-MAC-address to a string-representation
	std::string inetAddrString = inetAddr.str();

	// add an entry from INET-MAC-address to MiXiM-MAC-address
	inetToMiximMACAddr[inetAddrString] = miximAddr;

	// add an entry from MiXiM-MAC-address to INET-MAC-address
	miximToInetMACAddr[miximAddr] = inetAddrString;
}

int MIxNETWorldUtility::getMiximMACAddr(const MACAddress& inetAddr) const
{
	// convert INET-MAC-address to a string-representation
	std::string inetAddrString = inetAddr.str();

	// search map for an entry that matches the passed key
	std::map<std::string, int>::const_iterator it =
			inetToMiximMACAddr.find(inetAddrString);

	// if not found, return error value;
	// else return the mapped value
	if (it == inetToMiximMACAddr.end())
	{
		return 0;
	}
	else
	{
		return it->second;
	}
}
