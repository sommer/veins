/* -*- mode:c++ -*- ********************************************************
 * file:        apit_mob.cc
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
 * description: APIT class for the localization module
 **************************************************************************/

/* ************************************************************************
 * This file is an adapted copy of locNetwork/TestLocAppl and 
 * implements a APIT localization algorithm (with moving anchors, 
 * see omnetpp.ini file)
 **************************************************************************/

#include "apit_mob.h"
#include "NetwControlInfo.h"
#include "Timer.h"
#include <time.h>
#include <SimpleAddress.h>
#include <ConstSpeedMobility.h>
#include <FindModule.h>
#include <UnitDisk.h>
#include "refine.h"
#include <math.h>
#include <stdlib.h>

 



//Define_Module_Like(apit_mob, BaseLocalization);

void apit_mob::initialize(int stage)
{
    BaseLocalization::initialize(stage);

    if(stage == 0) {
      Timer::init(this);

      // Setting the timer_count to 0.
      timer_count = 0;

      if(isAnchor) { 
		/* Node is an anchor */
		setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
      }
      else{ 
		/* Node is a regular node */
		setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
		no_of_anchors = 0;
		no_of_neighbours = 0;
		time_elapsed = 0;
      }
    }

	UnitDisk *ud;
	ud = FindModule<UnitDisk *>::findGlobalModule();

	radio_range = ud->calcInterfDist();

	sensor_range	=  53;
	
}

void apit_mob::handleTimer(unsigned int index){
	switch(index){
		case SEND_ANCHOR_POS_TIMER: 	// Timer for the Anchor

						timer_count += ANCHOR_TIMER_INTERVAL;
						// check for threshold limit getting exceeded. If so, cancel the timer.
						// If not exceeded, continue broadcasting the beacons.
						if (timer_count >= ANCHOR_THRESHOLD_TIME)
							cancelTimer(SEND_ANCHOR_POS_TIMER);
						else{	
							// Get the new position.
							BaseUtility *utility = (BaseUtility *) 
									(findHost()->getSubmodule("utility"));
							Coord pos = utility->getPos();
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
						break;	

		case SEND_NODE_LOC_TIMER:	// Timer for the non-Anchor
						timer_count += NODE_TIMER_INTERVAL;
						// Check for threshold getting exceeded. Start the timer only if the 
						// threshold has been exceeded.
						if (timer_count >= NODE_THRESHOLD_TIME){
								LocPkt *lp = new 
									LocPkt("NODE_BROADCAST_MESSAGE",NODE_BROADCAST_MESSAGE);
								BaseUtility *utility = (BaseUtility *) 
										(findHost()->getSubmodule("utility"));
								Coord pos = utility->getPos();
	
								// Prepare the packet to send.
								Location *loc = new Location(pos,simTime(),1.0);
								lp->setPos(*loc);
								lp->setId(id);
								lp->setIsAnchor(false);
								// Needed by the receiving node to read the table.
								lp->addPar("no_of_anchors") = no_of_anchors;
	

	
								// Add the Anchor Table information.
								struct Anchor_Table *anchor_t_array = new Anchor_Table[anchor_t.size()];

								for (int iter = 0;iter <anchor_t.size();iter++){
									anchor_t_array[iter].id = anchor_t[iter].id;
									anchor_t_array[iter].pos = anchor_t[iter].pos;
									anchor_t_array[iter].power_level = anchor_t[iter].power_level;
								}
	
	
								//EV << "anchor table size : " << sizeof(*(anchor_t));
								add_array(lp,"Anchor_Table",anchor_t_array,no_of_anchors);
	
	
								// Send a message with the Node Id and the Table of Anchors that
								// it has heard.
								
								sendBroadcast(lp);

								delete [] anchor_t_array;
							// Cancel the timer. The Non Anchors exchange information only once.
							cancelTimer(SEND_NODE_LOC_TIMER);
							
						}
						else{
							// Reset the timer for the non-Anchor.
							setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
						}
						break;
	}
}

/*
	This method addresses the type of action to be carried out by anchors and non-Anchors when they receive a message.
	Anchors can safely ignore any messages recieved. Non-Anchors have to build an anchor table, wherein they store the power level
	(related to distance between sensors) of the anchor signal.	
*/

int apit_mob::calc_irregularity(){
	
	if (R == r)
		return 0;
	else return rand()%RANDOM_RANGE(R,r);
}

double apit_mob::compute_dist(double real_dist, double dist_variance){
	double lower_bound_distance;
	
	lower_bound_distance = real_dist * ( 1 - dist_variance);

	if (int( 2 * real_dist * dist_variance) == 0)
		return real_dist;
	else
		return ( lower_bound_distance + ( rand() % int( 2 * real_dist * dist_variance) ) );
}


void apit_mob::newAnchor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);
	NodeInfo * node = new NodeInfo(m, getPosition());
	/* Check if this point already exists in the anchor
	 * list. This check is made by position. */
	list<NodeInfo *>::const_iterator current;
	for (current = anchors.begin(); current != anchors.end();current ++) {
		if (node->pos == (*current)->pos) {			
			return;
		}
	}

	//Build the Anchor Table here.

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
	double estimated_distance = compute_dist(distance, PERCENTAGE_VARIANCE_IN_MEAN);
	double ideal_range;
	
	// The message has come from an anchor. Create a new entry in the anchor table.
	R = radio_range;	

	ideal_range = R / (1 + DOI);

	r = ideal_range*(1 - DOI);

	if (estimated_distance <= r + calc_irregularity()){
		anchors.push_back(node);

		no_of_anchors++;
		// The Node is a non-Anchor. We need to build the anchor Table based on the received signal strength	
		
		struct Anchor_Table tmp = {m->getId(),m->getPos(),estimated_distance};
		anchor_t.push_back(tmp);
	}
}

