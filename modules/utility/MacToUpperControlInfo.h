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

#ifndef MACTOUPPERCONTROLINFO_H_
#define MACTOUPPERCONTROLINFO_H_

#include <omnetpp.h>

// TODO consider that class MacControlInfo will be separated into two classes (one for each direction)
/**
 * @brief Stores control information from mac to upper layer.
 *
 * Holds the bit error rate of the transmission.
 *
 * @ingroup macLayer
 */
class MacToUpperControlInfo : public cObject {
protected:
	/** @brief The bit error rate for this packet.*/
	double bitErrorRate;

public:
	/**
	 * @brief Initializes with the passed bit error rate.
	 */
	MacToUpperControlInfo(double ber):
		bitErrorRate(ber)
	{}

	virtual ~MacToUpperControlInfo() {}

	/**
	 * @brief Returns the bit error rate for this packet.
	 */
	double getBitErrorRate() const
    {
        return bitErrorRate;
    }
};

#endif /* MACTOUPPERCONTROLINFO_H_ */
