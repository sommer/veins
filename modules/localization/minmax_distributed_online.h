/* -*- mode:c++ -*- ********************************************************
 * file:        minmax_distributed_online.h
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
 * description: minmax_distributed_online class for the localization module
 **************************************************************************/

#ifndef minmax_distributed_online_H
#define minmax_distributed_online_H

#include "BaseLocalization.h"
#include "Location.h"
#include "LocPkt_m.h"
#include "Coord.h"
#include "Timer.h"
#include "Position.h"
#include "BaseWorldUtility.h"

/* Should be in a network configuration file */
#define ANCHOR_TIMER_INTERVAL 0.5
#define NODE_TIMER_INTERVAL 0.25
#define ANCHOR_THRESHOLD_TIME 35
#define NODE_THRESHOLD_TIME 0.5
#define MIN_ANCHOR_POSITIONS 1 // 2D
#define RADIO_RANGE		0
#define DOI 0.0
#define RANDOM_RANGE(R,r)	int(R - r)

typedef enum { SEND_ANCHOR_POS_TIMER = 0,
	       SEND_NODE_LOC_TIMER } minmax_distributed_onlineTimer;

/**
 * @brief minmax_distributed_online class for the localization module
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

using std::vector;

class minmax_distributed_online:public BaseLocalization, public Timer {
      public:
	Module_Class_Members(minmax_distributed_online, BaseLocalization, 0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

struct error_info {
	double error;
	unsigned number_of_anchors;
	simtime_t ts;
};

protected:
    LocPkt *anchorTimer;
    LocPkt *nodeTimer;
    int nb_anchor_positions,nr_dims,total_anchor_msgs,total_neighbour_msgs,no_of_anchors,no_of_neighbours,no_of_node_msgs;
    double *dim, timer_count;
    double radio_range,sensor_range,ideal_radio_range,ideal_sensor_range,R,r;
    BaseWorldUtility *world;
	vector<struct error_info > localization_error;
    struct bounding_box self_box;	
	double final_x,final_y,final_z;


  enum { ANCHOR_BROADCAST_MESSAGE = APPLICATION_MSG + 1,
	 NODE_BROADCAST_MESSAGE};

 
protected:

	void wind_up();
	double max(double a, double b){
		if (a > b) return a; else return b;
	}

	double min(double a, double b){
		if (a < b) return a; else return b;
	}
	
	int boxes_intersect(struct bounding_box box);

	void rebuild_box(struct bounding_box box);

	void update_box_with_negative_info(struct bounding_box anchor_box);
	void display_error_vector();

	int calc_irregularity();
	void newAnchor(cMessage *);

	void newNeighbor(cMessage *);

    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    //virtual void handleLowerMsg(cMessage*);
  virtual void handleMsg(cMessage*);

  
    /** @brief send a broadcast packet to all connected neighbors */
    void sendBroadcast(LocPkt *pkt);

    virtual void handleTimer(unsigned int count);
};

#endif				/* minmax_distributed_online_H */
