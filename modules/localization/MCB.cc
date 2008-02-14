/* -*- mode:c++ -*- ********************************************************
 * file:        MCB.cc
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

#include "MCB.h"
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




Define_Module_Like(MCB, BaseLocalization);

void MCB::initialize(int stage)
{
    BaseLocalization::initialize(stage);

    if(stage == 0) {
      Timer::init(this);

	// Setting the timer_count to 0.
      	timer_count = 0;

	world = FindModule<BaseWorldUtility *>::findGlobalModule();

	const Coord *playgnd =  world->getPgs();

	UnitDisk *ud;
	ud = FindModule<UnitDisk *>::findGlobalModule();

	radio_range = ud->calcInterfDist();
	sensor_range = 53;

	variance  = 0.20 * radio_range;
	ideal_radio_range = radio_range / ( 1 + DOI );


	if(isAnchor) { 
			/* Node is an anchor */
			// Set the timer for localization.
			setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
		
	}
	else{ 
			ideal_sensor_range = 53;
			/* Node is a regular node */
			// Initialising location estimate.
			computed_pos = Coord(-1,-1);

			// Initialising Bounding box.
			self_box.xmax = playgnd->getX();
			self_box.ymax = playgnd->getY();
			self_box.xmin = 0;
			self_box.ymin = 0;
			// Initialising the message counts.
			total_anchor_msgs = 0;
			no_of_anchors = 0;
			no_of_neighbours = 0;
			no_of_node_msgs = 0;
			time_elapsed = 0;
			no_of_iterations = 0;
			// Initialising the number of samples.
			number_of_samples = 0;
			new_number_of_samples =0;

			// Set the timer for Localization.
			setTimer(SEND_NODE_LOCALIZATION_TIMER,NODE_TIMER_INTERVAL);
	}
    }
}

void MCB::handleTimer(unsigned int index){
	switch(index){
		case SEND_ANCHOR_POS_TIMER: 	// Timer for the Anchor
						{
							
							// Get the new position.
							BaseUtility *utility = (BaseUtility *) 
									(findHost()->submodule("utility"));
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

							anchor_pos.push_back(pos);
	
							// Resetting the Timer for the Anchor.
							setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
						}
						break;


		case SEND_NODE_LOCALIZATION_TIMER:	// Timer for the non-Anchor
						// Reset the timer for the non-Anchor.
						// Send a message only of the confidence value is higher than 0.005.

						if (! isAnchor)
						{

							// Stuff to send:
							//	1.	id
							//	2.	isAnchor
							//	3.	One hop anchor list (for inferring two hop conectivity)
							

							if (no_of_anchors > 0){
								// Get the new position.
								BaseUtility *utility = (BaseUtility *) 
										(findHost()->submodule("utility"));
								Coord pos = utility->getPos();
								Location *loc = new Location(pos,simTime(),1.0);

								// Prepare the packet to send.
								LocPkt *lp = 
								new LocPkt("NODE_BROADCAST_MESSAGE",NODE_BROADCAST_MESSAGE);
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
								no_of_node_msgs++;
								// Sending the packet.
								sendBroadcast(lp);
							}
				
							// Set the localization timer, so as to allow a certain minimum amount of time, during which a sensor might receive two hop anchor information.
							
							setTimer(START_LOCALIZATION_TIMER,START_LOCALIZATION_TIMER_INTERVAL);
						}

						
						break;
		case START_LOCALIZATION_TIMER:

						if (!isAnchor){
							// Code for MCB localization
							localize_MCB();
							// End Code for MCB localization
							// Reset anchor and neighbour list here. VERY IMPORTANT!!!
							anchors.clear();
							one_hop_anchor_list.clear();
							no_of_anchors = 0;
							neighbors.clear();
							two_hop_anchor_list.clear();
							no_of_neighbours = 0; 
							self_box.xmin = 0;
							self_box.ymin = 0;
							const Coord *playgnd =  world->getPgs();
							self_box.xmax = playgnd->getX();
							self_box.ymax = playgnd->getY();

							// NODE_TIMER_INTERVAL should be equal to ANCHOR_PAUSE_INTERVAL
							setTimer(SEND_NODE_LOCALIZATION_TIMER,NODE_TIMER_INTERVAL);
						}
						break;
	}
}

