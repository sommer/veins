/* -*- mode:c++ -*- ********************************************************
 * file:        BasePropagation.h
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
 * part of:     base modules
 * description: propagation layer: default single cell model
 ***************************************************************************/

#ifndef BASE_PROPAGATION
#define BASE_PROPAGATION 1

#include "BaseModule.h"
#include "AirFrame_m.h"

#define INGATE "lowergateIn"
class BasePhyLayer;

typedef std::list<BaseModule*> NodeList;

class BasePropagation : public BaseModule
{
public:
protected:
	NodeList *nodes;
	bool coreDebug;

public:
    /**
     * @brief Constructor
     **/
    Module_Class_Members(BasePropagation, BaseModule, 0);
    
	virtual void initialize(int stage);
	~BasePropagation();

	virtual void sendToChannel(BasePhyLayer *,AirFrame *msg);

    /** @brief Registers a nic to have its connections managed by this module.*/
    virtual void registerNic( BasePhyLayer*);

};
#endif
