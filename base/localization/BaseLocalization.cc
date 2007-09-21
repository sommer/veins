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
#include "FindModule.h"

Define_Module(BaseLocalization);

/**
 * 
 */
void BaseLocalization::initialize(int stage)
{
	BaseLayer::initialize(stage);

	switch (stage) {
	case 0:
		id = findHost()->index();
		headerLength = par("headerLength");
		isAnchor = par("isAnchor");
		break;
	default: 
		break;
	}
}

void BaseLocalization::finish()
{
	list<Node *>::const_iterator current;

	EV << "Anchor neighbors(" << anchors.size() <<
		") of node " << id << ": " << endl;
	for (current = anchors.begin();
	     current != anchors.end();
	     current++) {
		EV_clear << "\t" << (*current)->id << " <" <<
			(*current)->pos.getX() << "," <<
			(*current)->pos.getY() << ">:" << 
			(*current)->pos.getTimestamp() << endl;
	}

	EV << "Regular neighbors(" << neighbors.size() <<
		") of node " << id << ": " << endl;
	for (current = neighbors.begin();
	     current != neighbors.end();
	     current++) {
		EV_clear << "\t" << (*current)->id << " <" <<
			(*current)->pos.getX() << "," <<
			(*current)->pos.getY() << ">:" << 
			(*current)->pos.getTimestamp() << endl;
	}
}

Coord BaseLocalization::getPosition()
{
	BaseUtility *util =
	    FindModule < BaseUtility * >::findSubModule(findHost());
	return *util->getPos();
}

Location BaseLocalization::getLocation()
{
	Location loc(getPosition(), simTime());
	return loc;
}

Location BaseLocalization::getLocationEstimation()
{
	return pos;
}

bool BaseLocalization::newAnchor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	/* Check if this point already exists in the anchor
	 * list. This check is made by position. */
	Node * node = new Node(m->getId(),
			       m->getIsAnchor(),
			       m->getPos());
	bool newAnchor = true;
	list<Node *>::const_iterator current;
	for (current = anchors.begin(); 
	     current != anchors.end();
	     current ++) {
		if (node->pos.equals((*current)->pos)) {
			newAnchor = false;
			break;
		}
	}
	/* Add new anchor to list. */
	if (newAnchor) {
		anchors.push_back(node);
		handleNewAnchor(node);
	} else {
		delete node;
	}
	return newAnchor;
}

bool BaseLocalization::newNeighbor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	/* Check if this node already exists in the neighbor
	 * list. This check is made by id. */
	Node * node = new Node(m->getId(),
			       m->getIsAnchor(),
			       m->getPos());
	bool newNeighbor = true;
	bool updatedNeighbor = false;
	list<Node *>::iterator current;
	for (current = neighbors.begin();
	     current != neighbors.end();
	     current ++) {
		if (node->id == (*current)->id) {
			newNeighbor = false;
			/* Update position information of this node. */
			if (!node->pos.equals((*current)->pos))
				updatedNeighbor = true;
			(*current)->pos = node->pos;
			break;
		}
	}
	if (updatedNeighbor) {
		handleMovedNeighbor(node);
	}
	if (newNeighbor) {
		neighbors.push_back(node);
		handleNewNeighbor(node);
	} else {
		delete node;
	}
	return newNeighbor;
}

/**
 * Decapsulates the packet from the received Network packet 
 **/
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

	// delete the localization packet
	delete msg;
	return m;
}

/**
 * Encapsulates the received ApplPkt into a LocPkt and set all needed
 * header fields.
 **/
LocPkt *BaseLocalization::encapsMsg(cMessage * msg, int kind)
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

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
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

void BaseLocalization::sendMsg(cMessage * msg)
{
	sendDown(msg);
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the BaseLocalization we just use the destAddr of the network
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseLocalization::handleUpperMsg(cMessage * msg)
{
	sendDown(encapsMsg(msg, APPLICATION_MSG));
}
