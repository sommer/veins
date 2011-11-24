package org.mixim.modules.netw;
//***************************************************************************
// file:        DummyRoute.ned
//
// author:      Jerome Rousselot
// 
// copyright:   (C) 2009 CSEM SA, Neuchatel, Switzerland.
//
// description:  Adaptor module that simply "translates" netwControlInfo to macControlInfo
//				 and filters packets according to their network ID.
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// *
// **************************************************************************/

import org.mixim.base.modules.BaseNetwLayer;

// Placeholder module that simply "translates" netwControlInfo to macControlInfo.
simple DummyRoute extends BaseNetwLayer
{
    parameters:
        // debug switch
        bool trace = default(false);
        stats = default(true);
        headerLength = default(0);
        int networkID = default(0);  // network ID, allows coexistence tests
        @class(DummyRoute);
}

