//***************************************************************************
// * file:        SnrEval.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
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
// * description: class for the snrEval module, keeps track of the 
// *              snir levels during a packet reception
// ***************************************************************************
// * changelog:   $Revision: 302 $
// *              last modified:   $Date: 2006-06-09 17:54:47 +0200 (Fr, 09 Jun 2006) $
// *              by:              $Author: koepke $
// **************************************************************************/


simple SnrEval
    parameters:
	debug : bool, // debug switch
 	// if false, the RSSI will not be published 
        // during the reception of a frame
	publishRSSIAlways : bool, 
        transmitterPower : numeric,
	headerLength: numeric const,
	carrierFrequency: numeric,
	thermalNoise: numeric,
	pathLossAlpha: numeric,
	sensitivity: numeric;
    gates: 
        in: uppergateIn, radioIn;
        out: uppergateOut, upperControlOut;
endsimple
