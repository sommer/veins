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

#ifndef MIXNETWORLDUTILITY_H_
#define MIXNETWORLDUTILITY_H_

#include <BaseWorldUtility.h>
#include <MACAddress.h>
#include <string>
#include <map>

/**
 * @brief INET <-> MiXiM compatibility module. Stores mapping between
 * INET MAC-addresses and MiXiM MAC-addresses.
 *
 * This module is needed for MixnetBridge to work. Every bridge module
 * registers its own INET <-> MiXiM MAC-address pair with it so other
 * bridge modules can use this mapping to convert next hop MAC-addresses
 * in control info of packets sent between INET network-layer and MiXiM NIC.
 *
 * @see MixnetBridge for more details.
 *
 * @ingroup blackboard
 * @ingroup mixnet
 *
 * @author Michael Swigulski, Karl Wessel
 */
class MixnetWorldUtility: public BaseWorldUtility {
public:
	/**
	 * @brief Provides less-comparison between values of type MACAddress.
	 *
	 * Used for std::maps with MACAddress as keys.
	 */
	struct LessMACAddress {
		bool operator() (const MACAddress& lhs, const MACAddress& rhs) const {
			return lhs.compareTo(rhs) < 0;
		}
	};

	/** @brief Defines the error value returned when no corresponding
	 * MAC-address for a passed MAC-address was found.
	 */
	static const int NoMacPairFound;

protected:

	typedef std::map<MACAddress, int, LessMACAddress> INETToMiximMACMap;
	/**
	 * @brief Mapping from INET MAC-addresses to MiXiM MAC-addresses
	 */
	INETToMiximMACMap inetToMiximMACAddr;

	typedef std::map<int, MACAddress> MiximToINETMACMap;
	/**
	 * @brief Mapping from MiXiM MAC-addresses to INET MAC-addresses
	 */
	MiximToINETMACMap miximToInetMACAddr;


public:

	/**
	 * @brief Stores a pair of corresponding INET and MiXiM MAC-addresses.
	 *
	 * @param inetAddr INET's MAC-address for a NIC
	 * @param miximAddr MiXiM's MAC-address for the same NIC
	 */
	virtual void addMACAddrPair(const MACAddress& inetAddr, int miximAddr);

	/**
	 * @brief Returns the MiXiM MAC-address that corresponds to the passed
	 * INET MAC-address
	 *
	 * @return Returns the corresponding MiXiM MAC-address if an entry to
	 * the passed INET MAC-address exists, else returns
	 * MixnetWorldUtility::NoMacPairFound.
	 */
	virtual int getMiximMACAddr(const MACAddress& inetAddr) const;
};

#endif /* MIXNETWORLDUTILITY_H_ */
