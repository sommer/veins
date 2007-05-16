/* -*- mode:c++ -*- ********************************************************
 * file:        BaseModule.h
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


#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include <sstream>
#include <omnetpp.h>
//#include "BaseUtility.h"

#ifndef EV
//#define EV (ev.disabled()||!debug) ? (std::ostream&)ev : ev << simtimeToStr(simTime()) << ": "<< logName() << "::" << className()  << ": "
#define EV (ev.disabled()||!debug) ? (std::ostream&)ev : ev << logName() << "::" << className() << ": "
#endif
#ifndef coreEV
//#define coreEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << simtimeToStr(simTime()) << ": "<< logName() << "::" << className() <<": "
#define coreEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << logName() << "::" << className() <<": "
#endif


/**
 * @brief Base class for all simple modules of a host that want to have
 * access to the Blackboard module.
 *
 * The basic module additionally provides a function findHost which
 * returns a pointer to the host module and a function hostIndex to
 * return the index of the host module. The latter one correspondes to
 * the index shown in tkenv and comes in very handy for testing and
 * debugging using tkenv. It is used e.g. in all the 'print' macros
 * used for debugging.
 *
 * There is no Define_Module() for this class because we use
 * BasicModule only as a base class to derive all other
 * module. There will never be a stand-alone BasicModule module
 * (and that is why there is no Define_Module() and no .ned file for
 * BasicModule).
 *
 * @see Blackboard
 * @ingroup basicModules
 *
 * @author Steffen Sroka
 * @author Andreas Koepke
 */
//class BaseModule: public cSimpleModule, public ImNotifiable {
class BaseModule: public cSimpleModule {
  protected:
    /** @brief Cached pointer to the Blackboard module*/
    //BaseUtility *baseUtil;
    
    /** @brief Debug switch for all other modules*/
    bool debug;

  protected:
    /** @brief Function to get a pointer to the host module*/
    cModule *findHost(void) const;

    /** @brief Function to get the logging name of id*/
    std::string getLogName(int);

  public:
    Module_Class_Members(BaseModule, cSimpleModule, 0);

    /** @brief Basic initialization for all modules */
    virtual void initialize(int);

    /**
     * @brief Divide initialization into two stages
     *
     * In the first stage (stage==0), modules subscribe to notification
     * categories at Blackboard. The first notifications
     * (e.g. about the initial values of some variables such as RadioState)
     * should take place earliest in the second stage (stage==1),
     * when everyone interested in them has already subscribed.
     */
    virtual int numInitStages() const {
      return 3;
    }

    /**
     * @brief Function to get the logging name of the host
     *
     * The logging name is the ned module name of the host (unless the
     * host ned variable loggingName is specified). It can be used for
     * logging messages to simplify debugging in TKEnv.
     */
    std::string logName(void) const {
        std::ostringstream ost;
        cModule *parent = findHost();
        parent->hasPar("logName") ?
            ost << parent->par("logName").stringValue() : ost << parent->name();
        ost << "[" << parent->index() << "]";
        return ost.str();
    };

	/** 
	 * @brief Get a reference to a global singleton module
	 *
	 * Given the name of the module type, this function returns a reference
	 * to a global singleton module. Lookups for non-singleton modules will 
	 * return a random module of the specified type. NULL is returned on lookup
	 * failure
	 * @param modtype Module type name
	 */
	
	cModule * getGlobalModule(const char* modtype);

   	/** 
	 * @brief Get a reference to a node-level module
	 *
	 * Given the name of the module type, this function returns a reference
	 * to a node-level module. Lookups for modules with multiple instances of the
	 * same type in a single module will return a random module of the specified 
	 * type. NULL is returned on lookup failure
	 * @param modtype Module type name
	 */
	
	cModule * getNodeModule(const char* modtype);

	/**
	 * @brief Get a reference to the local node module
	 */

	cModule * getNode();

 /**
     * @brief Called by the Blackboard whenever a change of a category occurs
     * to which we have subscribed. Redefined from ImNotifiable.
     * In this base class just handle the context switching and
     * some debug notifications
     */
    /*virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId) {
        if(debug) {
            Enter_Method("receiveBBItem(\"%s, %i\")", details->info().c_str(), scopeModuleId);
        } else {
            Enter_Method_Silent();
        }
    }*/
};

#endif
