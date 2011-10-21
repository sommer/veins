//***************************************************************************
//* file:        BasicHost.ned
//*
//* author:      Daniel Willkomm
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: BasicHost Compound Module
//*
//***************************************************************************
//* changelog:   $Revision: 327 $
//*              last modified:   $Date: 2006-06-23 21:56:51 +0200 (Fr, 23 Jun 2006) $
//*              by:              $Author: koepke $
//***************************************************************************


//***************************************************************************
//*
//* edited by Michael Swigulski for testing purposes
//*
//***************************************************************************

package org.mixim.tests.baseMobility;

module TestHost

{
    parameters:
        double numHosts; 		// total number of hosts in the network
        double borderPolicy; 	// this hosts border policy


    submodules:
        mobility: BaseMobilityTest {
            parameters:
                borderPolicy = borderPolicy;
                @display("p=130,130;i=cogwheel2");

        }
}

