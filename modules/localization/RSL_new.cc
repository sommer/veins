/* -*- mode:c++ -*- ********************************************************
 * file:        RSL_new.cc
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

#include "RSL_new.h"
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
#include <ctime>




Define_Module_Like(RSL_new, BaseLocalization);

void RSL_new::initialize(int stage)
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
			self_info.id = id;
			// Initialising Bounding box.
			self_info.self_box.xmax = playgnd->getX();
			self_info.self_box.ymax = playgnd->getY();
			
			total_anchor_msgs = 0;
			total_neighbour_msgs = 0;

			
			self_info.self_box.xmin = 0;
			self_info.self_box.ymin = 0;
			// Initialising estimate
			self_info.estimate1.setX(-1);
			self_info.estimate2.setX(-1);
			self_info.estimate1.setY(-1);
			self_info.estimate2.setY(-1);
			// Initialising confidence.
			self_info.confidence = 0;

			no_of_anchors = 0;
			no_of_neighbours = 0;
			time_elapsed = 0;
			no_of_iterations = 0;	
			total_no_of_anchors = 0;

			setTimer(ANCHOR_TIMEOUT,ANCHOR_TIMEOUT_INTERVAL);

			// Initialise the pdf of all positions in the bounding box. The probability distribution is intially uniform, and is equal to 1.
		/*
			PostEst = new double *[int(playgnd->getX())];
			for (int i = 0 ; i < int(playgnd->getX()) ; i ++)
				*(PostEst + i) = new double[int(playgnd->getY())];

			for (int i = 0; i < int(playgnd->getX()); i++)
				for (int j = 0; j < int(playgnd->getY()); j++)
					PostEst[i][j] = 1.0;
		*/

	}
    }

	UnitDisk *ud;
	ud = FindModule<UnitDisk *>::findGlobalModule();

	radio_range = ud->calcInterfDist();
	sensor_range = 53;

	ideal_radio_range = radio_range / ( 1 + DOI);
	ideal_sensor_range = sensor_range / ( 1 + DOI);

	variance  = 0.20 * ideal_radio_range;
	srand((unsigned)time(NULL)); 
}

void RSL_new::handleTimer(unsigned int index){
	switch(index){
		case SEND_ANCHOR_POS_TIMER: 	// Timer for the Anchor
						{
							timer_count += ANCHOR_TIMER_INTERVAL;

							// check for threshold limit getting exceeded. If so, cancel the timer.
							// If not exceeded, continue broadcasting the beacons.
							if (timer_count <= ANCHOR_THRESHOLD_TIME)
							{	
								// Get the new position.
								BaseUtility *utility = (BaseUtility *) 
										(findHost()->submodule("utility"));
								Coord pos = utility->getPos();
								Location *loc = new Location(pos,simTime(),1.0);
								
								EV << "Anchor position: " << pos.getX() << " " << pos.getY() << endl;

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
						}
						break;

		case ANCHOR_TIMEOUT:
						compute_position(1);
						
						break;


		case SEND_NODE_LOC_TIMER:	// Timer for the non-Anchor
						// Reset the timer for the non-Anchor.
						// Send a message only of the confidence value is higher than 0.005.

						if (self_info.confidence > 0.0001)
						{
							// Stuff to send:
							//	1.	id
							//	2.	estimate
							//	3.	Bounding Box
							//	4.	confidence
							//	5. 	true position (for newNeighbour method)
							

							// Prepare the packet to send.
							LocPkt *lp = 
							new LocPkt("NODE_BROADCAST_MESSAGE",NODE_BROADCAST_MESSAGE);
								
								
								
							// id
							lp->setId(id);
							// position estimate
							lp->addPar("estimateX") = self_info.estimate1.getX();
							lp->addPar("estimateY") = self_info.estimate1.getY();

							// bounding box.
							add_struct2(lp,"bounding_box",&(self_info.self_box));
							// confidence.
							lp->addPar("confidence") = self_info.confidence;
							// true position.
							BaseUtility *utility = (BaseUtility *) 
										(findHost()->submodule("utility"));
							Coord pos = utility->getPos();
							Location *loc = new Location(pos,simTime(),1.0);
							lp->setPos(*loc);

							// Sending the packet.
							sendBroadcast(lp);
							cancelTimer(SEND_NODE_LOC_TIMER);
						}
						else
							setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);

						break;
	}
}


