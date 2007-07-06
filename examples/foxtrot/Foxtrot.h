/* -*- mode:c++ -*- ********************************************************
 * file:        Foxtrot.h
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
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
 * part of:     wsn-specific modules
 * description: aggregation layer: Foxtrot with original spacial merging
 ***************************************************************************/


#ifndef FOXTROT_LAYER_H
#define FOXTROT_LAYER_H

#include "BaseAggLayer.h"
#include "FrameTimer.h"
#include "Timer.h"

#include "AggPkt_m.h"
#include "SimpleAddress.h"
#include "FoxtrotPacket.h"
#include "FoxtrotPacketStorage.h"

#include "foxtrot_types.h"

#ifndef COMPILER_ASSERT
#define COMPILER_ASSERT(expr, msg) typedef int ERROR_##msg[1][(expr) ? 1 : 0][(expr) ? 1 : -1];
#endif

#define FOX_QUEUE_LEN 6
#define FOX_MAX_MERGE (FOX_QUEUE_LEN+1)
#define FOX_QUEUE_LEN_BYTES 1

// the above should have space for at least FOX_MAX_MERGE bits
// This is checked below
COMPILER_ASSERT(FOX_QUEUE_LEN_BYTES * 8 >= FOX_MAX_MERGE, error_FOX_QUEUE_LEN_BYTES_TOO_SMALL)
#define MAX_PTS 3
#define MAX_SECS_DIFF 3
#define X_DIR 0
#define Y_DIR 1
#define MAX_SECONDS 0xffffffff
#define MAX_FOXTROT_VALUE 0xffff
/**
 * @brief Foxtrot layer
 * 
 * @ingroup AggLayer
 * @author Tom Parker
 **/
class Foxtrot:public BaseAggLayer, public FrameTimer, public Timer
{
  public:

	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int);
	~Foxtrot();

  protected:
	 Module_Class_Members(Foxtrot, BaseAggLayer, 0);
	foxtrot_point limits[MAX_PTS];

	FoxtrotPacketStorage storage;
	FoxtrotPacket *local;
	uint8_t items;

	/** @brief Handle messages from upper layer */
	virtual void handleUpperMsg(cMessage * msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage * msg);

	virtual bool merge(uint8_t * subset, uint8_t * delete_count);

	void try_merge(FoxtrotPacket * newData, bool keep_new);	/* keep_new indicates whether to keep new data if it doesn't merge */
	static inline void gen_data(foxtrot_data * d, const foxtrot_point * p);
	void newPoint(const foxtrot_point * data);
	virtual void newData(const foxtrot_data * data);
	uint8_t mergeableSubset(FoxtrotPacketStorage * list, uint8_t * subset);
	//inline void print_data(const char* beg, const foxtrot_data* packet, seconds_t when);
	void print_data(const char *beg, const FoxtrotPacket * packet) const
	{
		packet->print(beg);
	}
	void processPackets();
	static inline bool val_in_range(const foxtrot_data * a, const foxtrot_point pt);
	static inline bool all_in_range(const foxtrot_data * a, const foxtrot_data * b);
	inline bool inside(const FoxtrotPacket * outer, const FoxtrotPacket * inner);
	seconds_t getGlobalSeconds();

	static uint8_t pts;
	bool wait_data;
	uint16_t maxLatency;

	virtual void handleTimer(unsigned int count);
	virtual void handleFrameTimer(unsigned int index);
	void doSend();

	virtual void dataUp(FoxtrotPacket * pkt)
	{
		sendUp(pkt);
	}

};

#endif
