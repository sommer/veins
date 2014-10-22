/***************************************************************************
 * file:        SimpleAddress.cc
 *
 * author:      Michael Lindig
 *
 * copyright:   (C) 2011 Fraunhofer-Gesellschaft, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************/
#include "veins/base/utils/SimpleAddress.h"

#include <cassert>

const LAddress::L2Type LAddress::L2BROADCAST = 
    LAddress::L2Type(-1);
const LAddress::L2Type LAddress::L2NULL      = 
    LAddress::L2Type(0);

const LAddress::L3Type LAddress::L3BROADCAST = LAddress::L3Type(-1);
const LAddress::L3Type LAddress::L3NULL      = LAddress::L3Type(0);
