/* -*- mode:c++ -*- ********************************************************
 * file:        SimpleAddress.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#ifndef SIMPLEADDRESS_H
#define SIMPLEADDRESS_H

#include <omnetpp.h>

#include "veins/base/utils/MiXiMDefs.h"

/** @brief Layer address handling helper function.
 *
 * This class helps to make the layer addressing more generic. The type definition
 * for L2, and L3 addresses should be used for address definitions to simplify the
 * switching to other types.
 *
 * @ATTENTION Both types (LAddress::L2Type, LAddress::L3Type) will be used as keys in
 *            in STL containers.
 */
class MIXIM_API LAddress {
public:
    /** @brief Type definition for a L2 (MAC) address.
     *
     * The type should support initialization with long/string values and casting to long/double.
     * The type should be also support the ==,<, and > operators.
     */
    typedef long L2Type;
    /** @brief Type definition for a L3 (Network) address.
     *
     * The type should support initialization with long values and casting to long/double.
     * The type should be also support the ==,<, and > operators.
     */
    typedef long L3Type;

    /** @brief Broadcast address for L2 addresses. */
    static const L2Type& L2BROADCAST() {
        static L2Type o(-1);
	return o;
    }
    /** @brief NULL address for L2 addresses. */
    static const L2Type& L2NULL() {
        static L2Type o(0);
	return o;
    }
    /** @brief Broadcast address for L3 addresses. */
    static const L3Type& L3BROADCAST() {
        static L3Type o(-1);
	return o;
    }
    /** @brief NULL address for L3 addresses. */
    static const L3Type& L3NULL() {
        static L3Type o(0);
	return o;
    }
public:
    /**
     * @brief Test if a L2 address (pSrcAddr) is a broadcast address.
     *
     * @param  pSrcAddr The L2 address which should be tested.
     * @return True if pSrcAddr is a braodcast address.
     */
    static inline bool isL2Broadcast(const L2Type& pSrcAddr) {
        return pSrcAddr == L2BROADCAST();
    }
    /**
     * @brief Test if a L3 address (pSrcAddr) is a broadcast address.
     *
     * @param  pSrcAddr The L3 address which should be tested.
     * @return True if pSrcAddr is a braodcast address.
     */
    static inline bool isL3Broadcast(const L3Type& pSrcAddr) {
        return pSrcAddr == L3BROADCAST();
    }
};

#endif
