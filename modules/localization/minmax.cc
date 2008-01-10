/* -*- mode:c++ -*- ********************************************************
 * file:        minmax.cc
 *
 * author:      Aline Baggio
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
 * description: minmax class for the localization module
 **************************************************************************/

/* ************************************************************************
 * This file is an adapted copy of locNetwork/TestLocAppl and 
 * implements a minmax localization algorithm (with moving anchors, 
 * see omnetpp.ini file)
 **************************************************************************/
 
#include "minmax.h"
#include "NetwControlInfo.h"
#include <SimpleAddress.h>
//#include <BaseMobility.h>
#include <ConstSpeedMobility.h>
#include <FindModule.h>
#include <UnitDisk.h>


Define_Module_Like(minmax, BaseLocalization);
//int minmax::nr_dims;
//double *minmax::dim;

//BaseWorldUtility * minmax::world;
/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/



void minmax::initialize(int stage)
{
    BaseLocalization::initialize(stage);
    if(stage == 0) {
      Timer::init(this);

      EV <<"initialize - Stage 0, id: "<<id<<"\n";
      EV <<"initialize - Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
      if(isAnchor) { /* Node is an anchor */
	EV << "\t\tInitializing minmax application for anchor "<<id<<"...\n";
	setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
      }
      else{ /* Node is a regular node */
	EV << "\t\tInitializing minmax application for node "<<id<<"...\n";
	setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
      }
    }

       // nr_dims = (world->use2D()?2:3);
	nr_dims = 3;
/*
        dim = new double[nr_dims];
        switch (nr_dims) {
        case 3: dim[2] = world->getPgs()->getZ(); // fallthrough 
        case 2: dim[1] = world->getPgs()->getY(); // fallthrough 
        case 1: dim[0] = world->getPgs()->getX();
                break;
        default:
                error("initialize() can't handle %d-dimensional space", nr_dims);
                abort();
        }
*/
	for(int i = 0;i<nr_dims;i++){
		if (isAnchor == false){
			rect.min[i] = 0;
			rect.max[i] = 0;
		}
	}


	UnitDisk *ud;
	ud = FindModule<UnitDisk *>::findGlobalModule();

	radio_range = ud->calcInterfDist();

	EV << "The Radio Range is " << radio_range << endl;
}


/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = ANCHOR_BROADCAST_MESSAGE) is a broadcast packet from a
 * neighbor node to which we have to send a reply. The second (kind =
 * NODE_POSITION_MESSAGE (NODE_BROADCAST_MESSAGE in minmax.h)) is a reply to a broadcast packet that we
 * have send and just causes some output before it is deleted
 **/
//void minmax::handleLowerMsg( cMessage* msg )
void minmax::handleMsg( cMessage* msg )
{
    LocPkt *m;
    simtime_t receivedTs;
    int receivedId;
    double x, y, z;
    Location loc;
    bool receivedIsAnchor;

    EV <<"handleMsg - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
    m = dynamic_cast < LocPkt * >(msg);

    receivedId=m->getId();
    loc=m->getPos();
	x=loc.getX();
	y=loc.getY();
	z=loc.getZ();	    
    receivedTs=loc.getTimestamp();
    receivedIsAnchor=m->getIsAnchor();

    switch(m->kind()){
	    case ANCHOR_BROADCAST_MESSAGE:
			EV << "Received a broadcast packet from anchor "<<m->getId()<<"\n";
			if(isAnchor){	/* anchor only stuff */
				  EV << "Anchor "<<id<<" got anchor position message with contents: 						"<<receivedId<<"("<<x<<","<<y<<","<<z<<") at "<<receivedTs<<
					"anchor:"<<receivedIsAnchor<<"\n";
			}
			else
			{
				/* node only stuff */
				build_bounding_box(loc);
			}
			delete msg;
	    break;
/*
	    case NODE_BROADCAST_MESSAGE:
			EV << "Node/anchor "<<id<<" got node position message with contents: "<<m->getId()<<"("<<
			   m->getPos().getX()<<","<<m->getPos().getY()<<","<<m->getPos().getZ()<<") at "
  		   	   <<m->getPos().getTimestamp()<<" anchor:"<<m->getIsAnchor()<<"\n";
 	  		   newNeighbor(msg);
			delete msg;
	    break;

*/
	    default:
	  	       EV <<"Error! Got packet with unknown kind/type: " << m->kind()<<endl;
		       delete msg;
    }
}

