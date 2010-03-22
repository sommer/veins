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

#ifndef MACTONETWCONTROLINFO_H_
#define MACTONETWCONTROLINFO_H_

#include <omnetpp.h>

/**
 * @brief Stores control information from mac to upper layer.
 *
 * Holds the bit error rate of the transmission as well as the
 * MAC address of the last hop.
 *
 * @ingroup baseUtils
 * @ingroup macLayer
 * @ingroup netwLayer
 *
 * @author Karl Wessel
 */
class MacToNetwControlInfo : public cObject {
protected:
	/** @brief The bit error rate for this packet.*/
	double bitErrorRate;

	/** @brief MAC address of the last hop of this packet.*/
	long lastHopMac;

public:
	/**
	 * @brief Initializes with the passed last hop address and bit error rate.
	 */
	MacToNetwControlInfo(long lastHop, double ber):
		bitErrorRate(ber),
		lastHopMac(lastHop)
	{}

	virtual ~MacToNetwControlInfo() {}

	/**
	 * @brief Returns the bit error rate for this packet.
	 */
	double getBitErrorRate() const
    {
        return bitErrorRate;
    }

	/**
	 * @brief Returns the MAC address of the packets last hop.
	 */
	long getLastHopMac() const {
		return lastHopMac;
	}
};

#endif /* MACTONETWCONTROLINFO_H_ */
