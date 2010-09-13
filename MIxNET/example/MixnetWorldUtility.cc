/*
 * MixnetWorldUtility.cc
 *
 *  Created on: 09.09.2010
 *      Author: Michael Swigulski
 */

#include "MixnetWorldUtility.h"

Define_Module(MixnetWorldUtility);

void MixnetWorldUtility::addMACAddrPair(const MACAddress& inetAddr,
										int miximAddr)
{
	// convert INET-MAC-address to a string-representation
	std::string inetAddrString = inetAddr.str();

	//check if mapping for this INET-address already exists
	INETToMiximMACMap::const_iterator it =
			inetToMiximMACAddr.find(inetAddr);
	if(it != inetToMiximMACAddr.end() && it->second != miximAddr)
	{
		opp_warning("Overwriting existing mapping for INET MAC %s to "
					"MiXiM MAC %d with MiXiM MAC %d!",
					inetAddr.str().c_str(),
					it->second,
					miximAddr);
	}
	// add an entry from INET-MAC-address to MiXiM-MAC-address
	inetToMiximMACAddr[inetAddr] = miximAddr;

	//check if mapping for this MiXiM-address already exists
	MiximToINETMACMap::const_iterator it2 =
			miximToInetMACAddr.find(miximAddr);
	if(it2 != miximToInetMACAddr.end() && it2->second != inetAddr)
	{
		opp_warning("Overwriting existing mapping for MiXiM MAC %d to "
					"INET MAC %s with INET MAC %s!",
					miximAddr,
					it2->second.str().c_str(),
					inetAddr.str().c_str());
	}
	// add an entry from MiXiM-MAC-address to INET-MAC-address
	miximToInetMACAddr[miximAddr] = inetAddr;
}

int MixnetWorldUtility::getMiximMACAddr(const MACAddress& inetAddr) const
{
	// convert INET-MAC-address to a string-representation
	std::string inetAddrString = inetAddr.str();

	// search map for an entry that matches the passed key
	INETToMiximMACMap::const_iterator it =
			inetToMiximMACAddr.find(inetAddr);

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
