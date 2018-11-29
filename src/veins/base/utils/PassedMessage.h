/* -*- mode:c++ -*- */

#pragma once

#include "veins/veins.h"

namespace Veins {

class VEINS_API PassedMessage : public cObject {
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
    static const char* gateToString(gates_t g)
    {
        const char* s;
        switch (g) {
        case UPPER_DATA:
            s = "UPPER_DATA";
            break;
        case UPPER_CONTROL:
            s = "UPPER_CONTROL";
            break;
        case LOWER_DATA:
            s = "LOWER_DATA";
            break;
        case LOWER_CONTROL:
            s = "LOWER_CONTROL";
            break;
        default:
            throw cRuntimeError("PassedMessage::gateToString: got invalid value");
            s = nullptr;
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

} // namespace Veins