void RSL_new::display_results(){
	
	BaseUtility *utility = (BaseUtility *) 
				(findHost()->submodule("utility"));
	Coord pos = utility->getPos();

	double xReal = pos.getX();
	double yReal = pos.getY();

	// Compute one's distance
	double xcalc = self_info.estimate1.getX();
	double ycalc = self_info.estimate1.getY();


	if ( xcalc != -1 || ycalc != -1 )
	{
		EV << "Node "<<id<<" final estimated position is: ("<<self_info.estimate1.getX()<<","<<self_info.estimate1.getY()<<","<<0<<")" << endl;

		if ((self_info.estimate2.getX() != -1) && (self_info.estimate2.getY() != -1) )
		{
			EV << "Node "<<id<<" has a candidate position at: ("<<self_info.estimate2.getX()<<","<<self_info.estimate2.getY()<<","<<0<<")" << endl;
		}

		double error = sqrt(pow(xcalc - xReal,2) + pow(ycalc - yReal,2));
		localization_error.push_back(error);
	
		EV << "LOCALIZATION ERROR IS " << error << endl;
	
		EV << "CONFIDENCE IS " << self_info.confidence << endl;

		EV << "COMPUTATION_TIME IS " << time_elapsed << endl;

		EV << "No of ITERATIONS IS " << no_of_iterations <<  endl;

		EV << "No of PSEUDO-ANCNHORS IS " << total_no_of_anchors << endl;
	}
	else
	{
		EV << "bounding box for Node " << id << endl;
		EV << "xmin = " << self_info.self_box.xmin;
		EV << "\t xmax = " << self_info.self_box.xmax << endl;
		EV << "ymin = " << self_info.self_box.ymin;
		EV << "\t ymax = " << self_info.self_box.ymax << endl;
	}

	EV << "NO OF ANCHORS HEARD IS " << total_anchor_msgs << endl;

	EV << "NO OF NEIGHBOURS HEARD IS " << total_neighbour_msgs << endl;
}

/*
	This method addresses the type of action to be carried out by anchors and non-Anchors when they receive a message.
	Anchors can safely ignore any messages recieved. Non-Anchors have to build an anchor table, wherein they store the power level
	(related to distance between sensors) of the anchor signal.	
*/


void RSL_new::newAnchor(cMessage * msg) {
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

	// Reset the timer, to extend the timeout interval.
	//cancelTimer(ANCHOR_TIMEOUT);
	BaseUtility *utility = (BaseUtility *) 
				(findHost()->submodule("utility"));
	Coord pos = utility->getPos();

	double real_dist = (node->pos).distance(pos);
	
	
	R = radio_range;	

	ideal_range = R / (1 + DOI);

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
	
		/*if (first_anchor == true){
			first_anchor = false;
			self_info.self_box.xmin = min(0,anchor_box.xmin;
			self_info.self_box.ymin = anchor_box.xmax;
		
			self_info.self_box.xmax = anchor_box.ymin;
			self_info.self_box.ymax = anchor_box.ymax;			

		}
		else*/
			rebuild_box(anchor_box);
		//comment for case 4, uncomment for case 3
		compute_position(0);
		//setTimer(ANCHOR_TIMEOUT,ANCHOR_TIMEOUT_INTERVAL);
	}
}