void apit_mob::newNeighbor(cMessage * msg) {
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

	if (distance <= r + calc_irregularity())
	{
		
		
		if (!isAnchor)
		{
			neighbors.push_back(node);

			// The Anchor Table from the current neighbour
			struct Anchor_Table *tmp; 
			
			if (msg->hasPar("no_of_anchors")) {
				int recvd_no_of_anchors = msg->par("no_of_anchors");
	
				if (recvd_no_of_anchors >= 3)
				{
					struct Anchor_Table *tmp_array = new struct Anchor_Table[recvd_no_of_anchors];
					
					get_array(msg,"Anchor_Table",tmp_array,recvd_no_of_anchors);
					vector<Anchor_Table> tmp(tmp_array, tmp_array + recvd_no_of_anchors);
	
				// Building the list all neighbours' anchor tables.
					bool new_neighbour = true;
					
					if (new_neighbour == true)
					{
						no_of_neighbours++;
						
						struct neighbour_info tmp_neighbour = {m->getId(),recvd_no_of_anchors,tmp};
						neighbour_t.push_back(tmp_neighbour);
					}
	
					delete [] tmp_array;
				}
			}
		}

	}
}


void apit_mob::handleMsg(cMessage* msg){
	LocPkt * m = dynamic_cast<LocPkt *>(msg);

	// Process the message, depending upon whether it is from an anchor or a non-anchor.
 	switch(m->getKind())
	{
		case ANCHOR_BROADCAST_MESSAGE:  
						if (! isAnchor)
							newAnchor(msg);
						delete msg;
						break;

		case NODE_BROADCAST_MESSAGE:	
						// The message has come from a non-Anchor. The code should store the anchor tables, in order
						// to perform the aggregation process, that follows.
						// The Node is a non-Anchor. We need to build the anchor Table based on the received signal strength
						if (! isAnchor)
							newNeighbor(msg);
						delete msg;
						break;
	}

}


