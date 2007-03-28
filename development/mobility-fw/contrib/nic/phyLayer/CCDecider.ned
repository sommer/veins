//***************************************************************************
// * file:        Decider80211a.ned
// *
// * author:      Thomas Freitag, Marc Loebbers
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
// Decider module of the 802.11a modules
//
simple CCDecider
    parameters:
        debug : bool, // debug switch
	ThresholdMode0: numeric const,
	ThresholdMode1: numeric const,
	ThresholdMode2: numeric const,
	ThresholdMode3: numeric const,
	ThresholdMode4: numeric const,
	ThresholdMode5: numeric const,
	ThresholdMode6: numeric const,
	ThresholdMode7: numeric const,
	df: numeric const,
	whiteGaussianNoise: numeric const,
	subbands: numeric const,
	pilots: numeric const,
	symbolTime: numeric const,
	transmissionPower: numeric const,
	ccaThreshold: numeric const;
    gates:
        out: uppergateOut, upperControlOut;
        in: lowergateIn, lowerControlIn;
endsimple