void RSL_new::newNeighbor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);

	EV << "message info :" << m->info() << endl;
	EV << "index is " << m->findPar("estimateX") << endl;

	if (m->hasPar("estimateX") && m->hasPar("estimateY")){
		double X = m->par("estimateX");
		double Y = m->par("estimateY");
		
	
		Coord pos(X,Y);
		// setting the estimated position.
		NodeInfo * node = new NodeInfo(m, pos);
		/* Check if this node already exists in the neighbor
		* list. This check is made by id. */
		list<NodeInfo *>::iterator current;
		for (current = neighbors.begin();current != neighbors.end();current ++) {
			if (node->id == (*current)->id) {
				return;
			}
		}
	
		BaseUtility *utility = (BaseUtility *) 
					(findHost()->submodule("utility"));
		Coord pos1 = utility->getPos();
	
		double real_dist = (node->pos).distance(pos1);
		
	
		R = sensor_range;	
	
		ideal_range = R / (1 + DOI);
	
		r = ideal_range*(1 - DOI);
	
		// Add the neighbour.
		//if (real_dist <= r + calc_irregularity()){
		if (real_dist <= r + int(double ((R - r) * rand())/(RAND_MAX + 1.0)) ) {
			neighbors.push_back(node);
			total_neighbour_msgs++;
			no_of_neighbours++;
		
			// Update the confidence information
			(node->pos).setConfidence(m->par("confidence"));
	
			// We need to rebuild the node's extended bounding box, by increasing the dimensions of the received box with the radio range R.
		
			struct bounding_box extended_box,recvd_box;
			get_struct2(m,"bounding_box",&recvd_box);
		
			extended_box.xmin = recvd_box.xmin - ideal_sensor_range;
			extended_box.ymin = recvd_box.ymin - ideal_sensor_range;
		
			extended_box.xmax = recvd_box.xmax + ideal_sensor_range;
			extended_box.ymax = recvd_box.ymax + ideal_sensor_range;
		
			// rebuild the sensor's bounding box.
		
			rebuild_box(extended_box);
		
			// Localize the sensor, if the number of anchors + number of neighbours > 2
			compute_position(1);
			//setTimer(ANCHOR_TIMEOUT,ANCHOR_TIMEOUT_INTERVAL);
		}

	}
}


void RSL_new::compute_position(int flag){
	struct timespec t1,t2;

	//if ( (self_info.self_box.xmax  - self_info.self_box.xmin) * (self_info.self_box.ymax  - self_info.self_box.ymin) < 0.75*radio_range*radio_range)
	if (no_of_anchors + no_of_neighbours >= 2)
	{
		self_info.estimate1 = localize_with_mode(flag);
		neighbors.clear();
		anchors.clear();
		total_no_of_anchors += no_of_anchors + no_of_neighbours;
		no_of_anchors = 0;
		no_of_neighbours = 0;
		//display_results();		
	}
	// comment out for case3, include it for case4
	/*
	else
	{
		// Clear the anchor and neighbour information gathered till now. It is likely to be outdated for the next round of localization.
		neighbors.clear();
		anchors.clear();
		total_no_of_anchors += no_of_anchors + no_of_neighbours;
		no_of_anchors = 0;
		no_of_neighbours = 0;
	}
	*/
	setTimer(ANCHOR_TIMEOUT,ANCHOR_TIMEOUT_INTERVAL);
	
}

double RSL_new::compute_dist(double real_dist, double dist_variance){
	double lower_bound_distance;
	
	lower_bound_distance = real_dist * ( 1 - dist_variance);

	if (int( 2 * real_dist * dist_variance) == 0)
		return real_dist;
	else
		return ( lower_bound_distance + ( rand() % int( 2 * real_dist * dist_variance) ) );
}

