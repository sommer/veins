/* -*- mode:c++ -*- ********************************************************
 * file:        ExampleLocalization.cc
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
 * description: example localization class
 *              This localization class just builds a list of neighbours
 *              and prints this list to the standard output upon finish().
 *              The call to estimatePosition() starts the process.
 *              Note that this class doesn't return the result to the
 *              application, this is something that has to be done in more
 *              serious localization classes.
 **************************************************************************/

#include "ExampleLocalization.h"
#include "NetwControlInfo.h"

#include <SimpleAddress.h>
#include <assert.h>
#include "BaseUtility.h"

#define add_struct(m,s,v)						\
	do { (m)->addPar(s) = (void *) memmove( new char[sizeof(*(v))], (v), \
						sizeof(*(v)));		\
		(m)->par(s).configPointer(NULL,NULL,sizeof(*(v)));	\
	} while (0)
#define add_array(m,s,p,n)						\
	do { (m)->addPar(s) = (void *) memmove( new char[(n)*sizeof(*(p))], (p), \
						(n)*sizeof(*(p)));	\
		(m)->par(s).configPointer(NULL,NULL,(n)*sizeof(*(p)));	\
	} while (0)

#define get_struct(m,s,p)	memmove( (p), (m)->par(s), sizeof(*(p)))
#define get_array(m,s,p,n)	memmove( (p), (m)->par(s), (n)*sizeof(*(p)))

Define_Module_Like(ExampleLocalization, BaseLocalization);

void ExampleLocalization::initialize(int stage)
{
	BaseLocalization::initialize(stage);
	if (stage == 0) {
		BaseUtility *utility=NULL;
		utility = (BaseUtility *)(findHost()->submodule("utility"));
		assert (utility);
		if (isAnchor) {
			position = *utility->getPos();
		}
	}
}

void ExampleLocalization::finish()
{
	nghbor_info *neighbor = NULL;

	BaseLocalization::finish();

	fprintf (stdout, "Neighbours of %d: ", findHost()->index());
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		neighbor = (nghbor_info *) iter();
		fprintf (stdout, "%d; ", neighbor->idx);
	}
	fprintf (stdout, "\n");
}

void ExampleLocalization::estimatePosition()
{
	Enter_Method("esitmatePosition");

	sendPosition();
}

void ExampleLocalization::updateNeighbor(cMessage * msg)
{
	nghbor_info *neighbor = NULL;
	LocPkt *m = static_cast < LocPkt * >(msg);
	int src = m->getSrcAddr();

	bool found = false;
	for (cLinkedListIterator iter(neighbors); !iter.end(); iter++) {
		neighbor = (nghbor_info *) iter();

		if (neighbor->idx == src) {
			found = true;
			break;
		}
	}

	if (!found) {
#ifndef NDEBUG
		fprintf (stdout, "Inserting new neighbour.\n");
#endif
		neighbor = new nghbor_info;
		neighbor->idx = src;
		neighbors.insert(neighbor);
	} else {
#ifndef NDEBUG
		fprintf (stdout, "Not a new neighbour.\n");
#endif
	}

	assert(msg->hasPar("pos"));
	get_struct(msg, "pos", &neighbor->position);
// 	neighbor->distance = (double) msg->par("distance");
}

void ExampleLocalization::handleLowerMsg(cMessage * msg)
{
	switch (msg->kind()) {
	case LOCALIZATION_MSG: {
		LocPkt *m = static_cast < LocPkt * >(msg);
		
		switch (m->getType()) {
		case MSG_POSITION:
			updateNeighbor(msg);
			delete msg;
			break;
		default:
			EV << "Error! got localization packet with unknown type: " << m->getType() << endl;
			delete msg;
		}
	}
		break;
	default:
		EV << "Error! got packet with unknown kind: " << msg->kind() << endl;
		delete msg;
		assert (false);
	}
}

void ExampleLocalization::sendPosition()
{
	LocPkt *pkt = new LocPkt("LOCALIZATION_MSG", LOCALIZATION_MSG);

	pkt->setDestAddr(-1);
	pkt->setSrcAddr(myApplAddr());
	pkt->setType(MSG_POSITION);
	pkt->setName("POSITION");
	pkt->setLength(headerLength);
	add_struct(pkt, "pos", &position);

	// set the control info to tell the network layer the layer 3 address;
	pkt->setControlInfo(new NetwControlInfo(L3BROADCAST));

	EV << "Sending broadcast packet!\n";

	sendDown(pkt);
}
