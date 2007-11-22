#ifndef SPECIAL_ADDRESS_H
#define SPECIAL_ADDRESS_H

#include <SimpleAddress.h>

// Routing protocols that want 'special' addresses (i.e. representing a 'type' of
// node rather than a specific address of one) should add them to this list. All
// of the chosen numbers should be negative, not -1 (which is defined in SimpleAddress.h
// as the broadcast one) and unique within this file.

#define SINK_ADDRESS -2

#endif
