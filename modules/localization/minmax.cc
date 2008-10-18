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
#include <ConstSpeedMobility.h>
#include <FindModule.h>
#include <UnitDisk.h>
#include <refine.h>
#include <vector>
#include <math.h>

using std::vector;

//Define_Module_Like(minmax, BaseLocalization);

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

      	// Setting the timer_count to 0.
      	timer_count = 0;

	world = FindModule<BaseWorldUtility *>::findGlobalModule();

	const Coord *playgnd =  world->getPgs();


	if(isAnchor) { 
			/* Node is an anchor */
			setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
	}
	else{ 
			/* Node is a regular node */
			setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);

			// Initialising Bounding box.
			self_box.xmax = playgnd->getX();
			self_box.ymax = playgnd->getY();
			
			total_anchor_msgs = 0;
			total_neighbour_msgs = 0;

			
			self_box.xmin = 0;
			self_box.ymin = 0;

			no_of_anchors = 0;
			no_of_neighbours = 0;
			no_of_node_msgs = 0;
	}
    }

	UnitDisk *ud;
	ud = FindModule<UnitDisk *>::findGlobalModule();

	radio_range = ud->calcInterfDist();
	sensor_range = 53;

	ideal_radio_range = radio_range / ( 1 + DOI );
	ideal_sensor_range = sensor_range / (1 + DOI);
	
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
    switch(msg->getKind()){
	    case ANCHOR_BROADCAST_MESSAGE:
			if(! isAnchor)
			{
				/* Anchor only stuff */
			  	newAnchor(msg); 
			}
			delete msg;
	    break;

	    case NODE_BROADCAST_MESSAGE:
			if(! isAnchor)
			{
 	  		   newNeighbor(msg);
			}
			delete msg;
	    break;


	    default:
	  	       EV <<"Error! Got packet with unknown kind/type: " << msg->getKind()<<endl;
		       delete msg;
    }
}

int minmax::calc_irregularity(){
	
	if (R == r)
		return 0;
	else return rand()%RANDOM_RANGE(R,r);
}

// Need to override the newAnchor method from BaseLocalization.cc. The newAnchor() method from BaseLocalization.cc adds a new
// anchor to the anchor list, only if the position of the anchor has not changed. However, in a minmax algorithm

void minmax::newAnchor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	NodeInfo * node = new NodeInfo(m, getPosition());
	/* Check if this point already exists in the anchor
	 * list. This check is made by position. */
	list<NodeInfo *>::const_iterator current;
	for (current = anchors.begin(); current != anchors.end();current ++) 
	{
		if (node->pos == (*current)->pos)
		{
			return;
		}
	}


	BaseUtility *utility = (BaseUtility *) 
				(findHost()->getSubmodule("utility"));
	Coord pos = utility->getPos();

	double real_dist = (node->pos).distance(pos);
	

	R = radio_range;	

	double ideal_range = R / (1 + DOI);

	r = ideal_range*(1 - DOI);

	// Add the neighbour.
	//if (real_dist <= r + calc_irregularity()){
	if (real_dist <= r + int(double ((R - r) * rand())/(RAND_MAX + 1.0)) ) {
		total_anchor_msgs ++;
		anchors.push_back(node);
	
		no_of_anchors++;
	
	
		// Build the anchor box, for the last anchor.
		
		struct bounding_box anchor_box;
	
		anchor_box.xmin = (node->pos).getX() - ideal_radio_range;
		anchor_box.xmax = (node->pos).getX() + ideal_radio_range;
		anchor_box.ymin = (node->pos).getY() - ideal_radio_range;
		anchor_box.ymax = (node->pos).getY() + ideal_radio_range;
	
		rebuild_box(anchor_box);
		wind_up();
	}
}


