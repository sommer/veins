/* -*- mode:c++ -*- ********************************************************
 * file:        CollisionsModel.h
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
 * description: propagation layer: single cell with message-length delays and collisions
 *              Assumes propagation delay is *ZERO*
 ***************************************************************************/
#ifndef COLLISIONS_MODEL_H
#define COLLISIONS_MODEL_H 1

#include "BasePropagation.h"
#include "Timer.h"

class CollisionsModel: public BasePropagation, public Timer
{
	protected:
		std::map<unsigned int,std::pair<int,AirFrame *>* > *active;
	public:
	    Module_Class_Members(CollisionsModel, BasePropagation, 0);
		void initialize(int stage);
		~CollisionsModel();
		virtual void sendToChannel(BasePhyLayer *m,AirFrame *msg);
		virtual void handleTimer(unsigned int index);
    	virtual void registerNic( BasePhyLayer*);
		virtual NodeList * canHear(BasePhyLayer*);
};

#endif
