

cplusplus {{
#include <NetwPkt_m.h>
}}



class NetwPkt;



message WiseRoutePkt extends NetwPkt
{
    int isFlood;
	int finalDestAddr;   
 	int initialSrcAddr;
}
