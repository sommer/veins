//***************************************************************************
//* file:        AlohaSnrNic.ned
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
//* description:  snr network interace card Compound Module
//*
//***************************************************************************
//* changelog:   $Revision: 279 $
//*              last modified:   $Date: 2006-05-08 14:18:06 +0200 (Mo, 08 Mai 2006) $
//*              by:              $Author: koepke $
//***************************************************************************

import
    "AlohaMacLayer",
    "SnrDecider",
    "SingleChannelRadio",
    "SnrEval";


// This is the easiest nic to implement "real" network behaviour. It uses
// the CsmaMacLayer and the SnrDecider and SnrEval modules.
//
// The csma mac provides basic medium access and is used here to prevent a
// host from trying to send a message while it is still in receive mode.
//
// The phy layer modules allow to account for collision and take fading into
// account so that messages can actually be lost.
//
// IMPORTANT:<br> In order to work with the ChannelControl module the snrEval
// module has to be called "snrEval" in the ned file.
//
// @see BasicMacLayer, SnrDecider, SnrEval
//
// @author Daniel Willkomm
module AlohaSnrNic
    gates:
        in: uppergateIn; // from upper layers
        out: uppergateOut; // to upper layers
	out: upperControlOut; // control connection
	in: radioIn; // to receive AirFrames

    submodules:
        mac: AlohaMacLayer;
            display: "p=60,50;b=32,30;i=prot2";
        decider: SnrDecider;
            display: "p=68,100;b=16,16;i=box2_s;o=white";
        snrEval: SnrEval;
            display: "p=60,150;b=32,30;i=prot3";
	radio: SingleChannelRadio;
            display: "p=90,50;b=32,30;o=red";

    connections:
        radioIn --> snrEval.radioIn;
        decider.uppergateOut --> mac.lowergateIn display "m=m,50,50,75,0";
        snrEval.uppergateIn <-- mac.lowergateOut display "m=m,25,0,25,0";
        snrEval.uppergateOut --> decider.lowergateIn display "m=m,73,0,50,50";

	snrEval.upperControlOut --> mac.lowerControlIn;	
        mac.uppergateOut --> uppergateOut;

	mac.upperControlOut --> upperControlOut;
        mac.uppergateIn <-- uppergateIn;

endmodule