void apit_mob::PIT_test(){
	// This method will perform the PIT testing for the different Anchors heard by the sensor. We make use of the Anchor Table for this purpose. Also, for every set of three anchors, we need to check if any of the sensor's neighbours have the same set. If so, the power levels of that neighbour are inspected to ascertain if our sensor lies within the triangle formed by those three anchors.

	

	unsigned int pit_cnt = 0;
	unsigned tot_combinations=0;

	vector<struct pit_struct> pit;
	bool inside = true;
	bool has_neighbour = false;
	int *res = new int[3],count;


	if (no_of_anchors >= 3)
	{
		for (int i = 0; i < no_of_anchors - 2; i++)
			for (int j = i + 1 ; j < no_of_anchors - 1; j++)
				for (int k = j + 1; k < no_of_anchors ; k++)
				{
					tot_combinations++;
					has_neighbour = false;
					inside = true;
					// This will generate the set of all anchors (N C 3) that are possible.
					unsigned int id1 = anchor_t[i].id;
					unsigned int id2 = anchor_t[j].id;
					unsigned int id3 = anchor_t[k].id;
	
					// Distance information pertaining to the sensor.
	
					SELF_DISTANCE(self_d1,i);
					SELF_DISTANCE(self_d2,j);
					SELF_DISTANCE(self_d3,k);
	
	
					// Check for neighbours that have the same set of anchors. Build a struct of neighbours and power levels.
					for (int nl = 0; nl < no_of_neighbours && (inside == true) ; nl++){
						count = 0;
						
	
						for (int x = 0; x < neighbour_t[nl].no_of_anchors; x++)
						{
							// Since anchors are mobile, the check needs to be performed on anchor positions.
							if (neighbour_t[nl].anchor_ptr[x].pos == anchor_t[i].pos || 	neighbour_t[nl].anchor_ptr[x].pos == anchor_t[j].pos ||
							neighbour_t[nl].anchor_ptr[x].pos == anchor_t[k].pos)
								if (count == 3)
									break;
								else
								{
									res[count] = x;
									count++;
								}
	
						}
	
	
						
						if (count ==  3)
						{
							// There is a neighbour that shares the same set (id1,id2,id3) of anchors.
							has_neighbour = true;
	
							// Distance information of the neighbour
	
							DISTANCE(d1,nl,res,0);
							DISTANCE(d2,nl,res,1);
							DISTANCE(d3,nl,res,2);
							// get the slopes of the three lines
/*
							BaseUtility *utility = (BaseUtility *) 
									(findHost()->getSubmodule("utility"));
							Coord pos = utility->getPos();
							Coord a1 = anchor_t[i].pos;
							Coord a2 = anchor_t[j].pos;
							Coord a3 = anchor_t[k].pos;
								
							if (! tri_test(pos,a1,a2,a3) )
							{
								inside = false;
							}	
*/

							if (( (d1 > self_d1) && (d2 > self_d2) && (d3 > self_d3) ) ||
							( (d1 <= self_d1) && (d2 <= self_d2) && (d3 <= self_d3) )	)
								inside = false;	

							
						}
	
					}

					// Add the three anchor information to a list only if the sensor node has neighbours.
				
					if (has_neighbour)
					{
						pit_cnt++;
	
						struct pit_struct tmp = {i,j,k,inside};
	
						pit.push_back(tmp);
					}
				
				}
	
			if (pit_cnt > 0){
				Coord final_estimate = compute_position(pit,pit_cnt);
			}

	}
}

int * apit_mob::hasAnchors(int nl,int id1,int id2,int id3){
	int count = 0;
	int *res = new int[3];

	for (int i = 0; i < neighbour_t[nl].no_of_anchors; i++){
		if (neighbour_t[nl].anchor_ptr[i].id == id1 || neighbour_t[nl].anchor_ptr[i].id == id2 ||
		    neighbour_t[nl].anchor_ptr[i].id == id3)
			res[count++] = i;
	}

	//return NULL;

	if (count == 3) return res; else return NULL;
}

int apit_mob::tri_test(Coord pos,Coord a1,Coord a2,Coord a3){

	double slope1 = (a1.getY() - a2.getY()) / (a1.getX() - a2.getX());
	double slope2 = (a2.getY() - a3.getY()) / (a2.getX() - a3.getX());
	double slope3 = (a3.getY() - a1.getY()) / (a3.getX() - a1.getX());

	double c1 = a1.getY() - (slope1*a1.getX());
	double c2 = a2.getY() - (slope2*a2.getX());
	double c3 = a3.getY() - (slope3*a3.getX());

	// Which side of the line?
	int side[3];

	if (a3.getY() > (slope1*a3.getX() + c1))
		side[2] = -1;
	else
		side[2] = 1;

	if (a1.getY() > (slope2*a1.getX() + c2))
		side[0] = -1;
	else
		side[0] = 1;

	if (a2.getY() > (slope3*a2.getX() + c3))
		side[1] = -1;
	else
		side[1] = 1;

	int self_side[3];
	

	if (pos.getY() > (slope1*pos.getX() + c1))
		self_side[2] = -1;
	else
		self_side[2] = 1;

	if (pos.getY() > (slope2*pos.getX() + c2))
		self_side[0] = -1;
	else
		self_side[0] = 1;

	if (pos.getY() > (slope3*pos.getX() + c3))
		self_side[1] = -1;
	else
		self_side[1] = 1;

	for (int i = 0;i < 3;i++)
		if (side[i] != self_side[i])
			return 0;

	return 1;
}



