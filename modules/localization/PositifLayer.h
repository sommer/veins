/* -*- mode:c++ -*- ********************************************************
 * file:        PositifLayer.h
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
 * part of:     mixim framework
 * description: localization layer: general class for the network layer
 *              subclass to create your own localization layer
 **************************************************************************/


#ifndef BASELOC_H
#define BASELOC_H

#include <BaseLayer.h>

#include "Position.h"
#include "LocPkt_m.h"
#include "refine.h"
#include "main.h"
#include "RepeatTimer.h"
#include "cores.h"
#include "Coord.h"
#include "BaseWorldUtility.h"

#define ZERO_CONF       0.01
#define LOW_CONF        0.1

#define TOPOLOGYTYPE_LENGTH 25
#define LOGLENGTH 102400
#define MAX_TIMERS 5

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

typedef int timer_info;

typedef struct {
	int idx;
	int seqno[MAX_MSG_TYPES];
} seqno_info;

typedef struct {
	int idx;
	FLOAT true_dist;
	FLOAT est_dist;
} neighbor_info;

typedef struct {
	Position curr_pos;
	int status;
	double confidence;
	double err;
	double phase1_err;
	double phase2_err;
	double anchor_range_error;
	double rel_anchor_range_error;
	double abs_anchor_range_error;
	double abs_rel_anchor_range_error;
	double anchor_range;
	int anchor_range_error_count;
	int flops;
	int *bcast_total;
	int *bcast_unique;
	char log[LOGLENGTH];
	int used_anchors;
	FLOAT *range_list;
	FLOAT *real_range_list;
} performance_info;

typedef struct {
	int ID;			// may be different from index in 'node'
	bool anchor;
	Position true_pos;
	Position init_pos;
	cLinkedList *neighbors;	// broadcast will reach them
	int recv_cnt;		// how many neighbors can I hear
	// Data that will be written by node.cc and read by network to produce
	// performance statistics.
	performance_info perf_data;
	// Can be used by algorithms for various purposes
	bool token;
	bool wants_token;
} node_info;

typedef enum { Anchor, Skip, Unknown, Stuck } node_t;

typedef struct {
	int idx;
	node_t type;
	int wave_cnt;
	FLOAT gain;
	FLOAT conf;
	bool stuck;
	bool bad;
	int twin;
	Position init_pos;
	struct {
		Position pos;
		FLOAT err;
		FLOAT conf;
	} curr, next;
} glob_info;

extern timer_info _timers[MAX_TIMERS];

/**
 * @brief Base class for the localization layer
 * 
 * @ingroup localization
 * @author Peterpaul Klein Haneveld
 **/
class PositifLayer:public BaseLayer, public RepeatTimer {

      protected:
	/**
	 * @brief Length of the LocPkt header 
	 * Read from omnetpp.ini 
	 **/
	int headerLength;
	NodeState status;

      public:
	 Module_Class_Members(PositifLayer, BaseLayer, 0);

	Position position;
	int me;
	int used_anchors;
	FLOAT *range_list;
	FLOAT *real_range_list;
	bool use_log;
	char *log;
	int seqno[MAX_MSG_TYPES];
	int last_sent_seqno[MAX_MSG_TYPES];	// Used to count unique bcasts

	// How confident are we about this position?
	// (May not be used in all algorithms, and values
	// produced by different algorithms cannot be compared)
	double confidence;
	// The number of times a message is repeated to make sure it reaches all nodes even when some are lost because of collision
	int reps;
	// Count how many refine messages were sent
	int refine_count;
	// Count the number of operations
	int flops;

	unsigned int start_timer;

