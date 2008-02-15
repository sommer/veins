
cplusplus {{
#include "Signal_.h"
}}

class noncobject Signal;

//
// Format of the messages that are sent to the channel
//
// subclass if you want to create your own AirFrame message class
// (see Omnet manual, chapter Messages - Inheritance among message 
// classes).
//
// This message format is used to send a packet from the physical
// layer to the channel. All other physical layers that can 'hear'
// this message can evaluate noise, interference etc. from the
// information contained in this message:
//
// If you need more fields for whatever reason, please do NOT create
// your own message! Just extend (subclass) this message format 
//
message AirFrame
{
//	properties:
//		customize = true;
		
    fields:
//      abstract Signal signal;    	// Contains the physical data of this AirFrame
        
        Signal signal;		// Contains the physical data of this AirFrame
        
        int state = 0; 		// state of the AirFrames, used by the physical layer
        					// as state machine for delay and transmission duration
        					// simulation
        					
        double duration; 	// Time it takes to transmit the packet, in seconds!
        
        int type = 0;		// If type isn't null then this is a control-AirFrame
        					// and type specifies the control type.
	
		long id;			// Unique ID of the AirFrame used as identifier for
							// related control-AirFrames
};