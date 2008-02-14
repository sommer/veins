/* -*- mode:c++ -*- ********************************************************
 * file:        MCB.h
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
 * description: MCB class for the localization module
 **************************************************************************/

#ifndef MCB_H
#define MCB_H

#include "BaseLocalization.h"
#include "Location.h"
#include "LocPkt_m.h"
#include "Coord.h"
#include "Timer.h"
#include "Position.h"
#include "BaseWorldUtility.h"




/* Should be in a network configuration file */
// Time interval between anchor broadcasts.
#define ANCHOR_TIMER_INTERVAL 0.5
// Time interval between node exchanges
#define NODE_TIMER_INTERVAL	0.25
// Time slot added to allow for two hop message processing
#define START_LOCALIZATION_TIMER_INTERVAL 0.25
#define MIN_SAMPLES_REQUIRED 50
#define DOI 0.0
#define RANDOM_RANGE(R,r)	int(R - r)
// Velocity of the mobile sensor.
#define VMAX 50


using std::vector;

typedef enum { SEND_ANCHOR_POS_TIMER = 0,
	       SEND_NODE_LOCALIZATION_TIMER, 			START_LOCALIZATION_TIMER } MCBTimer;
//					    ANCHOR_PAUSE_TIMER,
// The Anchor Table, containing the following information:
// 	unsigned int 	id;
//	Coord 		pos;
// 	double		power_level;
// For convenience, the power level is taken as the distance.
 

/**
 * @brief MCB class for the localization module
 *
 * @author Aline Baggio
 */

struct bounding_box{
	double	xmin;
	double	ymin;
	double 	xmax;
	double 	ymax;
};


struct Anchor_Table{
	unsigned id;
	Coord pos;
};


// Error information captured at every localization round. This holds relevance for mobile sensor networks.
struct error_info {
	double error;
	unsigned number_of_samples;
	unsigned number_of_anchors;
	simtime_t ts;
};

// A paceholder for outliers in the localization results.
struct outlier_info{
	struct bounding_box box;
	Coord pos;
	simtime_t ts;
};

class MCB:public BaseLocalization, public Timer {
      public:
	Module_Class_Members(MCB, BaseLocalization, 0);

	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int);
	virtual void finish();
	void newAnchor(cMessage *);
	
	void newNeighbor(cMessage *);

	bool completely_resides_in_box(struct bounding_box);
	void rebuild_box_one_hop(struct bounding_box);
	bool rebuild_box_two_hop(struct bounding_box, unsigned id);
	int boxes_intersect(struct bounding_box anchor_box);
	double max(double a, double b);
	double min(double a, double b);
	Coord generate_random_sample(Coord &,double);
	bool filter_with_anchors(Coord);
	void draw_samples_from_anchor_box(int);
	void localize_MCB();
	void display_results();
	void display_anchor_pos();
	void display_error_vector();


protected:


    	double final_x,final_y,final_z;
    	double timer_count;
    	double r, R,radio_range,ideal_range,sensor_range, ideal_sensor_range,time_elapsed,variance,ideal_radio_range;
    	int	 no_of_anchors,no_of_neighbours, total_neighbour_msgs,total_anchor_msgs,no_of_iterations,no_of_node_msgs;
	vector<struct Anchor_Table> two_hop_anchor_list;
	vector<Coord>	anchor_pos,one_hop_anchor_list;
    	BaseWorldUtility *world;
	vector<error_info> localization_error;
	vector<outlier_info> outlier;
	struct bounding_box 	self_box;
	int number_of_samples,new_number_of_samples;
	Coord sample_list[MIN_SAMPLES_REQUIRED],new_sample_list[MIN_SAMPLES_REQUIRED];
	Coord computed_pos;



  enum { ANCHOR_BROADCAST_MESSAGE = APPLICATION_MSG + 1,
	 NODE_BROADCAST_MESSAGE};

 
protected:
    /** @brief Handle self messages such as timer... */
//    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    //virtual void handleLowerMsg(cMessage*);
  virtual void handleMsg(cMessage*);

  
    /** @brief send a broadcast packet to all connected neighbors */
    void sendBroadcast(LocPkt *pkt);

    /** @brief send a reply to a broadcast message */
    //void sendReply(ApplPkt *msg);  

	


    virtual void handleTimer(unsigned int count);
};

#endif				/* MCB_H */
