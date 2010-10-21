/*
 * ArpInterface.h
 *
 *  Created on: Sep 9, 2010
 *      Author: Karl Wessel
 */

#ifndef ARPINTERFACE_H_
#define ARPINTERFACE_H_

/**
 * @brief Interface every Address resolution protocol (ARP)
 * module has to implement.
 *
 * Declares only one method that resolves a L3 address into a
 * L2 address.
 *
 * @ingroup netwLayer
 * @ingroup baseModules
 *
 * @author Karl Wessel
 */
class ArpInterface {
public:
	/** @brief returns a L2 address to a given L3 address*/
    virtual int getMacAddr(const int netwAddr) = 0;
};

#endif /* ARPINTERFACE_H_ */
