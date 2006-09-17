//***************************************************************************
// * file:        TestDecider.ned
// *
// * author:      Daniel Willkomm
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
// **************************************************************************/


//
// Test Decider module to test core functionality
//
simple TestDecider
    parameters:
       coreDebug : bool; // debug switch
    gates:
        out: uppergateOut;
        in: lowergateIn;
endsimple
