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

#pragma once

#include "veins/veins.h"

#include "veins/base/utils/SimpleAddress.h"

namespace Veins {

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
class VEINS_API MacToNetwControlInfo : public cObject {
protected:
    /** @brief The bit error rate for this packet.*/
    double bitErrorRate;

    /** @brief MAC address of the last hop of this packet.*/
    LAddress::L2Type lastHopMac;

    /** @brief The received signal strength for this packet.*/
    double rssi;

public:
    /**
     * @brief Initializes with the passed last hop address and bit error rate.
     */
    MacToNetwControlInfo(const LAddress::L2Type& lastHop, double ber = 0, double rssi = 0)
        : bitErrorRate(ber)
        , lastHopMac(lastHop)
        , rssi(rssi)
    {
    }

    ~MacToNetwControlInfo() override
    {
    }

    /**
     * @brief Returns the bit error rate for this packet.
     */
    double getBitErrorRate() const
    {
        return bitErrorRate;
    }

    /**
     * @brief Sets the bit error rate for this control infos packet.
     *
     * @param ber The bit error rate
     */
    virtual void setBitErrorRate(double ber)
    {
        bitErrorRate = ber;
    }

    /**
     * @brief Returns the MAC address of the packets last hop.
     */
    const LAddress::L2Type& getLastHopMac() const
    {
        return lastHopMac;
    }

    /**
     * @brief Sets the MAC address of the packets last hop.
     *
     * @param lastHop The last hops MAC address
     */
    virtual void setLastHopMac(const LAddress::L2Type& lastHop)
    {
        lastHopMac = lastHop;
    }

    /**
     * @brief Returns the packets received signal strength.
     *
     * @return The received signal strength
     */
    virtual const double getRSSI()
    {
        return rssi;
    }

    /**
     * @brief Sets the packets received signal strength.
     * @param _rssi The received signal strength
     */
    void setRSSI(double _rssi)
    {
        rssi = _rssi;
    }

    /**
     * @brief Attaches a "control info" structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the source MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg        The message where the "control info" shall be attached.
     * @param pSrcAddr    The MAC address of the message sender.
     */
    static cObject* const setControlInfo(cMessage* const pMsg, const LAddress::L2Type& pSrcAddr)
    {
        MacToNetwControlInfo* const cCtrlInfo = new MacToNetwControlInfo(pSrcAddr);
        pMsg->setControlInfo(cCtrlInfo);

        return cCtrlInfo;
    }
    static const LAddress::L2Type& getAddress(cMessage* const pMsg)
    {
        return getAddressFromControlInfo(pMsg->getControlInfo());
    }
    static const LAddress::L2Type& getAddressFromControlInfo(cObject* const pCtrlInfo)
    {
        MacToNetwControlInfo* const cCtrlInfo = dynamic_cast<MacToNetwControlInfo* const>(pCtrlInfo);

        if (cCtrlInfo) return cCtrlInfo->getLastHopMac();

        return LAddress::L2NULL();
    }
};

} // namespace Veins
