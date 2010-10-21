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
#include <cassert>

BaseModule::BaseModule():
	cSimpleModule(),
	utility(NULL)
{}

/**
 * Subscription to Blackboard should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BaseModule::initialize(int stage) {
    if (stage == 0) {
    	notAffectedByHostState = 	hasPar("notAffectedByHostState")
								 && par("notAffectedByHostState").boolValue();

        hasPar("debug") ? debug = par("debug").boolValue() : debug = false;
        utility = FindModule<BaseUtility*>::findSubModule(findHost());

        if(!utility) {
        	error("No BaseUtility module found!");
        } 

        hostId = findHost()->getId();

        /* host failure notification */
		HostState hs;
		hostStateCat = utility->subscribe(this, &hs, hostId);
    }
}

void BaseModule::receiveBBItem(int category, const BBItem *details, int scopeModuleId) {
	Enter_Method_Silent();

	if (category == hostStateCat) {

		handleHostState(*(HostState*)details);
	}
}

void BaseModule::handleHostState(const HostState& state)
{
	if(notAffectedByHostState)
		return;

	if(state.get() != HostState::ACTIVE) {
		error("Hosts state changed to something else than active which"
			  " is not handled by this module. Either handle this state"
			  " correctly or if this module really isn't affected by the"
			  " hosts state set the parameter \"notAffectedByHostState\""
			  " of this module to true.");
	}
}

void BaseModule::switchHostState(HostState::States state)
{
	HostState hostState(state);
	utility->publishBBItem(hostStateCat, &hostState, hostId);
}

cModule *BaseModule::findHost(void)
{
   return FindModule<>::findHost(this);
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
//std::string BaseModule::getLogName(int id)
//{
//    BaseModule *mod;
//    std::string lname;
//    mod = check_and_cast<BaseModule *>(simulation.getModule(id));
//    if (mod->isSimple()) {
//        lname = mod->logName();
//    }
//    else if(mod->getSubmodule("phy")) {
//        lname = check_and_cast<BaseModule *>(mod->getSubmodule("phy"))->logName();
//    }
//    return lname;
//};


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
			ost << parent->par("logName").stringValue() : ost << parent->getName();
		ost << "[" << parent->getIndex() << "]";
	}
	return ost.str();
}

