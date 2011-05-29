// A LMAC packet format definition
// 
// @author Anna Foerster


cplusplus {{
#include <MacPkt_m.h>
}}


class MacPkt;


message LMacPkt extends MacPkt
{
    int mySlot; // the node's current slot number
    short occupiedSlots[]; // currently known occupied slots
}
