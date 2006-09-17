//****************************************************************************
// * file:        YourApplLayer.ned
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


simple YourApplLayer
    parameters:
	debug: bool, // debug switch
	headerLength: numeric const; // length of the application message header (in bits)
	// ENTER YOUR CODE HERE
    gates:
	in: lowergateIn,
	    lowerControlIn;
	out: lowergateOut;
endsimple