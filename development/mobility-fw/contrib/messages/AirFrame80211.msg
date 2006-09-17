//**************************************************************************
// * file:        AirFrame80211.msg
// *
// * author:      Andreas Koepke
// *
// * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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
// In 802.11 the PHY layer maintains the data rate -- a good habit that we should keep
// 

cplusplus {{
#include <AirFrame_m.h>
}};

class AirFrame;

message AirFrame80211 extends AirFrame {
    fields:
	double bitrate;
};
