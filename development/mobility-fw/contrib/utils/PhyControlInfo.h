/* -*- mode:c++ -*- *******************************************************
 * file:        PhyControlInfo.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 **************************************************************************
 * part of:     framework implementation developed by tkn
 * description: - control info to pass physical information 
 **************************************************************************/

#ifndef PHYCONTROLINFO_H
#define PHYCONTROLINFO_H

#include <omnetpp.h>

/**
 * @brief Control info to pass information to and from the PHY layer
 *
 * The physical layer needs some additional information from the MAC,
 * like the bit rate to use, and passes up some information about the
 * signal strength. 
 * 
 * @author Andreas Koepke
 **/
class PhyControlInfo : public cPolymorphic
{
  protected:
    /** @brief bit rate at which to send the payload / at which the payload was send
     *  As a convention, a rate in bits per second is encouraged
     */
    double bitrate;
    /** @brief snr information specifically for this packet */
    double snr;
    
  public:
    /** @brief Default constructor*/
    PhyControlInfo(const double br=0, const double s=0) : cPolymorphic(), bitrate(br), snr(s) {};

    /** @brief Destructor*/
    virtual ~PhyControlInfo(){};

    /** @brief get Bitrate */
    virtual const double getBitrate() {
	return bitrate;
    };

    /** @brief set bitrate */
    virtual void setBitrate(const double br){
	bitrate = br;
    };

    /** @brief get snr */
    virtual const double getSnr() {
	return snr;
    };

    /** @brief set snr */
    virtual void setSnr(const double r){
	snr = r;
    };
};

#endif