// Need to override the newAnchor method from BaseLocalization.cc. The newAnchor() method from BaseLocalization.cc adds a new
// anchor to the anchor list, only if the position of the anchor has not changed. However, in a MinMax algorithm
//bool BaseLocalization::newAnchor(cMessage * msg) {


//}

void minmax::build_bounding_box( Location loc ){
	double *dim = new double[nr_dims];

	dim[0]=loc.getX();
    	dim[1]=loc.getY();
	dim[2]=loc.getZ();
	bound_rect anchor_box;

	for(int i = 0;i < nr_dims;i++){
		anchor_box.min[i] = dim[i] - radio_range;
		anchor_box.max[i] = dim[i] + radio_range;
	}

	//if (rect.min[0] == rect.max[0]){
	if (anchors.size() == 1){
		EV << "First anchor heard" << endl;
		// copy anchor box to the sensor's bounding box
		for(int i = 0;i < nr_dims;i++){
			rect.min[i] = anchor_box.min[i];
			rect.max[i] = anchor_box.max[i];
		}
	}else{
		// Perform the MinMax algorithm. i.e. compute the resulting bounding box
		// as the intersection of the anchor box and the sensor's bounding box.
		if (check_intersection_boxes(anchor_box))
			EV << "The two boxes intersect" << endl;
		else
			//EV << " The two boxes for " << id << " and " << anchor.id  << " do not intersect" << endl;
			EV << " The two boxes do not intersect" << endl;

		EV << "Recomputing the bounding box" << endl;
		for(int i = 0;i < nr_dims;i++){
			rect.min[i] = max(rect.min[i],anchor_box.min[i]);
			rect.max[i] = min(rect.max[i],anchor_box.max[i]);
		}	



		EV << "The recomputed bounds are (" << rect.min[0] << "," << rect.min[1] << ") and (" << rect.max[0]
		   << "," << rect.max[1] << ") " << endl;
		
	}

			
	

//	EV << "To build the bounding box for the new anchor that has been heard" << endl;
	
}

bool minmax::check_intersection_boxes(bound_rect &anchor_box){
	double **points;
	bool inside = false;

	points = new double *[4];
	for (int i = 0; i<2;i++){
		*(points+i) = new double[nr_dims];

		for (int j = 0; j < nr_dims; j++)
			if (i == 0 || j == 0)
				points[i][j] =  rect.min[j];
			else
				points[i][j] =  rect.max[j];
	}

	for (int i = 2; i<4;i++){
		*(points+i) = new double[nr_dims];

		for (int j = 0; j < nr_dims; j++)
			if (i == 2 || j == 1)
				points[i][j] =  rect.max[j];
			else
				points[i][j] =  rect.min[j];
	}

	for (int i = 0; i<4;i++){
		for (int j=0;j<nr_dims;j++)

			if (points[i][j] >= anchor_box.min[j] || points[i][j] <= anchor_box.max[j] ){
				inside = true;
				break;
			}

		if (inside == true) break;		
	}


	return inside;
}

/**
 * A timer with kind = SEND_BROADCAST_TIMER indicates that a new
 * broadcast has to be send (@ref sendBroadcast). 
 *
 * There are no other timer implemented for this module.
 *
 * @sa sendBroadcast
 **/
