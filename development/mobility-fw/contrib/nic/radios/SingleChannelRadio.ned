//***************************************************************************
// * file:        SingleChannelRadio.ned
// *
// * author:      Andreas Koepke
// *
// * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/

simple SingleChannelRadio
    parameters:
        debug : bool,
	// time parameters, how long does it take to 
        // switch to sleep mode
	swSleep : numeric const,
        // switch to send mode
	swSend : numeric const,
        // switch to recv mode
        swRecv : numeric const;
endsimple
