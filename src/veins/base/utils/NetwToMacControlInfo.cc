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
#include "veins/base/utils/NetwToMacControlInfo.h"

#include <cassert>
typedef NetwToMacControlInfo  tNetwToMacControlInfoBase;


cObject *const NetwToMacControlInfo::setControlInfo(cMessage *const pMsg, const LAddress::L2Type& pDestAddr)
{
	tNetwToMacControlInfoBase *const cCtrlInfo = new tNetwToMacControlInfoBase();

	cCtrlInfo->setDest(pDestAddr);
	pMsg->setControlInfo(cCtrlInfo);

	return cCtrlInfo;
}

const LAddress::L2Type& NetwToMacControlInfo::getDestFromControlInfo(const cObject *const pCtrlInfo)
{
	const tNetwToMacControlInfoBase *const cCtrlInfo = dynamic_cast<const tNetwToMacControlInfoBase *>(pCtrlInfo);

	assert(cCtrlInfo);
	return cCtrlInfo->getDest();
}