void minmax::handleSelfMsg(cMessage *msg) {}
void minmax::handleTimer(unsigned int index) {
	LocPkt* pkt;
	double x=0, y=0, z=0, xReal, yReal, zReal;
	Coord pos;
	Location *loc;

	EV <<"handelTimer - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
	EV<<"Msg kind: "<<index<<" SEND_ANCHOR_POS_TIMER: "<<SEND_ANCHOR_POS_TIMER<<" SEND_NODE_LOC_TIMER: "<<SEND_NODE_LOC_TIMER<</*" NBANCHORS: "<<NBANCHORS<<*/"\n";

	switch(index){
  	  case SEND_ANCHOR_POS_TIMER: // Anchor timer: send location
	    if(/*id>=NBANCHORS*/ !isAnchor){EV << "Non anchor node got SEND_ANCHOR_POS_TIMER message (this should never happen)\n"; exit(-1);}
	    if(utility){pos=utility->getPos();}
	    else{EV << "No submodule \"utility\" found\n"; exit(-1);}
	    EV << "Anchor "<<id<<" timer rang: anchor sends its position "<<id<<"("<<pos.getX()<<","<<pos.getY()<<","<<pos.getZ()<<")\n";
	    // do a broadcast
	    pkt = new LocPkt("ANCHOR_BROADCAST_MESSAGE", ANCHOR_BROADCAST_MESSAGE);
	    loc = new Location(pos,simTime(),1.0);
	    pkt->setPos(*loc);
	    pkt->setId(id);
	    pkt->setIsAnchor(true);
	    sendBroadcast(pkt);
	    // Re-init anchor timer
	    setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
	    break;
	  case SEND_NODE_LOC_TIMER: // Node timer: calculate location
	    if(isAnchor){EV << "Anchor node got SEND_NODE_LOC_TIMER message (this should never happen)\n"; exit(-1);}
	    EV << "Node "<<id<<" timer rang: should calculate position\n";
	    // Enough anchor positions? Then estimation location, otherwise wait for more
	    nb_anchor_positions=anchors.size();
	    if(nb_anchor_positions>=MIN_ANCHOR_POSITIONS){
	      EV << "Node "<<id<<" has heard enough anchors: "<<nb_anchor_positions<<"\n";
	      list<NodeInfo *>::const_iterator current;
	      EV << "Anchor neighbors(" << nb_anchor_positions <<") of node " << id << ": " << endl;
	      // calculate position
		double *position = new double[nr_dims];

		for (int i = 0;i<nr_dims;i++)
			position[i] = (rect.min[i] + rect.max[i])/2;			

		x = position[0];
		y = position[1];
		z = position[2];

		this->final_x = x;
		this->final_y = y;
		this->final_z = z;
		
		/*
	      for (current = anchors.begin(); current != anchors.end(); current++) {
		EV_clear << "\t" << (*current)->id << " <" << (*current)->pos.getX() << "," << (*current)->pos.getY() << ">:" << (*current)->pos.getTimestamp() << endl;
		x += (*current)->pos.getX();
		y += (*current)->pos.getY();
		z += (*current)->pos.getZ();
	      }
	      x /= nb_anchor_positions;
	      y /= nb_anchor_positions;
	      z /= nb_anchor_positions; */
	      // Get real position (ground truth)
	      if(utility){pos=utility->getPos(); xReal=pos.getX();yReal=pos.getY();zReal=pos.getZ();}
	      else{EV << "No submodule \"utility\" found\n"; exit(-1);}
	      EV << "Node "<<id<<" estimated position is: ("<<x<<","<<y<<","<<z<<") with "<<nb_anchor_positions<<" anchors heard. True position is:  ("<<xReal<<","<<yReal<<","<<zReal<<")\n";
		
	      // do a broadcast
	      pos = new Coord(x,y,z);
	      pkt = new LocPkt("NODE_BROADCAST_MESSAGE", NODE_BROADCAST_MESSAGE);
	      loc = new Location(pos,simTime(),0.0);
	      pkt->setPos(*loc);
	      pkt->setId(id);
	      pkt->setIsAnchor(false);
	      sendBroadcast(pkt);
	    }
	    else{
	      EV << "Node "<<id<<" has heard too few anchors: "<<nb_anchor_positions<<"\n";
	    }
	    // Re-init node timer
	    setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
	    break;
	  default:
	    EV << "Unkown timer rang -> delete, index: "<<index<<endl;
	}
	
}

/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void minmax::sendBroadcast(LocPkt *pkt)
{
    pkt->setLength(headerLength);
    // set the control info to tell the network layer the layer 3 address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void minmax::finish() 
{
/*
	double x=0, y=0, z=0, xReal, yReal, zReal;
	Coord pos;
	Location *loc;
	if (isAnchor == false){
		if(utility){
				pos=utility->getPos(); 
				xReal=pos.getX();yReal=pos.getY();zReal=pos.getZ();
		}
	      	else
			EV << "No submodule \"utility\" found\n"; exit(-1);
		
		double *position = new double[nr_dims];

		for (int i = 0;i<nr_dims;i++)
			position[i] = (rect.min[i] + rect.max[i])/2;			

		x = position[0];
		y = position[1];
		z = position[2];		

		EV << "Node "<<id<<" FINAL estimated position is: ("<<this->final_x<<","<<this->final_y<<","<<this->final_z<<") with "<<nb_anchor_positions<<" anchors heard. True position is:  ("<<xReal<<","<<yReal<<","<<zReal<<")\n";
	}
*/
    	EV << "Node "<<id<<" FINAL estimated position is: ("<<this->final_x<<","<<this->final_y<<","<<this->final_z << ")" << endl;
    BaseLocalization::finish();
    EV << "\t\tEnding minmax localization...\n";	

	
}