Coord MCB::generate_random_sample(Coord &pos, double radius){

	double dist,gen_x=0,gen_y=0;
	unsigned no_of_trials=0;
	Coord random_sample;
	struct bounding_box tmp;

	world = FindModule<BaseWorldUtility *>::findGlobalModule();

	const Coord *playgnd =  world->getPgs();

	tmp.xmin = max(0,pos.getX() - radius);
	tmp.ymin = max(0,pos.getY() - radius);
	tmp.xmax = min(playgnd->getX(),pos.getX() + radius);
	tmp.ymax = min(playgnd->getY(),pos.getY() + radius);
	//srand(12);
	
		
	if (boxes_intersect(tmp))
	{
		// Intersecting the two boxes
		tmp.xmin = max(self_box.xmin,tmp.xmin);
		tmp.ymin = max(self_box.ymin,tmp.ymin);
	
		tmp.xmax = min(self_box.xmax,tmp.xmax);
		tmp.ymax = min(self_box.ymax,tmp.ymax);	
	}
	else
	{
		EV << "problem: no intersection " << endl;
		EV << "xmin = " << self_box.xmin;
		EV << "\t xmax = " << self_box.xmax << endl;
		EV << "ymin = " << self_box.ymin;
		EV << "\t ymax = " << self_box.ymax << endl;

		EV << "xmin = " << tmp.xmin;
		EV << "\t xmax = " << tmp.xmax << endl;
		EV << "ymin = " << tmp.ymin;
		EV << "\t ymax = " << tmp.ymax << endl;
		// Apparently, the sample set does not intersect with the bounding box. In this case, we draw the samples directly from the
		// bounding box.
		tmp.xmin = self_box.xmin;
		tmp.ymin = self_box.ymin;
		tmp.xmax = self_box.xmax;
		tmp.ymax = self_box.ymax;
	}

	double length = tmp.xmax - tmp.xmin;
	double width = tmp.ymax - tmp.ymin;


	gen_x = tmp.xmin + double (length * rand())/(RAND_MAX + 1.0);
	gen_y = tmp.ymin + double (width * rand())/(RAND_MAX + 1.0);

	// Overwriting with the newly generated sample.
	random_sample.setX(gen_x);
	random_sample.setY(gen_y);

	return random_sample;
}

bool MCB::filter_with_anchors(Coord random_sample){
	// Apply the filtering condition with one hop and two anchor information.
	// Return true if all constraints are met, and false otherwise.
	double invalid_anchors = 0;
	// One hop constraint checking
	if ((no_of_anchors == 0) && (no_of_neighbours == 0) )
	{
		EV << "no anchors or neighbors to perform filtering" << endl;
	}

	for (int i = 0; i < one_hop_anchor_list.size() ;i ++)
	{
		struct bounding_box tmp;

		world = FindModule<BaseWorldUtility *>::findGlobalModule();

		const Coord *playgnd =  world->getPgs();

		// Filtering is done using the ideal radio range.
		tmp.xmin = max(0,one_hop_anchor_list[i].getX() - ideal_radio_range);
		tmp.ymin = max(0,one_hop_anchor_list[i].getY() - ideal_radio_range);
		tmp.xmax = min(playgnd->getX(),one_hop_anchor_list[i].getX() + ideal_radio_range);
		tmp.ymax = min(playgnd->getY(),one_hop_anchor_list[i].getY() + ideal_radio_range);
		
		if (boxes_intersect(tmp)){
			if ( random_sample.distance(one_hop_anchor_list[i]) > ideal_radio_range )
				return false;
		}
		else
			invalid_anchors++;
		//EV << "IN...filter_with_anchors" << endl;
	}

	EV << "invalid anchors: " << invalid_anchors << endl;
	// Two hop constraint checking
	for (int i = 0; i < two_hop_anchor_list.size(); i++)
	{
		if (	(random_sample.distance(two_hop_anchor_list[i].pos) <= ideal_radio_range) ||
			(random_sample.distance(two_hop_anchor_list[i].pos) > 2*ideal_radio_range) )
			return false;
	}

	//EV << "valid sample" << endl;
	return true;
}

