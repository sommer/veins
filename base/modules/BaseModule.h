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

#include "ImNotifiable.h"
#include "FindModule.h"

#include "HostState.h"
#include "BaseUtility.h"


#define EV_clear (ev.isDisabled()||!debug) ? ev : ev
//#define EV (ev.isDisabled()||!debug) ? ev : ev << logName() << "::" << getClassName() << ": " ==> EV is now part of <omnetpp.h>

#ifndef coreEV
#define coreEV_clear (ev.isDisabled()||!coreDebug) ? ev : ev
#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << logName() << "::" << getClassName() <<": "
#endif

/**
 * @brief Base class for all simple modules of a host that want to have
 * access to the BaseUtility module.
 *
 * BaseModule listens for HostState changes broadcasted over BaseUtility
 * and calls its own "handleHostState()" method. This method raises
 * an error if the host state changes to something else than ACTIVE.
 * Therefore that a sub-classing module can be used in a simulation
 * where the host state can change it has to override that method
 * which forces the author to make sure the module reacts well to
 * host state changes.
 * Alternatively one can also set a "notAffectedByHostState" parameter
 * of the module to true.
 *
 * The base module additionally provides a function findHost which
 * returns a pointer to the host module and a function hostIndex to
 * return the index of the host module. The latter one correspondes to
 * the index shown in tkenv and comes in very handy for testing and
 * debugging using tkenv. It is used e.g. in all the 'print' macros
 * used for debugging.
 *
 * There will never be a stand-alone BaseModule module.
 *
 * Note: most modules wont derive from BaseModule directly but from
 * its sub class "BatteryAccess" which extends BaseModule by several
 * methods for accessing the battery module.
 *
 * @see BaseUtility
 * @see BatteryAccess
 *
 * @ingroup baseModules
 *
 * @author Steffen Sroka
 * @author Andreas Koepke
 */
class BaseModule: public cSimpleModule, public ImNotifiable {
  protected:
    /** @brief Cached pointer to the utility module*/
    BaseUtility *utility;

    /** @brief Debug switch for all other modules*/
    bool debug;



	/** @brief Stores if this module is affected by changes in the
	 * hosts state. If not explicitly set this module has to capture
	 * changes in the host state.*/
	bool notAffectedByHostState;

	/** @brief Stores the category of the HostState*/
	int hostStateCat;

	/** @brief The hosts id. */
	int hostId;
protected:

	/**
	 * @brief Called whenever the hosts state changes.
	 *
	 * Default implementation of this method throws an error whenever the host
	 * state changes and the "notAffectedbyHostState" variable is not explicitly
	 * set. This is because every module of a host has to make sure to react
	 * well to changes in the host state. Or it has to explicitly set its
	 * parameter "notAffectedbyHostState" to true.
	 */
	virtual void handleHostState(const HostState& state);

	/**
	 * @brief Switches the host to the passed state.
	 *
	 * This is a convenience method for publishing the host state to
	 * BaseUtility.
	 *
	 * If the hosts state is switched to anything else than "ACTIVE" every
	 * module of the host has to handle this explicitly (see method
	 * "handleHostState()")!
	 */
	void switchHostState(HostState::States state);

    /** @brief Function to get a pointer to the host module*/
    cModule *findHost(void);
    /** @brief Function to get the logging name of id*/
    //std::string getLogName(int);



  public:
    BaseModule();

    /** @brief Basic initialization for all modules */
    virtual void initialize(int);

    /**
     * @brief Divide initialization into two stages
     *
     * In the first stage (stage==0), modules subscribe to notification
     * categories at Blackboard. The first notifications (e.g. about the initial
     * values of some variables such as RadioState) should take place earliest
     * in the second stage (stage==1), when everyone interested in them has
     * already subscribed.
     * Further one should try to keep calls to other modules out of stage 0 to
     * assure that the other module had at least once the chance to initialize
     * itself in stage 0.
     */
    virtual int numInitStages() const {
    	return 2;
    }

    /**
     * @brief Function to get the logging name of the host
     *
     * The logging name is the ned module name of the host (unless the
     * host ned variable loggingName is specified). It can be used for
     * logging messages to simplify debugging in TKEnv.
     */
    std::string logName(void);

    /**
     * @brief Get a reference to the local node module
     */
    cModule * getNode(){
    	return findHost();
    };

    /**
     * @brief Called by the Blackboard whenever a change of a category occurs
     * to which we have subscribed. Redefined from ImNotifiable.
     * In this base class just handle the host state switching and
     * some debug notifications
     */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
};

#endif
