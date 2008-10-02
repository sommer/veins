/***************************************************************************
 * file:        BaseLocalization.cc
 *
 * author:      Peterpaul Klein Haneveld
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     mixim framework
 * description: localization layer: general class for the network layer
 *              subclass to create your own localization layer
 ***************************************************************************/


#include "BaseLocalization.h"

Define_Module(BaseLocalization);

#define EV_clear ev

void BaseLocalization::initialize(int stage)
{
	BaseLayer::initialize(stage);

	switch (stage) {
	case 0:
		id = findHost()->getIndex();
		worldUtility = FindModule<BaseWorldUtility*>::findGlobalModule();
		if (worldUtility == NULL)
			error("Could not find BaseWorldUtility module");

		headerLength = par("headerLength");
		isAnchor = par("isAnchor");
		break;
	case 1:
		if (isAnchor) {
			pos = getLocation();
		}
		break;
	default:
		break;
	}
}

void BaseLocalization::finish()
{
	EV << "BaseLocalization::finish()"
	   << pos.info()
	   << getPosition().info()
	   << endl;

	list<NodeInfo *>::iterator current;

	EV << "Anchor neighbors("
	   << anchors.size()
	   << ") of node "
	   << id
	   << ": "
	   << endl;
	while (anchors.begin() != anchors.end()) {
		NodeInfo * node = *anchors.begin();
		EV_clear << "\t"
			 << node->info()
			 << " at "
			 << node->pos.getTimestamp()
			 << " s"
			 << endl;
		anchors.erase(anchors.begin());
		delete node;
	}

	EV << "Regular neighbors("
	   << neighbors.size()
	   << ") of node "
	   << id
	   << ": "
	   << endl;
	while (neighbors.begin() != neighbors.end()) {
		NodeInfo * node = *neighbors.begin();
		EV_clear << "\t"
			 << node->pos.info()
			 << " at "
			 << node->pos.getTimestamp()
			 << " s"
			 << endl;
		neighbors.erase(neighbors.begin());
		delete node;
	}

	BaseLayer::finish();
}

Coord BaseLocalization::getPosition()
{
	return *utility->getPos();
}

Location BaseLocalization::getLocation()
{
	Location loc(getPosition(), simTime(), 1.0);
	return loc;
}

Location BaseLocalization::getLocationEstimation()
{
	return pos;
}

void BaseLocalization::sendMsg(cMessage * msg)
{
	sendDown(msg);
}

void BaseLocalization::newAnchor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	NodeInfo * node = new NodeInfo(m, getPosition());
	/* Check if this point already exists in the anchor
	 * list. This check is made by position. */
	list<NodeInfo *>::const_iterator current;
	for (current = anchors.begin();
	     current != anchors.end();
	     current ++) {
		if (node->pos == (*current)->pos) {
			delete node;
			return;
		}
	}

	/* We didn't return, therefore new anchor */
	NodeInfo * node_info = handleNewAnchor(node);
	if (node_info)
		anchors.push_back(node_info);
}

void BaseLocalization::newNeighbor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	NodeInfo * node = new NodeInfo(m, getPosition());
	/* Check if this node already exists in the neighbor
	 * list. This check is made by id. */
	list<NodeInfo *>::iterator current;
	for (current = neighbors.begin();
	     current != neighbors.end();
	     current ++) {
		if (node->id == (*current)->id) {
			if (node->pos != (*current)->pos) {
				/* Update position information of this node. */
				handleMovedNeighbor(node);
				(*current)->pos = node->pos;
			}
			delete node;
			return;
		}
	}

	/* We didn't return, therefore new neighbor */
	NodeInfo * node_info = handleNewNeighbor(node);
	if (node_info)
		neighbors.push_back(node_info);
}

cPacket *BaseLocalization::decapsMsg(cPacket * msg)
{
	LocPkt * pkt = dynamic_cast<LocPkt *>(msg);
	if (pkt->getIsAnchor()) {
		newAnchor(msg);
	} else {
		newNeighbor(msg);
	}

	cObject *cInfo = msg->removeControlInfo();

	cPacket *m = msg->decapsulate();
	if (cInfo != NULL)
		m->setControlInfo(cInfo);

	EV << " pkt decapsulated\n";

	delete msg;
	return m;
}

cPacket *BaseLocalization::encapsMsg(cPacket * msg, int kind)
{
	LocPkt *pkt = new LocPkt(msg->getName(), kind);
	pkt->setBitLength(headerLength);
	pkt->setId(id);
	pkt->setIsAnchor(isAnchor);
	if (isAnchor)
		pkt->setPos(getLocation());
	else
		pkt->setPos(getLocationEstimation());

	cObject *cInfo = msg->removeControlInfo();

	if (cInfo != NULL)
		pkt->setControlInfo(cInfo);

	//encapsulate the application packet
	pkt->encapsulate(msg);

	EV << " pkt encapsulated\n";
	return pkt;
}

void BaseLocalization::handleLowerMsg(cMessage * msg)
{
	if (msg->getKind() == APPLICATION_MSG) {
		EV << " handling application packet" << endl;
		cPacket* pkt = static_cast<cPacket*>(msg);
		sendUp(decapsMsg(pkt));
	} else {
		EV << " handling localization packet" << endl;
		handleMsg(msg);
	}
}

void BaseLocalization::handleUpperMsg(cMessage * msg)
{
	assert(dynamic_cast<cPacket*>(msg) != 0);

	cPacket* pkt = static_cast<cPacket*>(msg);
	sendDown(encapsMsg(pkt, APPLICATION_MSG));
}