void MCB::draw_samples_from_anchor_box(int flag){
	// Draw the samples solely from the sensor's bounding box, and perform the filtering step.
	// Do so for every anchor in the one-hop and two-hop list, till you get the required number of samples.

	// Drawing samples from the one-hop anchor list
	if ((no_of_anchors == 0) && (two_hop_anchor_list.size() == 0) )
	{
		EV << "no anchors or neighbors to draw samples from" << endl;
	}
	//list<NodeInfo *>::const_iterator anchor_list;
	//for (anchor_list = anchors.begin(); anchor_list != anchors.end();anchor_list ++)
	for (int i = 0; i < one_hop_anchor_list.size() ;i ++)
	{
		//EV << "IN...draw_samples_from_anchor_box" << endl;
		if (flag == 0)
			for (int no = 0; no < 100 / one_hop_anchor_list.size() ; no++)
			{
				// Generate a random sample.
				Coord random_sample = generate_random_sample(one_hop_anchor_list[i],radio_range);
				if (filter_with_anchors(random_sample) == true)
				{
					// Update the sample list
					new_sample_list[new_number_of_samples] = random_sample;
					new_number_of_samples++;
	
					// Get out, if you have enough samples.
					if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
						break;
				}
			}
		else
			for (int no = 0; no < 100 ; no++)
			{
				// Generate a random sample.
				Coord random_sample = generate_random_sample(one_hop_anchor_list[i],radio_range);
				if (filter_with_anchors(random_sample) == true)
				{
					// Update the sample list
					new_sample_list[new_number_of_samples] = random_sample;
					new_number_of_samples++;

					// Get out, if you have enough samples.
					if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
						break;
				}
			}
		
		// Get out, if you have enough samples.
		if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
			break;
	}

	if (new_number_of_samples < MIN_SAMPLES_REQUIRED)
	{
		// Draw samples from the two-hop anchor list
		for (int i = 0; i < two_hop_anchor_list.size(); i++)
		{
			if (flag == 0)
				for (int no = 0; no < 100 / two_hop_anchor_list.size() ; no++)
				{
					// Generate a random sample.
					Coord random_sample = generate_random_sample(two_hop_anchor_list[i].pos,2*radio_range);
					if (filter_with_anchors(random_sample) == true)
					{
						// Update the sample list
						new_sample_list[new_number_of_samples] = random_sample;
						new_number_of_samples++;
		
						// Get out, if you have enough samples.
						if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
							break;
					}
				}
			else
				for (int no = 0; no < 100 ; no++)
				{
					// Generate a random sample.
					Coord random_sample = generate_random_sample(two_hop_anchor_list[i].pos,2*radio_range);
					if (filter_with_anchors(random_sample) == true)
					{
						// Update the sample list
						new_sample_list[new_number_of_samples] = random_sample;
						new_number_of_samples++;		
						// Get out, if you have enough samples.
						if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
							break;
					}
				}

			// Get out, if you have enough samples.
			if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
				break;
		}
		
	}
}

