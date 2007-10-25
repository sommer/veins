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
#include "BaseUtility.h"
#include "NetwControlInfo.h"

Define_Module(BaseLocalization);

void BaseLocalization::initialize(int stage)
{
	BaseLayer::initialize(stage);

	switch (stage) {
	case 0:
		id = findHost()->index();
		headerLength = par("headerLength");
		isAnchor = par("isAnchor");
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

	EV << "Anchor neighbors(" << anchors.size() <<
		") of node " << id << ": " << endl;
	while (anchors.begin() != anchors.end()) {
		NodeInfo * node = *anchors.begin();
		EV_clear << "\t" << node->id <<
			node->pos.info() <<
			node->pos.getTimestamp() << endl;
		anchors.erase(anchors.begin());
		delete node;
	}

	EV << "Regular neighbors(" << neighbors.size() <<
		") of node " << id << ": " << endl;
	while (neighbors.begin() != neighbors.end()) {
		NodeInfo * node = *neighbors.begin();
		EV_clear << "\t" << node->id <<
			node->pos.info() <<
			node->pos.getTimestamp() << endl;
		neighbors.erase(neighbors.begin());
		delete node;
	}

	BaseLayer::finish();
}

Coord BaseLocalization::getPosition()
{
	BaseUtility *util =
	    FindModule < BaseUtility * >::findSubModule(findHost());
	return *util->getPos();
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

cMessage *BaseLocalization::decapsMsg(cMessage * msg)
{
	LocPkt * pkt = dynamic_cast<LocPkt *>(msg);
	if (pkt->getIsAnchor()) {
		newAnchor(msg);
	} else {
		newNeighbor(msg);
	}

	NetwControlInfo *cInfo =
	    dynamic_cast < NetwControlInfo * >(msg->removeControlInfo());

	cMessage *m = msg->decapsulate();
	if (cInfo != NULL)
		m->setControlInfo(cInfo);

	EV << " pkt decapsulated\n";

	delete msg;
	return m;
}

cMessage *BaseLocalization::encapsMsg(cMessage * msg, int kind)
{
	LocPkt *pkt = new LocPkt(msg->name(), kind);
	pkt->setLength(headerLength);
	pkt->setId(id);
	pkt->setIsAnchor(isAnchor);
	if (isAnchor)
		pkt->setPos(getLocation());
	else 
		pkt->setPos(getLocationEstimation());

	NetwControlInfo *cInfo =
	    dynamic_cast < NetwControlInfo * >(msg->removeControlInfo());

	if (cInfo != NULL)
		pkt->setControlInfo(cInfo);

	//encapsulate the application packet
	pkt->encapsulate(msg);

	EV << " pkt encapsulated\n";
	return pkt;
}

void BaseLocalization::handleLowerMsg(cMessage * msg)
{
	if (msg->kind() == APPLICATION_MSG) {
		EV << " handling application packet" << endl;
		sendUp(decapsMsg(msg));
	} else {
		EV << " handling localization packet" << endl;
		handleMsg(msg);
	}
}

void BaseLocalization::handleUpperMsg(cMessage * msg)
{
	sendDown(encapsMsg(msg, APPLICATION_MSG));
}