void minmax::newNeighbor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	NodeInfo * node = new NodeInfo(m, getPosition());
	/* Check if this node already exists in the neighbor
	 * list. This check is made by id. */
	list<NodeInfo *>::iterator current;
	for (current = neighbors.begin();current != neighbors.end();current ++) {
		if (node->id == (*current)->id) {
			return;
		}
	}

	
	Location loc = m->getPos();

	// Get the co-ordinates of the anchor.
	double x = loc.getX();
	double y = loc.getY();	
	double z = loc.getZ();

	// Get the node's own co-ordinate.
	BaseUtility *utility = (BaseUtility *) 
			(findHost()->getSubmodule("utility"));
	Coord pos = utility->getPos();

	double xReal = pos.getX();
	double yReal = pos.getY();
	double zReal = pos.getZ();

	// Compute one's distance
	double distance = sqrt(pow(x - xReal,2) + pow(y-yReal,2) + pow(z - zReal,2));
	double ideal_range;

	/* We didn't return, therefore new neighbor */
	// The Node is a non-Anchor. We need to build the anchor Table based on the received signal strength

	R = sensor_range;

	ideal_range = R / (1 + DOI);

	r = ideal_range*(1 - DOI);

	//if (distance <= r + calc_irregularity())
	if (distance <= r + int(double ((R - r) * rand())/(RAND_MAX + 1.0)) ) 
	{
		if (!isAnchor)
		{
			neighbors.push_back(node);
			no_of_neighbours++;

			// The Anchor Table from the current neighbour

			if (msg->hasPar("no_of_anchors")) {
				int recvd_no_of_anchors = msg->par("no_of_anchors");
				struct Anchor_Table *tmp_array = new struct Anchor_Table[recvd_no_of_anchors];

				get_array(msg,"Anchor_Table",tmp_array,recvd_no_of_anchors);


				for (int i = 0; i < recvd_no_of_anchors; i ++)
				{
					bool common_anchor = false;
					list<NodeInfo *>::const_iterator anchor_list;
					for (anchor_list = anchors.begin(); anchor_list != anchors.end();anchor_list ++)
					{
						if (tmp_array[i].pos == (*anchor_list)->pos){
							common_anchor = true;
							break;
						}
					}
					
					if (common_anchor == false)
					{
						struct bounding_box anchor_box;
						// the bounding box for the negative region has a smaller length, refer to the technical paper for more details.
						double revised_range = ideal_sensor_range * sqrt(2);
	
						anchor_box.xmin = (*anchor_list)->pos.getX() - revised_range;
						anchor_box.xmax = (*anchor_list)->pos.getX() + revised_range;
						anchor_box.ymin = (*anchor_list)->pos.getY() - revised_range;
						anchor_box.ymax = (*anchor_list)->pos.getY() + revised_range;
					
						update_box_with_negative_info(anchor_box);
					}
				}

				delete [] tmp_array;

			}
		}

	}
}

void minmax::update_box_with_negative_info(struct bounding_box anchor_box){
	struct bounding_box intersection_box;

	if (boxes_intersect(anchor_box)){
		// Intersecting the two boxes
		intersection_box.xmin = max(self_box.xmin,anchor_box.xmin);
		intersection_box.ymin = max(self_box.ymin,anchor_box.ymin);
	
		intersection_box.xmax = min(self_box.xmax,anchor_box.xmax);
		intersection_box.ymax = min(self_box.ymax,anchor_box.ymax);	

		// For a convex negative region using bounding boxes, 2 corners of the sensor's bounding box must be in the negative region.
		unsigned number_of_corners_in = 0, corner_count = 0;

		if ( (intersection_box.xmin == self_box.xmin) && (intersection_box.ymin == self_box.ymin)) 
		{ 	number_of_corners_in |= 1; 
			corner_count++;
		}
		if ( (intersection_box.xmax == self_box.xmax) && (intersection_box.ymin == self_box.ymin)) 
		{ 	number_of_corners_in |= 2; 
			corner_count++;
		}
		if ( (intersection_box.xmin == self_box.xmin) && (intersection_box.ymax == self_box.ymax)) 
		{ 	number_of_corners_in |= 4;
			corner_count++;
		}
		if ( (intersection_box.xmax == self_box.xmax) && (intersection_box.ymax == self_box.ymax)) 
		{ 	number_of_corners_in |= 8; 
			corner_count++;
		}

		
		if (corner_count == 4)
			EV << "Problem, node " << id << " cannot be localized " << endl;
		else
			if (corner_count == 2)
			{
				switch(number_of_corners_in){
					// The combinations 0110 and 1001 cannot occur, for they represent the diagonals of the  bounding box.
					case 3: //0011
						{
							self_box.ymin = intersection_box.ymax;
						}
						break;

					case 5: //0101
						{
							self_box.xmin = intersection_box.xmax;
						}
						break;
					case 10: //1010
						{
							self_box.xmax = intersection_box.xmin;
						}
						break;
					case 12: //1100
						{
							self_box.ymax = intersection_box.ymin;
						}
						break;
					default:
						break;
				}
			}
	}
}