void MCB::localize_MCB(){
	if (no_of_neighbours != 0 || no_of_anchors != 0){
	//if (no_of_anchors > 0){
		EV << "no of samples before localization round is " << number_of_samples << endl;
		if (number_of_samples < MIN_SAMPLES_REQUIRED)
		{
			draw_samples_from_anchor_box(0);
		}
		else
		{
			// Draw samples around the earlier samples, and perform the filtering step.
			// Added the following check to generalise MCB for static sensor localization
			int velocity = VMAX;
			if (velocity != 0)
			{
				EV << "velocity-box" << endl;
				for (int no = 0; no < MIN_SAMPLES_REQUIRED; no++)
				{
					for (int i = 0;i< number_of_samples; i++)
					{
						Coord random_sample = generate_random_sample(sample_list[i],VMAX);
			
						if (filter_with_anchors(random_sample) == true)
						{
							// Update the sample list
							new_sample_list[new_number_of_samples] = random_sample;
							new_number_of_samples++;
			
							// Get out, if you have enough samples.
							if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
								break;
						}
					}
		
					// Get out, if you have enough samples.
					if (new_number_of_samples == MIN_SAMPLES_REQUIRED)
						break;
			
				}	
			}
	
			// Draw from anchor box, if the number of samples is still insufficient.
	
			if (new_number_of_samples < MIN_SAMPLES_REQUIRED){
				EV << "insufficient velocity box sampling" << endl;
				draw_samples_from_anchor_box(1);
				EV << "samples collected after velocity and anchor box sampling: " << new_number_of_samples << endl;
			}
		}
		
		// Update sample_list.
		number_of_samples = new_number_of_samples;
		if (number_of_samples != 0)
		{
			for (int i = 0; i < number_of_samples; i++)
				sample_list[i] = new_sample_list[i];
		
			// Resetting the number of samples, for the next round of localization.
			new_number_of_samples = 0;
		
			// Calculating the estimated position as the avergae of the drawn samples
			double x_est = 0;
			double y_est = 0;
			for (int i = 0; i < number_of_samples; i++)
			{
				x_est = x_est + sample_list[i].getX();
				y_est = y_est + sample_list[i].getY();
			}	
		
			computed_pos.setX(x_est / number_of_samples);
			computed_pos.setY(y_est / number_of_samples);
		}

		EV << "no of samples after localization round is " << number_of_samples << endl;
		display_results();
	}
}


void MCB::display_results(){
	
	BaseUtility *utility = (BaseUtility *) 
				(findHost()->submodule("utility"));
	Coord pos = utility->getPos();

	double xReal = pos.getX();
	double yReal = pos.getY();

	// Compute one's distance
	double xcalc = computed_pos.getX();
	double ycalc = computed_pos.getY();


	if ( xcalc != -1 || ycalc != -1 )
	{
		{
			EV << "Node "<<id<<" final estimated position is: ("<<xcalc<<","<<ycalc<<","<<0<<")" << endl;
	
			double error = sqrt(pow(xcalc - xReal,2) + pow(ycalc - yReal,2));
			struct error_info ei;
			ei.error = error;
			ei.ts= simTime();
			ei.number_of_samples = number_of_samples;
			ei.number_of_anchors = no_of_anchors;
			if (localization_error.size() > 0)
			{
				if (ei.ts > localization_error[localization_error.size() - 1].ts)
					localization_error.push_back(ei);
			}
			else
				localization_error.push_back(ei);

			if ((xcalc < self_box.xmin) || (xcalc > self_box.xmax) ||
				(ycalc < self_box.ymin) || (ycalc > self_box.ymax) )
			{
				struct outlier_info outlier_pos = {self_box,computed_pos,simTime()};
				outlier.push_back(outlier_pos);
			}
		
			EV << "LOCALIZATION ERROR IS " << error << endl;
		
			EV << "COMPUTATION_TIME IS " << time_elapsed << endl;
	
			EV << "No of ITERATIONS IS " << no_of_iterations <<  endl;
		}
	}
	else
	{
		EV << "bounding box for Node " << id << endl;
		EV << "xmin = " << self_box.xmin;
		EV << "\t xmax = " << self_box.xmax << endl;
		EV << "ymin = " << self_box.ymin;
		EV << "\t ymax = " << self_box.ymax << endl;
	}

	EV << "NO OF ANCHORS HEARD IS " << no_of_anchors << endl;

	EV << "NO OF NEIGHBOURS HEARD IS " << no_of_neighbours << endl;
	EV << "NO OF NODE MESSAGES SENT IS " << no_of_node_msgs << endl;
}



void MCB::newAnchor(cMessage * msg) {
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
	if (real_dist <= r + int(double ((R - r) * rand())/(RAND_MAX + 1.0)) ) {
		total_anchor_msgs ++;
		anchors.push_back(node);
		one_hop_anchor_list.push_back(node->pos);
		no_of_anchors++;
	
	
		// Build the anchor box, for the last anchor.
		
		struct bounding_box anchor_box;
	
		anchor_box.xmin = (node->pos).getX() - radio_range;
		anchor_box.xmax = (node->pos).getX() + radio_range;
		anchor_box.ymin = (node->pos).getY() - radio_range;
		anchor_box.ymax = (node->pos).getY() + radio_range;
	
		rebuild_box_one_hop(anchor_box);
	}
}

