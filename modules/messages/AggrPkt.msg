
// This class is used by the transport layer Aggregation
// to aggregate several packets into one item
// to deliver to the routing layer.


cplusplus {{
#include "ApplPkt_m.h"
typedef ApplPkt* pApplPkt;
}} 

class noncobject pApplPkt;

message AggrPkt extends cPacket  {
  @customize(true);
  abstract pApplPkt storedPackets[];    //WARNING: omnet++ doesn't want to see pointers here so we declare long instead 
};
