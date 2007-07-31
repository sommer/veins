// - mode:c++ -//- //////////////////////////////////////////////////////////
 // file:        UnitDisk.ned
 //
 // author:      Tom Parker
 //
 // copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 //              Technische Universiteit Delft, The Netherlands.
 //
 //              This program is free software; you can redistribute it 
 //              and/or modify it under the terms of the GNU General Public 
 //              License as published by the Free Software Foundation; either
 //              version 2 of the License, or (at your option) any later 
 //              version.
 //              For further information see file COPYING 
 //              in the top level directory
 /////////////////////////////////////////////////////////////////////////////
 // description: propagation layer: unit disk model
 ////////////////////////////////////////////////////////////////////////////

simple UnitDisk
    parameters:
		debug		  : numeric const, // debug switch
		radioRange 	  : numeric const,

		// below are unused here, but needed for BasePhyLayer
		pMax			  : numeric const, // maximum sending power used for this network (in mW)
		sat			  : numeric const, // minimum signal attenuation threshold (in dBm)
		alpha			  : numeric const, // minimum path loss coefficient
		carrierFrequency	  : numeric const; // minimum carrier frequency of the channel (in Hz)
endsimple