void MCB::newNeighbor(cMessage * msg) {
	LocPkt * m = dynamic_cast<LocPkt *>(msg);

	EV << "message info :" << m->info() << endl;
	EV << "index is " << m->findPar("estimateX") << endl;

		// setting the estimated position.
		NodeInfo * node = new NodeInfo(m, getPosition());
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
		if (real_dist <= r + int(double ((R - r) * rand())/(RAND_MAX + 1.0)) ) {
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
	
						anchor_box.xmin = tmp_array[i].pos.getX() - (2 * radio_range);
						anchor_box.xmax = tmp_array[i].pos.getX() + (2 * radio_range);
						anchor_box.ymin = tmp_array[i].pos.getY() - (2 * radio_range);
						anchor_box.ymax = tmp_array[i].pos.getY() + (2 * radio_range);
					
						

						// Rebuild the bounding box with the two hop information. The filtering phase will reject the invalid samples later on.
						if (rebuild_box_two_hop(anchor_box,tmp_array[i].id))
							two_hop_anchor_list.push_back(tmp_array[i]);						
					}
				}

				delete [] tmp_array;

			}
		}

//	}
}

void MCB::rebuild_box_one_hop(struct bounding_box box){
	// Compute the intersection of the existing bounding box and the anchor box.
	if (boxes_intersect(box)){
		// Intersecting the two boxes
		EV << "INTERSECTION FOUND" << endl;
		self_box.xmin = max(self_box.xmin,box.xmin);
		self_box.ymin = max(self_box.ymin,box.ymin);
	
		self_box.xmax = min(self_box.xmax,box.xmax);
		self_box.ymax = min(self_box.ymax,box.ymax);	
	}
	else
	{
		// Copy the box. If box is an anchor box, then it is always consistent.
		EV << "bounding boxes do not intersect" << endl;
		anchors.clear();
		double x_pos = (box.xmin + box.xmax)/2;
		double y_pos = (box.ymin + box.ymax)/2;
		// If the boxes do not intersect, then the previous bounsing box is very likely an obsolete one, and has to be cleared.
		one_hop_anchor_list.clear();
		// Retain only the current anchor information.
		one_hop_anchor_list.push_back(Coord(x_pos,y_pos));
		no_of_anchors = 1;
		// Clear the two hop anchor list as well. We do not want inconsistent two-hop anchors wrecking the filter process.
		two_hop_anchor_list.clear();
		EV << "self.xmin = " << self_box.xmin;
		EV << "\t self.xmax = " << self_box.xmax << endl;
		EV << "self.ymin = " << self_box.ymin;
		EV << "\t self.ymax = " << self_box.ymax << endl;

		EV << "box.xmin = " << box.xmin;
		EV << "\t box.xmax = " << box.xmax << endl;
		EV << "box.ymin = " << box.ymin;
		EV << "\t box.ymax = " << box.ymax << endl;

		world = FindModule<BaseWorldUtility *>::findGlobalModule();

		const Coord *playgnd =  world->getPgs();


		self_box.xmin = max(0,box.xmin);
		self_box.ymin = max(0,box.ymin);
	
		self_box.xmax = min(playgnd->getX(),box.xmax);
		self_box.ymax = min(playgnd->getY(),box.ymax);	
	}
}

bool MCB::completely_resides_in_box(struct bounding_box box){
	struct bounding_box tmp = {	box.xmin + ideal_radio_range,
								box.ymin + ideal_radio_range,
								box.xmax - ideal_radio_range,
								box.ymax - ideal_radio_range};

	if (self_box.xmin != max(self_box.xmin,tmp.xmin))
		return false;
	if (self_box.ymin != max(self_box.ymin,tmp.ymin))
		return false;
	if (self_box.xmax != min(self_box.xmax,tmp.xmax))
		return false;
	if (self_box.ymax != min(self_box.ymax,tmp.ymax))
		return false;
	
	return true;
}