void minmax::rebuild_box(struct bounding_box box){
	// Compute the intersection of the existing bounding box and the anchor box.
	
	if (boxes_intersect(box)){
		// Intersecting the two boxes
		self_box.xmin = max(self_box.xmin,box.xmin);
		self_box.ymin = max(self_box.ymin,box.ymin);
	
		self_box.xmax = min(self_box.xmax,box.xmax);
		self_box.ymax = min(self_box.ymax,box.ymax);	
	}
	else
	{
		// Copy the box. If box is an anchor box, then it is always consistent. This is a patch that is added to the original algorithm
		// in order to overwrite obsolete anchor information. Note that we do not need to clear the anchor lists, as all information required to localize the node
		// is captured in the bounding box. Nonetheless, it would still be a good idea to clear them.
		self_box.xmin = box.xmin;
		self_box.xmax = box.xmax;
		self_box.ymin = box.ymin;
		self_box.ymax = box.ymax;
	}
}

int minmax::boxes_intersect(struct bounding_box box){
	// check for four corners

	if ( (box.xmin >= self_box.xmin) && (box.xmin <= self_box.xmax) &&
	     (box.ymin >= self_box.ymin) && (box.ymin <= self_box.ymax) )
		return 1;

	if ( (box.xmin >= self_box.xmin) && (box.xmin <= self_box.xmax) &&
	     (box.ymax >= self_box.ymin) && (box.ymax <= self_box.ymax) )
		return 1;

	if ( (box.xmax >= self_box.xmin) && (box.xmax <= self_box.xmax) &&
	     (box.ymin >= self_box.ymin) && (box.ymin <= self_box.ymax) )
		return 1;

	if ( (box.xmax >= self_box.xmin) && (box.xmax <= self_box.xmax) &&
	     (box.ymax >= self_box.ymin) && (box.ymax <= self_box.ymax) )
		return 1;

	if ( (self_box.xmin >= box.xmin) && (self_box.xmin <= box.xmax) &&
	     (self_box.ymin >= box.ymin) && (self_box.ymin <= box.ymax) )
		return 1;

	if ( (self_box.xmin >= box.xmin) && (self_box.xmin <= box.xmax) &&
	     (self_box.ymax >= box.ymin) && (self_box.ymax <= box.ymax) )
		return 1;

	if ( (self_box.xmax >= box.xmin) && (self_box.xmax <= box.xmax) &&
	     (self_box.ymin >= box.ymin) && (self_box.ymin <= box.ymax) )
		return 1;


	if ( (self_box.xmax >= box.xmin) && (self_box.xmax <= box.xmax) &&
	     (self_box.ymax >= box.ymin) && (self_box.ymax <= box.ymax) )
		return 1;

	return 0;
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
	BaseUtility *utility=NULL;
	//BaseMobility *utility=NULL;
	Coord pos;
	Location *loc;

	EV <<"handelTimer - Node: "<<id<<" Name: "<<getFullName()<< " Path: "<<getFullPath()<<"\n";
	EV<<"Msg kind: "<<index<<" SEND_ANCHOR_POS_TIMER: "<<SEND_ANCHOR_POS_TIMER<<" SEND_NODE_LOC_TIMER: "<<SEND_NODE_LOC_TIMER<</*" NBANCHORS: "<<NBANCHORS<<*/"\n";

	switch(index){
		case SEND_ANCHOR_POS_TIMER: // Anchor timer: send location
			timer_count += ANCHOR_TIMER_INTERVAL;

			// check for threshold limit getting exceeded. If so, cancel the timer.
			// If not exceeded, continue broadcasting the beacons.
			if (timer_count <= ANCHOR_THRESHOLD_TIME)
			{
		
				// Get the new position.
				BaseUtility *utility = (BaseUtility *) 
						(findHost()->getSubmodule("utility"));
				Coord pos = utility->getPos();

				EV << "Anchor position: " << pos.getX() << " " << pos.getY() << endl;
				Location *loc = new Location(pos,simTime(),1.0);
				
				// Prepare the packet to send.
				LocPkt *lp = 
				new LocPkt("ANCHOR_BROADCAST_MESSAGE",ANCHOR_BROADCAST_MESSAGE);
				lp->setId(id);
				lp->setPos(*loc);
				lp->setIsAnchor(true);
	
				// Sending the packet.
				sendBroadcast(lp);
	
				// Resetting the Timer for the Anchor.
				setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
			}
			else
			{
				cancelTimer(SEND_ANCHOR_POS_TIMER);
			}
		break;

		case SEND_NODE_LOC_TIMER: // Node timer: calculate location
		{
				timer_count += NODE_TIMER_INTERVAL;
				if (timer_count >= NODE_THRESHOLD_TIME)
				{
			
					if (no_of_anchors > 0){
						// Prepare the packet to send.
						LocPkt *lp = 
						new LocPkt("NODE_BROADCAST_MESSAGE",NODE_BROADCAST_MESSAGE);
						// Get the new position.
						BaseUtility *utility = (BaseUtility *) 
								(findHost()->getSubmodule("utility"));
						Coord pos = utility->getPos();
						Location *loc = new Location(pos,simTime(),1.0);
						lp->setId(id);
						lp->setPos(*loc);
						lp->setIsAnchor(false);
						lp->addPar("no_of_anchors") = no_of_anchors;
	
						// Add the Anchor Table information.
						struct Anchor_Table anchor_t_array[no_of_anchors];
						list<NodeInfo *>::const_iterator current;
						int iter=0;
						for (current = anchors.begin(); current != anchors.end();current ++)
						{
							/*Coord &tmp = (*current)->pos;*/
							anchor_t_array[iter].id  = (*current)->id;
							anchor_t_array[iter++].pos = (*current)->pos;
						}
	
						

	
						add_array(lp,"Anchor_Table",anchor_t_array,no_of_anchors);
				
						// Sending the packet.
						sendBroadcast(lp);
						no_of_node_msgs++;
					}
			
						// Re-init node timer
						cancelTimer(SEND_NODE_LOC_TIMER);
					
				}
				else
				{
					setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
				}
		}
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
    pkt->setBitLength(headerLength);
    // set the control info to tell the network layer the layer 3 address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void minmax::wind_up(){

	if ((no_of_anchors != 0) || (no_of_neighbours != 0)){
		BaseUtility *utility = (BaseUtility *) 
					(findHost()->getSubmodule("utility"));
		Coord pos = utility->getPos();
	
		double xReal = pos.getX();
		double yReal = pos.getY();
	
		// Compute position as the center of the bounding box.
		double xcalc = (self_box.xmin + self_box.xmax) / 2;
		double ycalc = (self_box.ymin + self_box.ymax) / 2;
	
		double error = sqrt(pow(xcalc - xReal,2) + pow(ycalc - yReal,2));
		
    	if (no_of_anchors + no_of_neighbours >= 2){
			// The following action is to be taken in the case of a moving node. For a static sensor network, these lines are to be commented.
			/****************************/
			EV << "Time = " << simTime() << endl;
			struct error_info ei;
			ei.error = error;
			ei.ts= simTime();
			ei.number_of_anchors = no_of_anchors;

			if (localization_error.size() > 0)
			{
				if (ei.ts > localization_error[localization_error.size() - 1].ts)
					localization_error.push_back(ei);
			}
			else
				localization_error.push_back(ei);
			/****************************/
		
			EV << "Node "<<id<<" final estimated position is: ("<<xcalc<<","<<ycalc<<","<<0<<")" << endl;
		
			EV << "LOCALIZATION ERROR IS " << error << endl;
		
			EV << "NO OF ANCHORS HEARD IS " << no_of_anchors << endl;
		
			EV << "NO OF NEIGHBOURS HEARD IS " << no_of_neighbours << endl;
			EV << "NO OF NODE MESSAGES SENT IS " << no_of_node_msgs << endl;
			// The following action is to be taken in the case of a moving node. For a static sensor network, these lines are to be commented.
			/****************************/
			neighbors.clear();
			anchors.clear();
			no_of_anchors = 0;
			no_of_neighbours = 0;
			/****************************/
			
		}
	}

}

void minmax::display_error_vector(){
	for (int i = 0; i< localization_error.size(); i++)
	{
		EV << "error " << i << " " << localization_error[i].error << endl;
		EV << "ts " << i << " " << localization_error[i].ts << endl;
		EV << "number of anchors " << i << " " << localization_error[i].number_of_anchors << endl;
	}

	EV << "Total anchors heard: " << total_anchor_msgs << endl;

	EV << "Number of rounds: " << localization_error.size() << endl;
}


void minmax::finish() 
{
    if (! isAnchor){
    	wind_up();
		// The following method applies to a moving sensor node, which localizes repeatedly.
		display_error_vector();
	}
    BaseLocalization::finish();
    EV << "\t\tEnding minmax localization...\n";	
}

