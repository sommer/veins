//**************************************************************************
// * file:        UWBIRMacPkt.msg
// *
// * author:      Jerome Rousselot
// *
// * copyright:   (C) 2008 CSEM SA, Neuchatel, Switzerland.
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// **************************************************************************
// * part of:     framework implementation developed by tkn
// * description: - 
// **************************************************************************/



cplusplus {{
#include <MacPkt_m.h>
}}


class MacPkt;


message UWBIRMacPkt extends MacPkt
{
    @customize(true);
    abstract bool bitValues[];
    int nbSymbols;
}