bool MCB::rebuild_box_two_hop(struct bounding_box box, unsigned id){
	// Compute the intersection of the existing bounding box and the anchor box.
	if (boxes_intersect(box)){
		// If the node's bounsing box is completely inside the two hop anchor box (within a radius in distance), then discard the two hop anchor information.
		if (completely_resides_in_box(box))
		{
			// Discard the two hop anchor information, as it is inconsistent.
			return false;
		}
		else
		{
			// Intersecting the two boxes
			EV << "INTERSECTION FOUND" << endl;
			self_box.xmin = max(self_box.xmin,box.xmin);
			self_box.ymin = max(self_box.ymin,box.ymin);
		
			self_box.xmax = min(self_box.xmax,box.xmax);
			self_box.ymax = min(self_box.ymax,box.ymax);	
		}
	}
	else
	{
		// Copy the box. If box is an anchor box, then it is always consistent.
		EV << "bounding boxes do not intersect" << endl;
	
		double x_pos = (box.xmin + box.xmax)/2;
		double y_pos = (box.ymin + box.ymax)/2;
		// Clear both one hop and to hop anchor lists, and start afresh with the anchor box building.
		one_hop_anchor_list.clear();
		two_hop_anchor_list.clear();

		EV << "self.xmin = " << self_box.xmin;
		EV << "\t self.xmax = " << self_box.xmax << endl;
		EV << "self.ymin = " << self_box.ymin;
		EV << "\t self.ymax = " << self_box.ymax << endl;

		EV << "box.xmin = " << box.xmin;
		EV << "\t box.xmax = " << box.xmax << endl;
		EV << "box.ymin = " << box.ymin;
		EV << "\t box.ymax = " << box.ymax << endl;

		world = FindModule<BaseWorldUtility *>::findGlobalModule();

		const Coord *playgnd =  world->getPgs();

		self_box.xmin = max(0,box.xmin);
		self_box.ymin = max(0,box.ymin);
	
		self_box.xmax = min(playgnd->getX(),box.xmax);
		self_box.ymax = min(playgnd->getY(),box.ymax);	
	}

	return true;
}

int MCB::boxes_intersect(struct bounding_box box){
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

double MCB::max(double a, double b){
	if (a > b) return a; else return b;
}

double MCB::min(double a, double b){
	if (a < b) return a; else return b;
}



void MCB::handleMsg(cMessage* msg){
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
						// The message has come from a non-Anchor. 
						if (! isAnchor)
							newNeighbor(msg);
							;
						delete msg;
						break;
	}

}



/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void MCB::sendBroadcast(LocPkt *pkt)
{
    pkt->setLength(headerLength);
    // set the control info to tell the network layer the layer 3 address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void MCB::display_anchor_pos(){
	unsigned i;

	for (i = 0; i < anchor_pos.size() ; i++)
		EV << "Anchor pos = " << anchor_pos[i].getX() << " " << anchor_pos[i].getY() << endl;
}

void MCB::display_error_vector(){
	for (int i = 0; i< localization_error.size(); i++)
	{
		EV << "error " << i << " " << localization_error[i].error << endl;
		EV << "ts " << i << " " << localization_error[i].ts << endl;
		EV << "number of samples " << i << " " << localization_error[i].number_of_samples << endl;
		EV << "number of anchors " << i << " " << localization_error[i].number_of_anchors << endl;
	}

	for (int i = 0; i< outlier.size(); i++){
		EV << "box xmin: " << outlier[i].box.xmin << "\t box xmax: " << outlier[i].box.xmax << endl;
		EV << "box ymin: " << outlier[i].box.ymin << "\t box ymax: " << outlier[i].box.ymax << endl;

		EV << "computed position: (" << outlier[i].pos.getX() << "," << outlier[i].pos.getY() << ")" << endl;
		EV << "timestamp: " << outlier[i].ts << endl;
	}

	EV << "Total anchors heard: " << total_anchor_msgs << endl;
	EV << "Number of rounds: " << localization_error.size() << endl;
}

void MCB::finish(){
	if (! isAnchor){
		display_results();
		display_error_vector();
	}else
		display_anchor_pos();
	BaseLocalization::finish();
	EV << "\t\tEnding MCB localization...\n";	
}



