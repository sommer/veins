
cplusplus {{
#include "ChannelState.h"
}}



class noncobject ChannelState;

//
// The ChannelSenseRequest is a control packet from the
// MAC layer to the physical layer.
// It tells the physical layer to sense for a specific time whether
// the channel is currently busy. If the result is determined
// the physical layer stores the result inside the request and
// sends it back to the MAC layer.
//
packet ChannelSenseRequest
{
    simtime_t senseDuration; // Time to listen on the channel to determine if its busy or not.
    ChannelState result; // The result of the request containing the current state of the channel.
							 // See ChannelState for details.
}
