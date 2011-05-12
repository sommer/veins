

cplusplus {{
#include <NetwPkt_m.h>
}}


class NetwPkt;


message ProbabilisticBroadcastPkt extends NetwPkt
{
    unsigned int id;    // Unique message ID ~ f(node address, unique number in node scope).
    simtime_t appTtl;   // Message time-to-live set by application.
    double criticality; // Message criticality in range [0, 1], set by application.
    int initialSrcAddr;
    int finalDestAddr;

}
