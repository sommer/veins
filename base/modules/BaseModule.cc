/* -*- mode:c++ -*- ********************************************************
 * file:        BaseModule.cc
 *
 * author:      Steffen Sroka
 *              Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#include "BaseModule.h"
#include "BaseUtility.h"

/**
 * Subscription to Blackboard should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BaseModule::initialize(int stage)
{    
    if (stage == 0) {        
        hasPar("debug") ? debug = par("debug").boolValue() : debug = false;
        utility = FindModule<BaseUtility*>::findSubModule(findHost());
        if (utility == NULL) {
            error("Could not find BaseUtility module");
        }
    }
}


cModule *BaseModule::findHost(void)
{
    cModule *parent = parentModule();
    cModule *node = this;

    // all nodes should be a sub module of the simulation which has no parent module!!!
    while( parent->parentModule() != NULL ){
	node = parent;
	parent = node->parentModule();
    }

    return node;
}


/**
 * This function returns the logging name of the module with the
 * specified id. It can be used for logging messages to simplify
 * debugging in TKEnv.
 *
 * Only supports ids from simple module derived from the BaseModule
 * or the nic compound module id.
 *
 * @param id Id of the module for the desired logging name
 * @return logging name of module id or NULL if not found
 * @sa logName
 */
std::string BaseModule::getLogName(int id)
{
    BaseModule *mod;
    std::string lname;
    mod = check_and_cast<BaseModule *>(simulation.module(id));
    if (mod->isSimple()) {
        lname = mod->logName();
    }
    else if(mod->submodule("phy")) {
        lname = check_and_cast<BaseModule *>(mod->submodule("phy"))->logName();
    }
    return lname;
};


std::string BaseModule::logName(void)
{
        std::ostringstream ost;
	if (hasPar("logName")) // let modules override
	{
		ost << par("logName").stringValue();
	}
	else
	{
		cModule *parent = findHost();
		parent->hasPar("logName") ?
			ost << parent->par("logName").stringValue() : ost << parent->name();
		ost << "[" << parent->index() << "]";
	}
	return ost.str();
}

