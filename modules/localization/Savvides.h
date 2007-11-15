/* -*- mode:c++ -*- ********************************************************
 * file:        Savvides.h
 *
 * author:      Peterpaul Klein Haneveld <pp_kl_h@hotmail.com>
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
 * description: savvides class for the localization module
 **************************************************************************/

#ifndef SAVVIDES_H
#define SAVVIDES_H

#include "BaseLocalization.h"
#include "PositifUtil.h"
#include "Timer.h"
#include "Move.h"

#include <list>
using std::list;
#include <string>
using std::string;

#define msec		(1e-3)

#define ACCEPT		0.1
#define CONF_THR	(LOW_CONF*1.01)

#define ZERO_CONF       0.01
#define LOW_CONF        0.1

typedef enum {
	STATUS_ANCHOR = 0 /* This node has a known position */ ,
	STATUS_UNKNOWN =
	    1 /* This node has not yet been able to estimate a position */ ,
	STATUS_POSITIONED = 2 /* This node has estimated its position */ ,
	STATUS_BAD = 3		/* This is a bad node (meaning will depend on the algorithm) */
} NodeState;

#define add_struct(m,s,v)						\
	do {								\
		(m)->addPar(s) = (void *)memmove			\
				 (new char[sizeof(*(v))],(v),		\
				  sizeof(*(v)));			\
		(m)->par(s).configPointer(NULL,NULL,sizeof(*(v)));	\
	} while (0)
#define get_struct(m,s,p)	memmove((p),(m)->par(s),sizeof(*(p)))

#define add_array(m,s,p,n)						\
	do {								\
		(m)->addPar(s) = (void *)memmove			\
				 (new char[(n)*sizeof(*(p))],(p),	\
				  (n)*sizeof(*(p)));			\
		(m)->par(s).configPointer(NULL,NULL,(n)*sizeof(*(p)));	\
	} while (0)
#define get_array(m,s,p,n)	memmove( (p), (m)->par(s), (n)*sizeof(*(p)))

#define PUSH_BACK(list,item) 						\
	{								\
		EV << "Pushing " << item->info() << " into list: " 	\
		   << #list << endl;					\
		list.push_back(item);					\
	}


class AnchorInfo: public NodeInfo {
public:
	AnchorInfo()
		: NodeInfo(0, false, Location(), 0.0),
		  path_dst(0.0) {}
	AnchorInfo(int i, bool b, Location l, double d)
		: NodeInfo(i,b,l,d),
		  path_dst(d) {}
	AnchorInfo(NodeInfo * n)
		: NodeInfo(*n),
		  path_dst(n->distance) {}
	double path_dst;
	int last_hop_idx;
	int cnt;
	bool flood;

	virtual std::string info() const {
		std::stringstream os;
		os << NodeInfo::info() << ":" << path_dst;
		return os.str();
	}
};

class NeighborInfo: public NodeInfo {
public:
	NeighborInfo()
		: NodeInfo(0, false, Location(), 0.0),
		  nr_nghbrs(0),
		  nghbr_idx(NULL),
		  twin(false) {}
	NeighborInfo(int i, bool b, Location l, double d)
		: NodeInfo(i,b,l,d),
		  nr_nghbrs(0),
		  nghbr_idx(NULL),
		  twin(false) {}
	NeighborInfo(NodeInfo * n)
		: NodeInfo(*n),
		  nr_nghbrs(0),
		  nghbr_idx(NULL),
		  twin(false) {}
	~NeighborInfo() {
		if (nghbr_idx)
			delete nghbr_idx;
	}
	int nr_nghbrs;
	int *nghbr_idx;
	bool twin;
};

typedef struct {
	FLOAT res; 
	FLOAT sum_conf;
	int neighbor_count;
	Coord pos;
} tria_data;

// typedef struct {
// 	Position curr_pos;
// 	NodeState status;
// 	double confidence;
// 	double err;
// 	double phase1_err;
// 	double phase2_err;
// 	double anchor_range_error;
// 	double rel_anchor_range_error;
// 	double abs_anchor_range_error;
// 	double abs_rel_anchor_range_error;
// 	double anchor_range;
// 	int anchor_range_error_count;
// 	int flops;
// 	int *bcast_total;
// 	int *bcast_unique;
// 	int used_anchors;
// 	FLOAT *range_list;
// 	FLOAT *real_range_list;
// } performance_info;

class Savvides:public BaseLocalization, public Timer, public PositifUtil {
protected:
	enum {
		ANCHOR_MSG = APPLICATION_MSG + 1,
		POSITION_MSG,
	};

	enum {
		ANCHOR_TIMER = 0,
		POSITION_TIMER, 
		TRIANGULATION_TIMER,
	};

	/**
	 * @brief Members needed for interaction with mobility framework.
	 */
	/*@{ */
	Move move;
	int moveCategory;
	
	void check_if_moved(void);
	virtual void receiveBBItem(int, const BBItem*, int);
	/*@} */

	list<unsigned int> triangulation_timers;
	list<AnchorInfo *> anc;
// 	list<NeighborInfo *> nb;

	/* Localization event handlers */
	virtual NodeInfo * handleNewAnchor(NodeInfo *);
	virtual NodeInfo * handleNewNeighbor(NodeInfo *);
	/* Messaging methods */
	void sendMsg(cMessage *);
	virtual void handleMsg(cMessage *);
	/* Timer methods */
	virtual void handleTimer(unsigned int);

	virtual int numInitStages() const {
		return 2;
	}
private:
	struct {
		Coord min, max;
	} rectangle;		// convex region I must be in
	bool valid_rectangle;

	static int refine_limit;
	static int flood_limit;
	static int repeats;
	static int tri_alg;
	static int phase1_min_anchors;
	static int phase1_max_anchors;

	static double range;
	static double accuracy;
	/** Time to wait untill anchor_message will be sent */
	static double anchor_timer_interval;
	static double position_timer_interval;
	static double triangulation_timer_interval;

	int refine_count;
	bool summary_update;
	NeighborInfo summary;

// 	performance_info perf_data;
	int status;
	int used_anchors;
	bool i_am_a_twin;
	FLOAT *range_list;
	FLOAT *real_range_list;
	FLOAT residu;

	/* Private algorithm functions */
	bool checkAnchors(cMessage *, NodeInfo *);
	void sendAnchors();

	void checkNeighbors(cMessage *, NodeInfo *);
	void sendPosition();

	void update_rectangle(AnchorInfo * anchor);
	bool inside_rectangle(Coord pos);
	bool inside_neighbors_range(Coord pos);

	void savvides();
	void doTriangulation();
	void delayedDoTriangulation(unsigned int, void *);
	void removeTriangulationTimer(unsigned int);

	bool twins(NeighborInfo *, NeighborInfo *);

public:
	Module_Class_Members(Savvides, BaseLocalization, 0);
	/* Initialization and Finalization of the layer */
	virtual void initialize(int);
	virtual void finish();
};

#endif /* SAVVIDES_H */
