//***************************************************************************
// * file:        GENic.ned
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
// * description: an example nic using the Gilbert-Elliot modules and csma..
// *              
// ***************************************************************************
// * changelog:   $Revision: 272 $
// *              last modified:   $Date: 2006-05-04 16:37:37 +0200 (Do, 04 Mai 2006) $
// *              by:              $Author: koepke $
// **************************************************************************/

import 
	"CSMAMacLayer",
	"ErrAndCollDecider",
	"SingleChannelRadio",
	"GilbertElliotSnr";

module GENic
    gates: 
	in: radioIn; // to receive AirFrames
        in: uppergateIn; // from upper layers
        out: uppergateOut; // to upper layers
	out: upperControlOut; // control connection

     submodules:
	mac: CSMAMacLayer;
            display: "p=60,50;b=32,30;i=prot2";
        decider: ErrAndCollDecider; 
            display: "p=68,100;b=16,16;i=box2_s;o=white";
        snrEval: GilbertElliotSnr; 
            display: "p=60,150;b=32,30;i=prot3";
	radio: SingleChannelRadio
	display: "p=200,30;b=30,25";

    connections: 
        radioIn --> snrEval.radioIn;

        decider.uppergateOut --> mac.lowergateIn display "m=m,50,50,75,0";
        snrEval.uppergateIn <-- mac.lowergateOut display "m=m,25,0,25,0";
        snrEval.uppergateOut --> decider.lowergateIn display "m=m,73,0,50,50";

	snrEval.upperControlOut --> mac.lowerControlIn;	
        mac.uppergateOut --> uppergateOut;

	mac.upperControlOut --> upperControlOut;
        mac.uppergateIn <-- uppergateIn;

    display: "p=10,10;b=101,180,rect;o=white";
endmodule 
