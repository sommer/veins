/* -*- mode:c++ -*- ********************************************************
 * file:        apit_mob.h
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
 * description: apit_mob class for the localization module
 **************************************************************************/

#ifndef apit_mob_H
#define apit_mob_H

#include "BaseLocalization.h"
#include "Location.h"
#include "LocPkt_m.h"
#include "Coord.h"
#include "Timer.h"
#include "Position.h"
#include "BaseWorldUtility.h"
#include <vector>



/* Should be in a network configuration file */
#define ANCHOR_TIMER_INTERVAL 0.9384
#define NODE_TIMER_INTERVAL 0.9384
#define ANCHOR_THRESHOLD_TIME		20.0
#define NODE_THRESHOLD_TIME	20.0
#define DISTANCE(d1,nl,p,i) 	double d1 = neighbour_t[nl].anchor_ptr[p[i]].power_level;
#define SELF_DISTANCE(self_d1,i) double self_d1 = anchor_t[i].power_level;
#define GET_ANCHOR(anchor1,x)	Coord anchor1 = anchor_t[x].pos;
#define IS_IN_GRID(anchor1)	((anchor1.getX() >= GETCOORD(g1,grid_size) - (grid_size/2)) && (anchor1.getX() <  GETCOORD(g1,grid_size) + (grid_size/2)) && (anchor1.getY() >= GETCOORD(g2,grid_size) - (grid_size/2)) && (anchor1.getY() <  GETCOORD(g2,grid_size) + (grid_size/2)) )
#define CORNER_IN_TRAINGLE(c1)		!( (c1.distance(anchor1) > centroid.distance(anchor1) && c1.distance(anchor2) > centroid.distance(anchor2) && c1.distance(anchor3) > centroid.distance(anchor3)) || (c1.distance(anchor1) <= centroid.distance(anchor1) && c1.distance(anchor2) <= centroid.distance(anchor2) && c1.distance(anchor3) <= centroid.distance(anchor3) ) )

#define DOI 0.0
#define RANDOM_RANGE(R,r)	int(R - r)
#define GETCOORD(pos,grid_size)	(pos + 0.5)* grid_size
#define PERCENTAGE_VARIANCE_IN_MEAN 0.0

using std::vector;

typedef enum { SEND_ANCHOR_POS_TIMER = 0,
	       SEND_NODE_LOC_TIMER } apit_mobTimer;

// The Anchor Table, containing the following information:
// 	unsigned int 	id;
//	Coord 		pos;
// 	double		power_level;
// For convenience, the power level is taken as the distance.
 
struct Anchor_Table{
//	Anchor_Table(int i, Coord p, double pl): id(i), pos(p), power_level(pl){}

 	unsigned int 	id;
	Location		pos;
 	double			power_level;
};

struct neighbour_info{
	unsigned int id;					// Id of the sensor
	unsigned int no_of_anchors;			// No of anchors that the neighbouring sensor heard
	vector<Anchor_Table> anchor_ptr;	// The sensor's Anchor Table.
};


// SET of triples of anchors that bound an unknown within the triange formed between them.
struct pit_struct{
		unsigned int pos1,pos2,pos3;
		bool inside;
};


/**
 * @brief apit_mob class for the localization module
 *
 * @author Aline Baggio
 */

class apit_mob:public BaseLocalization, public Timer {
      public:
	//Module_Class_Members(apit_mob, BaseLocalization, 0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();
    void PIT_test();
    int * hasAnchors(int,int,int,int);
    int in_triangle(Coord c, Coord anchor1, Coord anchor2, Coord anchor3);
    int tri_test(Coord pos,Coord a1,Coord a2,Coord a3);
	Coord compute_position(vector<struct pit_struct> ,int);

	int calc_irregularity();
	double compute_dist(double real_dist, double dist_variance);
	void newAnchor(cMessage *);

	void newNeighbor(cMessage *);

//	double GETCOORD(int pos, double grid_size);

protected:
    

    double final_x,final_y,final_z;
    double timer_count,time_elapsed;
	double r, R,radio_range,sensor_range;
    vector<Anchor_Table> anchor_t;
    vector<neighbour_info> neighbour_t;
    int	 no_of_anchors,no_of_neighbours;
	BaseWorldUtility *world;


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

#endif				/* apit_mob_H */
