/* -*- mode:c++ -*- ********************************************************
 * file:        Packet.h
 *
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2009 CSEM SA
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

#ifndef BBPACKET_H
#define BBPACKET_H

#include <omnetpp.h>
#include <Blackboard.h>
#include <sstream>

/**
 * @brief Class that keeps track of the number of packets sent.
 *
 * @ingroup blackboard
 * @author Andreas K�pke
 * @sa Blackboard
 */

class  Packet : public BBItem
{
    BBITEM_METAINFO(BBItem);

protected:
    /** @brief number of packets generated. */
    long nbPacketsReceived;
    long nbPacketsSent;
    /** @brief The size of each of the received and sent packet.*/
    long packetBitLength;
    bool sent;
    int host;
public:

    /** @brief Constructor*/
    Packet(long bitLength) :
    	BBItem(),
    	nbPacketsReceived(0), nbPacketsSent(0),
    	packetBitLength(bitLength),
    	sent(true), host(0) {
    };


    double getNbPacketsReceived() const  {
        return nbPacketsReceived;
    }

    double getNbBitsReceived() const {
		return nbPacketsReceived * packetBitLength;
	}

    double getNbPacketsSent() const  {
            return nbPacketsSent;
	}

    double getNbBitsSent() const {
    	return nbPacketsSent * packetBitLength;
    }

    void setNbPacketsReceived(int n)  {
        nbPacketsReceived = n;
    }

    void setNbPacketsSent(int n)  {
            nbPacketsSent = n;
	}

    void setBitLength(long bitLength) {
    	packetBitLength = bitLength;
    }

    long getBitLength() const {
    	return packetBitLength;
    }

    bool isSent() const {
    	return sent;
    }

    void setPacketSent(bool isSent) {
    	sent = isSent;
    }

    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " Number of packets generated is "<< 0;
        return ost.str();
    }
};


#endif
