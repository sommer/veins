
cplusplus {{
#include "ChannelState.h"
}}




class noncobject ChannelState;

enum SenseMode {
    UNTIL_IDLE = 1;		//sense until channel is busy or timeout is reached
    UNTIL_BUSY = 2;		//sense until channel is idle or timeout is reached
    UNTIL_TIMEOUT = 3;	//sense until timeout is reached
};
    
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
    int senseMode enum(SenseMode);	// defines the sensing mode (see SenseMode enum)
    simtime_t senseTimeout; 		// Time to listen on the channel to determine if its busy or not.
    ChannelState result; 			// The result of the request containing the current state of the channel.
									// See ChannelState for details.
}
