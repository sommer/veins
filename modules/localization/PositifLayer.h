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

#define LOGLENGTH 102400

/**
 * @brief Type used for positif timer stuff.
 */
// typedef struct {
//      int repeats, cnt;
//      int handler;
//      void *arg;
// } timer_info;

// #define SIM_SUITE_SIZE 31	// number of elements in params->sim_suite array

// extern struct myParams {
// 	int dim;		// dimension of geographical space
// 	int grid_bound;		// maximum coordinate value
// 	int vis;		// radio visibility range
// 	int alg_sel;		// algorithm selection: 0=traditional least squares, 1=qr-ls, 2=svd-ls, 3=MMSE
// 	int start_alg;		// start-up algorithm selection: 0=terrain, 1=hop-terrain
// 	int conf_mets;		// flag: 0=do not use confidence metrics in determining positions, 1=use them
// 	int net_size;		// total number of nodes in network
// 	int anch_size;		// number of anchor nodes in network
// 	int ref_its;		// maximum number of refinement iterations to perform
// 	int range_err_var;	// variance of noise in range measurements (normal distrubution)
// 	int verbosity;		// controls level of detail in report log. 0=summaries only, 1=scenarios and summaries, 2=full detail
// 	int sim_suite[SIM_SUITE_SIZE];	// array containing parameter ranges for simulation suite
// 	FILE *logfile;		// pointer to log file
// 	char filename[32];	// name of log file
// } params;

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

	static int num_nodes;
	static int num_anchors;
	static int algorithm;
	static int version;
	static unsigned int nr_dims;
	static node_info *node;
	static FLOAT range;
	static FLOAT area;
	static double *dim;
	static double range_variance;
	static int flood_limit;
	static int refine_limit;
	static int tri_alg;
	// Whether or not to proceed to a 2nd phase (if it exists)
	static bool do_2nd_phase;
	// The number of anchors to do the initial estimate with
	static int phase1_min_anchors;
	static int phase1_max_anchors;

	/** @brief Initialization of the module and some variables*/
	virtual void finish(void);
	virtual void initialize(int);
	Coord getPosition();

	int logprintf(__const char *__restrict __format, ...);
	char *pos2str(Position);	// WARNING: uses a static buffer
	void setGlobalVars(void);
	void DoNeighbours(void);

	// Timer functions
//      cLinkedList timeouts;
//      timer_info *timer(int reps, int handler, void *arg = NULL);
//      void addTimer(timer_info * e);
//      void resetTimer(timer_info * e);
//      cLinkedListIterator getTimers(void);
//      void invokeTimer(timer_info * e);

	unsigned int timer(int reps, int handler, void *arg = NULL);
	void resetTimer(unsigned int index);


	void send(cMessage * msg);	// synchronous send
	FLOAT distance(Position, Position);
	FLOAT savvides_minmax(unsigned int n_pts, FLOAT ** positions,
			      FLOAT * ranges, FLOAT * confs, int target);
	FLOAT triangulate(unsigned int n_pts, FLOAT ** positions, FLOAT * ranges,
			  FLOAT * weights, int target);
	FLOAT hoptriangulate(unsigned int n_pts, FLOAT ** positions, FLOAT * ranges,
			     int target);

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
//      virtual void handleTimer(timer_info * timer) {}
	virtual void handleStartMessage(cMessage * msg) {
	}
	virtual void handleMessage(cMessage * msg, bool newNeighbor) {
	}
	virtual void handleStopMessage(cMessage * msg);
	/*@} */

	virtual void handleRepeatTimer(unsigned int index) {
	}
};

#endif