Coord RSL_new::localize_with_mode(int flag){
	if ((self_info.self_box.xmax <= self_info.self_box.xmin) ||
	    (self_info.self_box.ymax <= self_info.self_box.ymin) ){
		EV "BAD BOX for Node " << id << endl;
		return new Coord(-1,-1);
	}
	
	no_of_iterations++;
	// Calculate area of Bounding box.
	int length = int(ceil(self_info.self_box.xmax - self_info.self_box.xmin));
	int width  = int(ceil(self_info.self_box.ymax - self_info.self_box.ymin));

	PostEst = new double *[length];
	for (int i = 0 ; i < length; i++)
		*(PostEst + i) = new double[width];

			

	// Initializing the probability map.
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
			PostEst[i][j] = 1.0;

	double area = length * width;
	// local probability map.
	double refine[length][width];
	double sum_probability = 0.0;
	


	// Get sensor's location.
	BaseUtility *utility = (BaseUtility *) 
			(findHost()->submodule("utility"));
	Coord pos = utility->getPos();
	Coord avg;
	int total_anchors_heard = 0;

	

	

	// Anchor list.
	list<NodeInfo *>::const_iterator current;
	for (current = anchors.begin(); current != anchors.end();current ++) 
	{
		total_anchors_heard++;

		// The real distance is taken as the mean of a Gaussian distribution, with variance 0.2 times the radio range.
		double real_dist = ((*current)->pos).distance(pos);
		double estimated_dist = compute_dist(real_dist,PERCENTAGE_VARIANCE_IN_MEAN);

		//variance = 0.20 * real_dist;
		
		// Creating the new probability Map.
		for (int i = 0 ; i < length; i++)
			for (int j = 0; j < width; j++)
			{
				// Translation of indexes to co-ordinates.
				double x = self_info.self_box.xmin + i;
				double y = self_info.self_box.ymin + j;
				// The distance of the point under test, from the anchor.
				Coord tmp(x,y);
				double pt_distance 	=	((*current)->pos).distance(tmp);
				// Refinement , based on a Gaussian function.
				refine[i][j]		=	gaussian(pt_distance,estimated_dist,variance);

				PostEst[i][j]	*=	refine[i][j];
				
			}

		// Calculate for the avg anchor position.
		avg = avg + (*current)->pos;
	}

	// The neighbours list

	if (flag == 1)
	{
		for (current = neighbors.begin(); current != neighbors.end();current ++) 
		{
			total_anchors_heard++;
			// The real distance is taken as the mean of a Gaussian distribution, with variance 0.2 times the radio range.
			double real_dist = ((*current)->pos).distance(pos);

			double estimated_dist = compute_dist(real_dist,PERCENTAGE_VARIANCE_IN_MEAN);
			//variance = 0.20 * real_dist;
			// Creating the new probability Map.
			for (int i = 0 ; i < length; i++)
				for (int j = 0; j < width; j++)
				{
					// Translation of indexes to co-ordinates.
					double x = self_info.self_box.xmin + i;
					double y = self_info.self_box.ymin + j;
					// The distance of the point under test, from the anchor.
					Coord tmp(x,y);
					double pt_distance 	=	((*current)->pos).distance(tmp);
					// Refinement , based on a Gaussian function.
					refine[i][j]		=	 ( (*current)->pos).getConfidence() * gaussian(pt_distance,estimated_dist,0.20*ideal_sensor_range/( (*current)->pos).getConfidence());
	
					PostEst[i][j] 		*=	refine[i][j];
					
				}
			// Calculate for the avg anchor position.
			avg = avg + ( (*current)->pos * ( (*current)->pos).getConfidence() );
		}
	}
	
	// avg anchor location.
	avg = avg / total_anchors_heard;

	

	double max = 0, candidate_max = -1 ;
	int    max_x = 0 ,max_y = 0, cmax_x = -1 ,cmax_y = -1;

	
	
	// Calculating the mode of the pdf.
	
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
		{
			// Translation of indexes to co-ordinates.
			double x = self_info.self_box.xmin + i;
			double y = self_info.self_box.ymin + j;

			if (max <  PostEst[i][j])
			{
				// copy previous maximum information into the candidate solution.
				//candidate_max = max;
				//cmax_x = max_x;
				//cmax_y = max_y;

				max_x 	= i;
				max_y 	= j;
				max 	= PostEst[i][j];
			}
			/*else 
				if (candidate_max <  PostEst[int(x)][int(y)])
				{
					// copy only into the previous maximum information.
					candidate_max = max;
					cmax_x = i;
					cmax_y = j;
				}*/
		}	
/*
	if (candidate_max / max > 0.5)
	{
		if ( (abs(max_x - cmax_x) > 0.5 * (self_info.self_box.xmax - self_info.self_box.xmin) ) ||
		     (abs(max_y - cmax_y) > 0.5 * (self_info.self_box.ymax - self_info.self_box.ymin) ) )
		{
			self_info.estimate2 = new Coord(self_info.self_box.xmin + cmax_x,self_info.self_box.ymin + cmax_y);
		}
	}
	else
	{
			self_info.estimate2 =  new Coord(-1,-1);
	}
*/
	// Calculating the sum of the individual probabilities.
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
		{
			// Translation of indexes to co-ordinates.
			double x = self_info.self_box.xmin + i;
			double y = self_info.self_box.ymin + j;
			sum_probability += PostEst[i][j];
		}
	// Recalculating each map-point's likelihood of being the true location of the sensor node.
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
		{
			// Translation of indexes to co-ordinates.
			double x = self_info.self_box.xmin + i;
			double y = self_info.self_box.ymin + j;

			PostEst[i][j] /= sum_probability;
		}

	// Confidence of the sensor
	self_info.confidence = max / sum_probability;

	if (equal_pairs(avg))
	{
		for (int i = 0 ; i < length; i++)
			for (int j = 0; j < width; j++)
			{
				// Translation of indexes to co-ordinates.
				double x = self_info.self_box.xmin + i;
				double y = self_info.self_box.ymin + j;
	
				if (candidate_max  <  PostEst[i][j])
				{
					// copy previous maximum information into the candidate solution.
					if (! ( (i == max_x) && (j == max_y) ) )
					{
						candidate_max = PostEst[i][j];
						cmax_x = i;
						cmax_y = j;
					}
				}
			}

		self_info.estimate2 = new Coord(self_info.self_box.xmin + cmax_x,self_info.self_box.ymin + cmax_y);
	}
	else
		self_info.estimate2 =  new Coord(-1,-1);


	delete(PostEst);
	return new Coord(self_info.self_box.xmin + max_x,self_info.self_box.ymin + max_y);
}


