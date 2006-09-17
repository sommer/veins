//**************************************************************************
// * file:        YourPkt.msg
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
// **************************************************************************
// * part of:     framework implementation developed by tkn
// * description: - template file for creating an own message type
// ***************************************************************************
// * changelog:   $Revision: 388 $
// *              last modified:   $Date: 2006-08-10 11:02:52 +0200 (Do, 10 Aug 2006) $
// *              by:              $Author: willkomm $
// **************************************************************************/

// Don't forget to include the header file of the basic msg
// e.g. the AirFrame
cplusplus {{
#include <AirFrame_m.h>
}};

// the class of the basic msg has to be declared:
class AirFrame;

// template file for creating own messages
//
// own .msg files have to be derived from the obligatory basic .msg files
// (see manual)
// Replace YourPkt with the name you want the msg to have and
// replace BasePkt with the name of the corresponding obligatory 
// basic message
// 
// @author Your Name 
message YourPkt extends AirFrame {
    fields:
	// Define your extra fields here....
}
