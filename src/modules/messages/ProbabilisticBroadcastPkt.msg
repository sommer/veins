

cplusplus {{
#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message ProbabilisticBroadcastPkt extends NetwPkt
{
    long             id;    // Unique message ID ~ f(node address, unique number in node scope).
    simtime_t        appTtl;   // Message time-to-live set by application.
    LAddress::L3Type initialSrcAddr;
    LAddress::L3Type finalDestAddr;
    int              nbHops;
}
