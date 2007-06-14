//opp_msgc -I/home/baggio/TUD/Soft/mixim/trunk/base/messages/ PositionPkt.msg

cplusplus {{
#include "LocPkt_m.h"
}};
class LocPkt;

message PositionPkt extends LocPkt
{
    fields:
		int id;
		double xPos;
		double yPos;
		double zPos;
};
