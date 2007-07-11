/* -*- mode:c++ -*- *******************************************************
 * file:        REDMacControlInfo.h
 *
 * author:      Jochen Adamek
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

#ifndef REDMACCONTROLINFO_H
#define REDMACCONTROLINFO_H

#include <omnetpp.h>
#include <MacControlInfo.h>

/**
 * @brief Control info to pass information to and from the PHY layer
 *
 * The physical layer needs some additional information from the MAC,
 * like the bit rate to use, and passes up some information about the
 * signal strength. 
 * 
 * @author Jochen Adamek
 **/
class REDMacControlInfo : public cPolymorphic
{
  protected:
    /** @brief bit rate at which to send the payload / at which the payload was send
     *  As a convention, a rate in bits per second is encouraged
     */
    double bitrate;
    /** @brief snr information specifically for this packet */
    double snr;
    
    double strength;
    
    int ack;
    
  public:
    /** @brief Default constructor*/
    REDMacControlInfo(const double br=0, const double s=0, const double st=0, const int a=0) : cPolymorphic(), bitrate(br), snr(s), strength(st), ack(a) {};

    /** @brief Destructor*/
    virtual ~REDMacControlInfo(){};

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
    
    virtual const double getStrength(){
    return strength;	
    };
    
    virtual void setStrength(const double st){
    strength = st;	
    };
    
    virtual const int getAck(){
    return ack;	
    }; 

    virtual void setAck(const int a){
    ack = a;	
    };
};

#endif