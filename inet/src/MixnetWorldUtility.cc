//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "MixnetWorldUtility.h"

Define_Module(MixnetWorldUtility);

const int MixnetWorldUtility::NoMacPairFound = -242;

void MixnetWorldUtility::addMACAddrPair(const MACAddress& inetAddr,
										int miximAddr)
{
	//Normally the MiXiM-address should never be smaller than -1 since it uses
	//module-ids as address. But in case we were wrong, we check and give a
	//warning at this point if the passed MiXiM-address is the same as the value
	//used as error value for "NoMacPairFound"
	if(miximAddr == NoMacPairFound) {
		opp_warning("Added an address pair whose MiXiM-address has the same "
					"value as the error value \"NoMacPairFound\". This either "
					"means you are using another addressing scheme for MiXiM "
					"than the default one or OMNeT++ has changed the way it "
					"assigns module ids.");
	}

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
	// add an entry from INET MAC-address to MiXiM MAC-address
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
	// add an entry from MiXiM MAC-address to INET MAC-address
	miximToInetMACAddr[miximAddr] = inetAddr;
}

int MixnetWorldUtility::getMiximMACAddr(const MACAddress& inetAddr) const
{
	// convert INET MAC-address to a string-representation
	std::string inetAddrString = inetAddr.str();

	// search map for an entry that matches the passed key
	INETToMiximMACMap::const_iterator it =
			inetToMiximMACAddr.find(inetAddr);

	// if not found, return error value;
	// else return the mapped value
	if (it == inetToMiximMACAddr.end())
	{
		return NoMacPairFound;
	}
	else
	{
		return it->second;
	}
}
