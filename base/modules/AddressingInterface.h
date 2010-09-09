/*
 * AddressingInterface.h
 *
 *  Created on: Sep 9, 2010
 *      Author: Karl Wessel
 */

#ifndef ADDRESSINGINTERFACE_H_
#define ADDRESSINGINTERFACE_H_

#include <omnetpp.h>

/**
 * @brief Interface for modules which assign L2 and L3 addresses
 * for modules.
 *
 * This interface is implemented by ARP modules which rely on a
 * certain addressing scheme to work.
 * @see HostArp for an example which demands the module's host's
 * index as L2 and L3 address to be able to provide a simplified
 * ARP which is useful for debugging purposes.
 *
 * @see BaseARP
 * @see HostARP
 *
 * @ingroup netwLayer
 * @ingroup baseModules
 *
 * @author Karl Wessel
 */
class AddressingInterface {
public:
	/** @brief Returns the L2 address for the passed mac*/
    virtual int myMacAddr(cModule* mac) = 0;

    /** @brief Returns the L3 address for the passed net*/
    virtual int myNetwAddr(cModule* netw) = 0;
};

#endif /* ADDRESSINGINTERFACE_H_ */
