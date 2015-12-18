/* -*- mode:c++ -*- */

#ifndef PASSED_MESSAGE_H
#define PASSED_MESSAGE_H

#include <omnetpp.h>

#include "veins/base/utils/MiXiMDefs.h"

class MIXIM_API PassedMessage : public cObject {
 public:
    enum gates_t {
        UPPER_DATA,
	UPPER_CONTROL,
	LOWER_DATA,
	LOWER_CONTROL
    };

    enum direction_t {
        INCOMING,
        OUTGOING
    };
    
 public:
    static const char *gateToString(gates_t g) {
        const char *s;
        switch(g) {
        case UPPER_DATA: s = "UPPER_DATA"; break;
        case UPPER_CONTROL: s = "UPPER_CONTROL"; break;
        case LOWER_DATA: s = "LOWER_DATA"; break;
        case LOWER_CONTROL: s = "LOWER_CONTROL"; break;
        default:
            throw cRuntimeError("PassedMessage::gateToString: got invalid value");
            s = 0;
            break;
        }
        return s;
    }
    
 public:
    // meta information 
    int fromModule;
    gates_t gateType;
    direction_t direction;

    // message information
    int kind;
    const char* name;
};

#endif

