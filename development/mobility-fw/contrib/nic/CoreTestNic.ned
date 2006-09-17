//***************************************************************************
//* file:        CoreTestNic.ned
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
//* description:  basic network interace card Compound Module
//*
//***************************************************************************

import  
    "AlohaMacLayer", 
    "TestDecider", 
    "SingleChannelRadio",
    "TestSnrEval"; 
                    
 
// The nic (network interface card) is a compound module that contains the 
// MAC and PHY of a host within the Mobility Framework
//
// The coreTesdNic is just for core testing purposes
//
// IMPORTANT:<br> In order to work with the ChannelControl module the snrEval
// module has to be called "snrEval" in the ned file.
//
// @author Daniel Willkomm
module CoreTestNic 
    gates: 
        in: uppergateIn; // to upper layers
        out: uppergateOut, // from upper layers
             upperControlOut; // control information 
	in: radioIn; // radioIn gate for sendDirect
    submodules: 
        mac: AlohaMacLayer; 
            display: "p=60,50;b=32,30;i=prot2";
        decider: TestDecider; 
            display: "p=68,100;b=16,16;i=box2_s;o=white";
        snrEval: TestSnrEval; 
            display: "p=60,150;b=32,30;i=prot3";
	radio: SingleChannelRadio;
	    display: "p=90,50;b=32,30;o=red";
    
    connections: 
        decider.uppergateOut --> mac.lowergateIn display "m=m,50,50,75,0"; 
        snrEval.uppergateIn <-- mac.lowergateOut display "m=m,25,0,25,0"; 
        snrEval.uppergateOut --> decider.lowergateIn display "m=m,73,0,50,50";                                                                        
	snrEval.upperControlOut --> mac.lowerControlIn;
        mac.uppergateOut --> uppergateOut; 
	mac.upperControlOut --> upperControlOut;	   
        mac.uppergateIn <-- uppergateIn; 
	
	radioIn --> snrEval.radioIn;
                                         
    display: "p=10,10;b=101,180,rect;o=white";
endmodule 

