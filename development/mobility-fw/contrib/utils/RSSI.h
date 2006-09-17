/* -*- mode:c++ -*- ********************************************************
 * file:        RSSI.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
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

#ifndef RSSI_H
#define RSSI_H

#include <omnetpp.h>
#include <Blackboard.h>
#include <sstream>

/**
 * @brief Class that keeps track of the current RSSI value
 *        (recived signal strength indicator)
 *
 * @ingroup utils
 * @ingroup blackboard
 * @author Andreas Köpke
 * @sa Blackboard
 */

class  RSSI : public BBItem
{
    BBITEM_METAINFO(BBItem);

protected:
    /** @brief rssi value */
    double rssi;
    
public:

    /** @brief Constructor*/
    RSSI(double r=0) : BBItem(), rssi(r) {
    };

    /** @brief get current rssi */
    double getRSSI() const  {
        return rssi;
    }
    
    /** @brief set rssi */
    void setRSSI(double r) {
        rssi = r;
    }
    
    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " rssi level is "<<rssi<<" mW";
        return ost.str();
    }
};


#endif