	static int num_nodes;
	static int num_anchors;
	static int algorithm;
	static int version;
	static int nr_dims;
	static node_info *node;
	static FLOAT range;
	static FLOAT area;
	static double *dim;
	static double range_variance;
	static double pos_variance;
	static int flood_limit;
	static int refine_limit;
	static int tri_alg;
	// Whether or not to proceed to a 2nd phase (if it exists)
	static bool do_2nd_phase;
	// The number of anchors to do the initial estimate with
	static int phase1_min_anchors;
	static int phase1_max_anchors;
	static char topology_type[TOPOLOGYTYPE_LENGTH];
	static FLOAT var0;
	static FLOAT var1;
	static FLOAT var2;

	/** @brief Initialization of the module and some variables*/
	virtual void finish(void);
	virtual void initialize(int);
	Coord getPosition();

	int logprintf(__const char *__restrict __format, ...);
	char *pos2str(Position);	// WARNING: uses a static buffer
	void setup_global_vars(void);
	void setup_neighbors(void);
	void setup_grid(void);


	// Timer functions
	timer_info *timer(int reps, int handler, void *arg = NULL);
	void addTimer(timer_info * e);
	void resetTimer(timer_info * e);
	void cancelTimer(timer_info * e);
	virtual void handleRepeatTimer(unsigned int index);
	virtual void handleTimer(timer_info * handler) {
		error
		    ("Subclasses of PositifLayer should implement handleTimer()");
	}
	void send(cMessage * msg);	// synchronous send
	FLOAT distance(Position, Position);
	FLOAT savvides_minmax(int n_pts, FLOAT ** positions,
			      FLOAT * ranges, FLOAT * confs, int target);
	FLOAT triangulate(int n_pts, FLOAT ** positions,
			  FLOAT * ranges, FLOAT * weights, int target);
	FLOAT hoptriangulate(int n_pts, FLOAT ** positions,
			     FLOAT * ranges, int target);


	void write_statistics();
	void write_configuration(const char *);
	void statistics(bool);
      protected:
	/** 
	 * @name Handle Messages
	 * @brief Functions to redefine by the programmer
	 *
	 * These are the functions provided to add own functionality to your
	 * modules. These functions are called whenever a self message or a
	 * data message from the upper or lower layer arrives respectively.
	 *
	 **/
	/*@{ */

	/** @brief Handle messages from upper layer */
	virtual void handleUpperMsg(cMessage * msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage * msg);

	/** @brief Handle self messages */
	virtual void handleSelfMsg(cMessage * msg) {
		error("PositifLayer does not handle self messages");
	};

	/** @brief Handle control messages from lower layer */
	virtual void handleLowerControl(cMessage * msg);

	/** @brief Handle control messages from lower layer */
	virtual void handleUpperControl(cMessage * msg) {
		error("PositifLayer does not handle control messages");
	};

	/*@} */

	/** @brief decapsulate higher layer message from LocPkt */
	virtual cMessage *decapsMsg(LocPkt *);

	/** @brief Encapsulate higher layer packet into an LocPkt*/
	virtual LocPkt *encapsMsg(cMessage *);

	/**
	 * @name Positif Interface methods
	 * @brief Functions to redefine by positif modules
	 *
	 * These methods are interface methods required by positif modules.
	 **/
	/*@{ */
	virtual void init(void) {
	}
	virtual void handleStartMessage(cMessage * msg) {
	}
	virtual void handleMessage(cMessage * msg, bool newNeighbor) {
	}
	virtual void handleStopMessage(cMessage * msg);
	/*@} */

      private:
	bool msg_buffering;
	cQueue putAside;
	static BaseWorldUtility * world;

	static glob_info * ginfo;
	static FILE *scenario;

	void glob_triangulate(glob_info * nd);
	void stats(const char *str, bool details = true);
	void run_algorithms(void);
	void run_terrain(bool stats_only, bool * undetermined);
	void prune_loose_nodes(bool * skip);
	bool collapse_twins(bool * skip);
	void topology_stats(bool * skip);
	void find_bad_nodes(bool * bad);
	void save_scenario(bool * skip, bool * bad);
	void analyzeTopology(void);

	void addNewNeighbor(int, double, double);
	bool isNewNeighbor(int);
	void update_perf_data();
};

#endif
