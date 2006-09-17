//****************************************************************************
// * file:        YourNetwork.ned
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


import
    "YourHost",
    "ChannelControl";

module YourNetwork
    parameters:
        // parametes for the framework
        playgroundSizeX : numeric const,
        playgroundSizeY : numeric const,
        numHosts : numeric const;
        

    submodules:
        channelcontrol: ChannelControl;
            parameters:
                playgroundSizeX = playgroundSizeX,
                playgroundSizeY = playgroundSizeY;
            display: "p=50,25;i=eye14";
        host: YourHost[numHosts];
            display: "p=50,50;i=node";

    connections nocheck:
    // all connections and gates are to be generated dynamically

    display: "p=0,0;b=$playgroundSizeX,$playgroundSizeY,rect;o=white";
endmodule


network yourSim : YourNetwork
endnetwork
