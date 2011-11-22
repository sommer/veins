

cplusplus {{
#include "NetwPkt_m.h"
}}

class NetwPkt;
class LAddress::L3Type extends void;

message WiseRoutePkt extends NetwPkt
{
	int              isFlood;
	LAddress::L3Type finalDestAddr;   
 	LAddress::L3Type initialSrcAddr;
 	int              nbHops;
}
