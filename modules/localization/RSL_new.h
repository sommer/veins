/* -*- mode:c++ -*- ********************************************************
 * file:        RSL_new.h
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
 * description: RSL_new class for the localization module
 **************************************************************************/

#ifndef RSL_new_H
#define RSL_new_H

#include "BaseLocalization.h"
#include "Location.h"
#include "LocPkt_m.h"
#include "Coord.h"
#include "Timer.h"
#include "Position.h"
#include "BaseWorldUtility.h"




/* Should be in a network configuration file */
#define ANCHOR_TIMER_INTERVAL 1.0
#define NODE_TIMER_INTERVAL 1.0
#define ANCHOR_TIMEOUT_INTERVAL 5.0
#define ANCHOR_THRESHOLD_TIME 35.0
#define NODE_THRESHOLD_TIME 35.0

#define DOI 0.0
#define RANDOM_RANGE(R,r)	int(R - r)
#define PERCENTAGE_VARIANCE_IN_MEAN 0.0

using std::vector;

typedef enum { SEND_ANCHOR_POS_TIMER = 0,
	       SEND_NODE_LOC_TIMER, ANCHOR_TIMEOUT } RSL_newTimer;

// The Anchor Table, containing the following information:
// 	unsigned int 	id;
//	Coord 		pos;
// 	double		power_level;
// For convenience, the power level is taken as the distance.
 

/**
 * @brief RSL_new class for the localization module
 *
 * @author Aline Baggio
 */

struct bounding_box{
	double	xmin;
	double	ymin;
	double 	xmax;
	double 	ymax;
};

struct node_info{
	unsigned 		int id;
	double			confidence;
	Coord 			estimate1;
	Coord 			estimate2;
	struct bounding_box	self_box;
};




class RSL_new:public BaseLocalization, public Timer {
      public:
	Module_Class_Members(RSL_new, BaseLocalization, 0);

	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int);
	virtual void finish();
	void newAnchor(cMessage *);
	
	void newNeighbor(cMessage *);

	void rebuild_box(struct bounding_box);
	int boxes_intersect(struct bounding_box anchor_box);
	double max(double a, double b);
	double min(double a, double b);
	void compute_position(int flag);
	Coord   localize_with_mean(int flag);
	Coord   localize_with_mode(int flag);
	bool 	equal_pairs(Coord avg);
	double gaussian(double pt_distance, double real_dist, double variance);
	void display_results();
	int calc_irregularity();
	double compute_dist(double,double);
	void display_error_vector();

protected:


    	double final_x,final_y,final_z;
    	double timer_count;
    	double r, R,radio_range,ideal_range,sensor_range,ideal_sensor_range,ideal_radio_range, time_elapsed,variance;
    	int	 no_of_anchors,no_of_neighbours, total_no_of_anchors, total_neighbour_msgs,total_anchor_msgs,no_of_iterations;
		double **PostEst;
		vector<double> localization_error;
    	BaseWorldUtility *world;

	struct node_info 	self_info;



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

#endif				/* RSL_new_H */