Coord apit_mob::compute_position(vector <struct pit_struct> pit, int pit_cnt){

	// Firstly, we need to form a grid layout of the playground. The grid resolution is set to 0.1 times the radio range, which means we need to extract 
	// the radio range and the play ground size in this method. We assume that the grids completely fill up the playground.

	// Getting the playground size.
	BaseWorldUtility *world;

	world = FindModule<BaseWorldUtility *>::findGlobalModule();

	const Coord *playgnd =  world->getPgs();

	
	// Setting the grid resolution.
	double grid_size = 0.1 * radio_range;

	int rows =	int(floor(playgnd->getX() / grid_size ));
	int columns =	int(floor(playgnd->getY() / grid_size ));

	// Declaring the grid array.

	
	int grd[rows][columns];

	// Initializing the centres of the grids.

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < columns; j++){
				grd[i][j]  = 0;
		}


	// Secondly, we need to assign to each grid, a count depending on whether the centroid lies within the set of anchors that are heard. Since we cannot
	// test whether a small part of a triangle lies withing the grid, we consider only those cases where the corners of a grid lie inside the triangle.
	// OR the endpoints of the triangle lie within the grid. In case the triangle does not enclose the unknown sensor, the count is decremented, 
	// else it is incremented.

		for (int i = 0; i< pit_cnt ;  i++){
			// Get the anchor coordinates.
			GET_ANCHOR(anchor1,pit[i].pos1);
			GET_ANCHOR(anchor2,pit[i].pos2);
			GET_ANCHOR(anchor3,pit[i].pos3);
/*
			if (pit[i].inside == true){
				EV << "Inside triangle bounded by anchors (" << anchor_t[pit[i].pos1].id << "," << anchor_t[pit[i].pos2].id << "," << anchor_t[pit[i].pos3].id << ")" << endl;
			}
*/
			// Get the coordinates of the centroid.
			Coord centroid = new Coord((anchor1.getX() + anchor2.getX() + anchor3.getX())/3, (anchor1.getY() + anchor2.getY() + anchor3.getY())/3 );

			// Test each grid whether it contains the triangle.
			for(int g1 = 0; g1 < rows ; g1++)
				for(int g2 = 0; g2 < columns ; g2++)
					// Test the end points of the triangle first.

					if (IS_IN_GRID(anchor1) || IS_IN_GRID(anchor2) || IS_IN_GRID(anchor3) ){	
						// The count is incremented/decremented depending on whether the anchors enclose the sensor.
						if (pit[i].inside == true)
							grd[g1][g2]++;
						else
							grd[g1][g2]--;
					}
					else
					{
						// Test the four corners of the grid, checking to see if any of them lie inside the triangle.
						// Bottom left
						Coord  c1,c2,c3,c4;
						if (g1 == rows - 1)
						{
							c2.setX(playgnd->getX());
							c4.setX(playgnd->getX());	
						}
						else
						{
							c2.setX(GETCOORD(g1,grid_size) + (grid_size)/2);
							c4.setX(GETCOORD(g1,grid_size) + (grid_size)/2);
						}
					
						if (g2 == columns - 1)
						{
							c3.setY(playgnd->getY());
							c4.setY(playgnd->getY());
						}
						else
						{
							c3.setY(GETCOORD(g2,grid_size) + (grid_size)/2);
							c4.setY(GETCOORD(g2,grid_size) + (grid_size)/2);		
						}

						c1.setX(GETCOORD(g1,grid_size) - (grid_size)/2);						
						c1.setY(GETCOORD(g2,grid_size) - (grid_size)/2);
						c2.setY(GETCOORD(g2,grid_size) - (grid_size)/2);
						c3.setX(GETCOORD(g1,grid_size) - (grid_size)/2);
							
						
						
						if (	tri_test(c1,anchor1, anchor2, anchor3) || 
							tri_test(c2,anchor1, anchor2, anchor3) ||
							tri_test(c3,anchor1, anchor2, anchor3) ||
							tri_test(c4,anchor1, anchor2, anchor3) )
						{
							if (pit[i].inside == true)
								grd[g1][g2]++;
							else
								grd[g1][g2]--;
						}
						else
						{
							Coord centre;
							centre.setX((c1.getX() + c2.getX())/2);
							centre.setY((c1.getY() + c3.getY())/2);
							
							if (tri_test(centre,anchor1,anchor2,anchor3))
								if (pit[i].inside == true)
									grd[g1][g2]++;
								else
									grd[g1][g2]--;
						}
					}
					
		}


	

	// Thirdly and finally, we have to identify the grid positions that have the maximum count for a given sensor node. The location of the node is then
	// computed as the average of all the grids that lie in the region of intersection.
	// Computing the maximum count.
	// Computing the position estimate of the node as the average of all grid points.
	double total_x=0,total_y=0,no_of_grids=0;

	
	int max_count = grd[0][0];
	for(int g1 = 0; g1 < rows ; g1++)
			for(int g2 = 0; g2 < columns ; g2++)	
				if (grd[g1][g2] > max_count)
					max_count = grd[g1][g2];

	// This checks if a sensor has received anchors, and is yet unable to localize. In such a case, we return new Coord(-1,-1), and do not include this node in the coverage.

	if (max_count != 0)
	{

		for(int g1 = 0; g1 < rows ; g1++)
			for(int g2 = 0; g2 < columns ; g2++)
				if (grd[g1][g2] == max_count)
				{
					total_x = total_x +  GETCOORD(g1,grid_size);
					total_y = total_y +  GETCOORD(g2,grid_size);
					no_of_grids++;
				}
	
		EV << "Node "<<id<<" final estimated position is: ("<<total_x/no_of_grids<<","<<total_y/no_of_grids<<","<<0<<")" << endl;

		

		BaseUtility *utility = (BaseUtility *) 
				(findHost()->getSubmodule("utility"));
		Coord pos = utility->getPos();

		double xReal = pos.getX();
		double yReal = pos.getY();

		// Compute one's distance
		double xcalc = total_x/no_of_grids;
		double ycalc = total_y/no_of_grids;

		double error = sqrt(pow(xcalc - xReal,2) + pow(ycalc - yReal,2));

		EV << "LOCALIZATION ERROR IS " << error << endl;

		EV << "NO OF ANCHORS HEARD IS " << no_of_anchors << endl;

		EV << "NO OF NEIGHBOURS HEARD IS " << no_of_neighbours << endl;

		Coord ret;
		
		ret.setX(total_x/no_of_grids);
		ret.setY(total_x/no_of_grids);

		for (int i = 0; i< pit_cnt ;  i++){
			// Get the anchor coordinates.
			GET_ANCHOR(anchor1,pit[i].pos1);
			GET_ANCHOR(anchor2,pit[i].pos2);
			GET_ANCHOR(anchor3,pit[i].pos3);

			if (pit[i].inside == true)
			{
				if (! tri_test(pos,anchor1,anchor2,anchor3))
				{
					EV << "OuttoIn error for Node " << id << endl;
					break;
				}
			}
			else
			{
				if (tri_test(pos,anchor1,anchor2,anchor3))
				{
					EV << "IntoOut error for Node " << id << endl;
					break;
				}
			}

		}

		for (int i = 0; i < pit_cnt ; i++){
			// Get the anchor coordinates.
			GET_ANCHOR(anchor1,pit[i].pos1);
			GET_ANCHOR(anchor2,pit[i].pos2);
			GET_ANCHOR(anchor3,pit[i].pos3);

			if (pit[i].inside == true)
			{
				if (! tri_test(ret,anchor1,anchor2,anchor3))
				{
					EV << "CoG problem for Node " << id << endl;
					break;
				}
				
			}
		}

		EV << "pit_cnt: " << pit_cnt << endl;
		return ret;
	}
	else
		return new Coord(-1,-1);
	
		
}
   

