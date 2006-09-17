//****************************************************************************
// * file:        YourNetwLayer.ned
// *
// * author:      Your Name
// *
// * copyright:   (C) 2004 Your Institution
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
// * changelog:   $Revision: 116 $
// *              last modified:   $Date: 2004-10-12 18:57:45 +0200 (Di, 12 Okt 2004) $
// *              by:              $Author: omfw-willkomm $
// ***************************************************************************/


simple YourNetwLayer
    parameters:
	debug: bool, // debug switch
	headerLength: numeric const; // length of the network packet header (in bits)
	// ENTER YOUR CODE HERE
    gates:
	in: uppergateIn;
	out: uppergateOut;
	in: lowergateIn;
	out: lowergateOut;
endsimple
