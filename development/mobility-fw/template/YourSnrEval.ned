//****************************************************************************
// * file:        YourSnrEval.ned
// *
// * author:      Your name
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
// * changelog:   $Revision: 388 $
// *              last modified:   $Date: 2006-08-10 11:02:52 +0200 (Do, 10 Aug 2006) $
// *              by:              $Author: willkomm $
// ***************************************************************************/


simple YourSnrEval	
     parameters:		
	debug : bool, // debug switch
	headerLength: numeric const;
    gates: 
        in: uppergateIn, radioIn;
        out: uppergateOut, upperControlOut;
endsimple