bool RSL_new::equal_pairs(Coord avg){
	// We need to check if any of the heard anchor's location widely differ from the avg anchor location, with respect to different axes . If so, then the anchors are probably in a line, and we need to pick the second best estimate as well. In other words, we add a value to estimate2.
	list<NodeInfo *>::const_iterator current;
	Coord diff;
	for (current = anchors.begin(); current != anchors.end();current ++) 
	{
		diff =  avg - (*current)->pos;
		if ((diff.getX() / diff.getY() > 10.0) || (diff.getY() / diff.getX() > 10.0) )
			return true;
	}

	for (current = neighbors.begin(); current != neighbors.end();current ++) 
	{
		diff =  avg - (*current)->pos;
		if ((diff.getX() / diff.getY() > 10.0) || (diff.getY() / diff.getX() > 10.0) )
			return true;
	}

	return false;
}


Coord RSL_new::localize_with_mean(int flag){
	if ((self_info.self_box.xmax <= self_info.self_box.xmin) ||
	    (self_info.self_box.ymax <= self_info.self_box.ymin) ){
		EV "BAD BOX for Node " << id << endl;
		return new Coord(-1,-1);
	}
	
	
	// Calculate area of Bounding box.
	int length = int(ceil(self_info.self_box.xmax - self_info.self_box.xmin));
	int width  = int(ceil(self_info.self_box.ymax - self_info.self_box.ymin));

	double area = length * width;
	// local probability map.
	double refine[length][width];
	double sum_probability = 0.0;
	


	// Get sensor's location.
	BaseUtility *utility = (BaseUtility *) 
			(findHost()->submodule("utility"));
	Coord pos = utility->getPos();

	// Anchor list.
	list<NodeInfo *>::const_iterator current;
	for (current = anchors.begin(); current != anchors.end();current ++) 
	{
		// The real distance is taken as the mean of a Gaussian distribution, with variance 0.2 times the radio range.
		double real_dist = ((*current)->pos).distance(pos);

	
		// Creating the new probability Map.
		for (int i = 0 ; i < length; i++)
			for (int j = 0; j < width; j++)
			{
				// Translation of indexes to co-ordinates.
				double x = self_info.self_box.xmin + i;
				double y = self_info.self_box.ymin + j;
				// The distance of the point under test, from the anchor.
				Coord tmp(x,y);
				double pt_distance 	=	((*current)->pos).distance(tmp);
				// Refinement , based on a Gaussian function.
				refine[i][j]		=	gaussian(pt_distance,real_dist,variance);

				PostEst[i][j] 		*=	refine[i][j];
				
			}		
	}

	// The neighbours list
	if (flag == 1)
	{
		for (current = neighbors.begin(); current != neighbors.end();current ++) 
		{
			// The real distance is taken as the mean of a Gaussian distribution, with variance 0.2 times the radio range.
			double real_dist = ((*current)->pos).distance(pos);

			
			// Creating the new probability Map.
			for (int i = 0 ; i < length; i++)
				for (int j = 0; j < width; j++)
				{
					// Translation of indexes to co-ordinates.
					double x = self_info.self_box.xmin + i;
					double y = self_info.self_box.ymin + j;
					// The distance of the point under test, from the anchor.
					Coord tmp(x,y);
					double pt_distance 	=	((*current)->pos).distance(tmp);
					// Refinement , based on a Gaussian function.
					refine[i][j]		=	gaussian(pt_distance,real_dist,variance/( (*current)->pos).getConfidence());
	
					PostEst[i][j] 		*=	refine[i][j];
					
				}
		}
	}

	// Calculating the sum of the individual probabilities.
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
			sum_probability += PostEst[i][j];

	// Recalculating each map-point's likelihood of being the true location of the sensor node.
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
			PostEst[i][j] /= sum_probability;

	double max = PostEst[0][0], max_x = 0 ,max_y = 0;
	
	// Calculating the mode of the pdf.
	
	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
		{
			if (max <  PostEst[i][j])
			{
				//max_x 	= i;
				//max_y 	= j;
				max 	= PostEst[i][j];
			}
		}	
	
	// Confidence of the sensor
	self_info.confidence = max;

	for (int i = 0 ; i < length; i++)
		for (int j = 0; j < width; j++)
		{
			max_x += (i + self_info.self_box.xmin) * PostEst[i][j];
			max_y += (j + self_info.self_box.ymin) * PostEst[i][j];
		}

	return new Coord(max_x,max_y);
}

