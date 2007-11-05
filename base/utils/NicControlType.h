/* -*- mode:c++ -*- ********************************************************
 * file:        NicControlType.h
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

#ifndef NICCONTROLTYPE_H
#define NICCONTROLTYPE_H

#include <omnetpp.h>

/**
 * @brief Enumerate possible control messages
 *
 * Enumerate for the kind field for control messages exchanged within
 * a NIC
 *
 * @ingroup basicUtils
 * @ingroup utils
 * @author Andreas Koepke
 */

class NicControlType : public cPolymorphic
{    
 public:
    enum Types {
        NOTHING=1, // we don't want to start at zero -- it is ambiguous

		RX_START,
		RX_HDR,
		RX_FAIL,

		TX_END, // this is TRANSMISSION_OVER replacement
		TX_FAIL,

		SET_TRANSMIT,
		SET_LISTEN,
		SET_SLEEP,

		SET_RSSI,
    };
    

    /** @brief Enables inspection
     *
     * If you want to know the description of a certain number, make
     * this object out of the number and call info. 
     * 
     * @{ */ 
    NicControlType(Types t=NOTHING) : cPolymorphic(), type(t) {
        ;
    };
    
    int getType() const  {
        return type;
    }
    
    void setType(Types t) {
        type = t;
    }
    
    std::string info() const {
        std::ostringstream ost;
		switch(type)
		{
			case NOTHING:
	        	ost<<"NOTHING";
				break;

    		case RX_START:
    			ost<<"RX_START";
				break;
			case RX_HDR:
				ost<<"RX_HDR";
				break;
			case RX_FAIL:
				ost<<"RX_FAIL";
				break;

			case TX_END:
				ost<<"TX_END";
				break;
			case TX_FAIL:
				ost<<"TX_FAIL";
				break;

			case SET_TRANSMIT:
				ost<<"SET_TRANSMIT";
				break;
			case SET_LISTEN:
				ost<<"SET_LISTEN";
				break;
			case SET_SLEEP:
				ost<<"SET_SLEEP";
				break;

			case SET_RSSI:
				ost<<"SET_RSSI";
				break;
			
		}
        return ost.str();
    }
    /* @} */

protected:
    Types type;

};

#endif
