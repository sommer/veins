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
#include "miximkerneldefs.h"

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
public:
	simsignalwrap_t(const char *const pSignalName)
	  : ssChangeSignal(SIMSIGNAL_NULL)
	  , sSignalName(pSignalName)
	{}
	simsignalwrap_t(const simsignalwrap_t& pCpy)
	  : ssChangeSignal(pCpy.ssChangeSignal)
	  , sSignalName(pCpy.sSignalName)
	{}

	/** Cast operator to simsignal_t, we initialize the signal here if it is empty ;). */
	operator simsignal_t () const {
		if (ssChangeSignal == SIMSIGNAL_NULL && sSignalName != NULL) {
			ssChangeSignal = cComponent::registerSignal(sSignalName);
			// opp_warning("%d = cComponent::registerSignal(\"%s\")", ssChangeSignal, sSignalName);
		}
		return ssChangeSignal;
	}
private:
	// not allowed
	simsignalwrap_t()
	  : ssChangeSignal(SIMSIGNAL_NULL)
	  , sSignalName(NULL)
	{}
};

#endif