double RSL_new::gaussian(double pt_distance, double real_dist, double variance){
	/* The pdf is estimated as the difference betwen the cdf's of two neighbouring values */
	/* Need to test for 0% variance */
	if (variance != 0)
	{
		double z1 = (pt_distance + 0.5 - real_dist)/(variance*sqrt(2));
		double z2 = (pt_distance - 0.5 - real_dist)/(variance*sqrt(2));
		double z  = (pt_distance - real_dist)/(variance*sqrt(2));
	
		double ref1 = 0.5 * ( 1 + erf(z1));
		double ref2 = 0.5 * ( 1 + erf(z2));
	
		return (ref1 - ref2);
	}
	else
	{
		// checking for equal distance upto two decimal
		if (ceil(pt_distance) == ceil(real_dist) )
			return 1;
		else
			return 0;
	}
}

void RSL_new::rebuild_box(struct bounding_box box){
	// Compute the intersection of the existing bounding box and the anchor box.

	if (boxes_intersect(box)){
		// Intersecting the two boxes
		self_info.self_box.xmin = max(self_info.self_box.xmin,box.xmin);
		self_info.self_box.ymin = max(self_info.self_box.ymin,box.ymin);
	
		self_info.self_box.xmax = min(self_info.self_box.xmax,box.xmax);
		self_info.self_box.ymax = min(self_info.self_box.ymax,box.ymax);	
	}
	else
	{
		// Copy the box. If box is an anchor box, then it is always consistent.
		self_info.self_box.xmin = box.xmin;
		self_info.self_box.xmax = box.xmax;
		self_info.self_box.ymin = box.ymin;
		self_info.self_box.ymax = box.ymax;
	}
}

