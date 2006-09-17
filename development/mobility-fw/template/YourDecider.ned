//***************************************************************************
// * file:        YourDecider.ned
// *
// * author:      Your Name
// *
// * copyright:   (C) 2004 Your Institute
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// ***************************************************************************
// * part of:     Your Simulation
// * description: - Your Description
// *
// ***************************************************************************
// * changelog:   $Revision: 382 $
// *              last modified:   $Date: 2006-07-31 16:10:24 +0200 (Mo, 31 Jul 2006) $
// *              by:              $Author: koepke $
// **************************************************************************/


simple YourDecider
    parameters:
        debug : bool, // debug switch
	// extend this by the variables that you need.
        snrThresholdLevel: numeric;
    gates:
        out: uppergateOut;
        in: lowergateIn;
endsimple