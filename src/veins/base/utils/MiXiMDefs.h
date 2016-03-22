//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __MIXIM_MIXIMDEFS_H
#define __MIXIM_MIXIMDEFS_H

#include <omnetpp.h>

namespace omnetpp { }
using namespace omnetpp;

#include "veins/base/utils/miximkerneldefs.h"

// OMNeT 5 compatibility
// Around OMNeT++ 5.0 beta 2, the "ev" and "simulation" macros were eliminated, and replaced
// by the functions/methods getEnvir() and getSimulation(), the INET codebase updated.
// The following lines let the code compile with earlier OMNeT++ versions as well.
#ifdef ev
inline cEnvir *getEnvir() {return cSimulation::getActiveEnvir();}
inline cSimulation *getSimulation() {return cSimulation::getActiveSimulation();}
inline bool hasGUI() {return cSimulation::getActiveEnvir()->isGUI();}
#endif  //ev

// new OMNeT++ 5 logging macros
#if OMNETPP_VERSION < 0x500
#  define EV_FATAL                 EV << "FATAL: "
#  define EV_ERROR                 EV << "ERROR: "
#  define EV_WARN                  EV << "WARN: "
#  define EV_INFO                  EV
#  define EV_DETAIL                EV << "DETAIL: "
#  define EV_DEBUG                 EV << "DEBUG: "
#  define EV_TRACE                 EV << "TRACE: "
#  define EV_FATAL_C(category)     EV << "[" << category << "] FATAL: "
#  define EV_ERROR_C(category)     EV << "[" << category << "] ERROR: "
#  define EV_WARN_C(category)      EV << "[" << category << "] WARN: "
#  define EV_INFO_C(category)      EV << "[" << category << "] "
#  define EV_DETAIL_C(category)    EV << "[" << category << "] DETAIL: "
#  define EV_DEBUG_C(category)     EV << "[" << category << "] DEBUG: "
#  define EV_TRACE_C(category)     EV << "[" << category << "] TRACE: "
#  define EV_STATICCONTEXT         /* Empty */
#endif    // OMNETPP_VERSION < 0x500

// Around OMNeT++ 5.0 beta 2, random variate generation functions like exponential() became
// members of cComponent. By prefixing calls with the following macro you can make the code
// compile with earlier OMNeT++ versions as well.
#if OMNETPP_BUILDNUM >= 1002
#define RNGCONTEXT  (cSimulation::getActiveSimulation()->getContext())->
#else
#define RNGCONTEXT
#endif

// Around OMNeT++ 5.0 beta 3, MAXTIME was renamed to SIMTIME_MAX
#if OMNETPP_BUILDNUM < 1005
#define SIMTIME_MAX MAXTIME
#endif

// Around OMNeT++ 5, SubmoduleIterator changed from using () to * to get the module
#if OMNETPP_VERSION < 0x500
#define SUBMODULE_ITERATOR_TO_MODULE(x) x()
#else
#define SUBMODULE_ITERATOR_TO_MODULE(x) *x
#endif


#if defined(MIXIM_EXPORT)
#  define MIXIM_API OPP_DLLEXPORT
#elif defined(MIXIM_IMPORT)
#  define MIXIM_API OPP_DLLIMPORT
#else
#  define MIXIM_API
#endif

/**
 * @brief Helper function to initialize signal change identifier on use and
 *        not on initializing static sections.
 */
class MIXIM_API simsignalwrap_t {
private:
	mutable volatile simsignal_t ssChangeSignal;
	const char *const            sSignalName;
	mutable const char *         sRunId;
public:
	simsignalwrap_t(const char *const pSignalName)
	  : ssChangeSignal(SIMSIGNAL_NULL)
	  , sSignalName(pSignalName)
	  , sRunId(NULL)
	{}
	simsignalwrap_t(const simsignalwrap_t& pCpy)
	  : ssChangeSignal(pCpy.ssChangeSignal)
	  , sSignalName(pCpy.sSignalName)
	  , sRunId(pCpy.sRunId)
	{}

	/** Cast operator to simsignal_t, we initialize the signal here if it is empty ;). */
	operator simsignal_t () const {
		// if this signal was never used (or if this is a new run): register the signal
		if ((ssChangeSignal == SIMSIGNAL_NULL) || (getRunId() != sRunId)) {
			ASSERT(sSignalName);
			sRunId = getRunId();
			ssChangeSignal = cComponent::registerSignal(sSignalName);
			// throw cRuntimeError("%d = cComponent::registerSignal(\"%s\")", ssChangeSignal, sSignalName);
		}
		return ssChangeSignal;
	}
protected:
	const char* getRunId() const {
		return cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID);
	}
private:
	// not allowed
	simsignalwrap_t()
	  : ssChangeSignal(SIMSIGNAL_NULL)
	  , sSignalName(NULL)
	  , sRunId(NULL)
	{}
};

#endif