int apit_mob:: in_triangle(Coord c, Coord anchor1, Coord anchor2, Coord anchor3){
	const double pi = 8.0*atan(1.0);

	if (	atan((c.getY() - anchor1.getY())/(c.getX() - anchor1.getX())) + 
		atan((c.getY() - anchor2.getY())/(c.getX() - anchor2.getX())) + 
		atan((c.getY() - anchor3.getY())/(c.getX() - anchor3.getX()) ) == pi)
		return 1;
	else
		return 0;
	
} 


/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void apit_mob::sendBroadcast(LocPkt *pkt)
{
    pkt->setBitLength(headerLength);
    // set the control info to tell the network layer the layer 3 address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void apit_mob::finish(){
	struct timespec t1,t2;

	
	if (!isAnchor)		
	{
	//	clock_gettime(CLOCK_REALTIME, &t1);

    		PIT_test();

	//	clock_gettime(CLOCK_REALTIME, &t2);
	//	time_elapsed = time_elapsed + (double)(t2.tv_sec - t1.tv_sec) + 1.e-9*(t2.tv_nsec - t1.tv_nsec);
	}
	

	EV << "Time elapsed is " << time_elapsed << endl;	
	BaseLocalization::finish();
	EV << "\t\tEnding APIT localization...\n";	
}