int RSL_new::boxes_intersect(struct bounding_box box){
	// check for four corners

	if ( (box.xmin >= self_info.self_box.xmin) && (box.xmin <= self_info.self_box.xmax) &&
	     (box.ymin >= self_info.self_box.ymin) && (box.ymin <= self_info.self_box.ymax) )
		return 1;

	if ( (box.xmin >= self_info.self_box.xmin) && (box.xmin <= self_info.self_box.xmax) &&
	     (box.ymax >= self_info.self_box.ymin) && (box.ymax <= self_info.self_box.ymax) )
		return 1;

	if ( (box.xmax >= self_info.self_box.xmin) && (box.xmax <= self_info.self_box.xmax) &&
	     (box.ymin >= self_info.self_box.ymin) && (box.ymin <= self_info.self_box.ymax) )
		return 1;

	if ( (box.xmax >= self_info.self_box.xmin) && (box.xmax <= self_info.self_box.xmax) &&
	     (box.ymax >= self_info.self_box.ymin) && (box.ymax <= self_info.self_box.ymax) )
		return 1;


	if ( (self_info.self_box.xmin >= box.xmin) && (self_info.self_box.xmin <= box.xmax) &&
	     (self_info.self_box.ymin >= box.ymin) && (self_info.self_box.ymin <= box.ymax) )
		return 1;

	if ( (self_info.self_box.xmin >= box.xmin) && (self_info.self_box.xmin <= box.xmax) &&
	     (self_info.self_box.ymax >= box.ymin) && (self_info.self_box.ymax <= box.ymax) )
		return 1;

	if ( (self_info.self_box.xmax >= box.xmin) && (self_info.self_box.xmax <= box.xmax) &&
	     (self_info.self_box.ymin >= box.ymin) && (self_info.self_box.ymin <= box.ymax) )
		return 1;


	if ( (self_info.self_box.xmax >= box.xmin) && (self_info.self_box.xmax <= box.xmax) &&
	     (self_info.self_box.ymax >= box.ymin) && (self_info.self_box.ymax <= box.ymax) )
		return 1;



	return 0;
}

double RSL_new::max(double a, double b){
	if (a > b) return a; else return b;
}

double RSL_new::min(double a, double b){
	if (a < b) return a; else return b;
}



void RSL_new::handleMsg(cMessage* msg){
	LocPkt * m = dynamic_cast<LocPkt *>(msg);

	// Process the message, depending upon whether it is from an anchor or a non-anchor.
 	switch(m->kind())
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



/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void RSL_new::sendBroadcast(LocPkt *pkt)
{
    pkt->setLength(headerLength);
    // set the control info to tell the network layer the layer 3 address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void RSL_new::display_error_vector(){
	for (int i = 0; i< localization_error.size(); i++)
	{
		EV << "error " << i << " " << localization_error[i] << endl;
	}
		EV << "coverage " << localization_error.size() << endl;
}

void RSL_new::finish(){
	if (! isAnchor)
		display_results();
		//display_error_vector();
	BaseLocalization::finish();
	EV << "\t\tEnding APIT localization...\n";	
}



