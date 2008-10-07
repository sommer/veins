/* -*- mode:c++ -*- ********************************************************
 * file:        FoxtrotPatterned.h
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: aggregation layer: Foxtrot with patterned spacial merging
 ***************************************************************************/

#ifndef FOXTROT_PATT_H
#define FOXTROT_PATT_H

#include "Foxtrot.h"

class FoxtrotPatterned:public Foxtrot
{
  public:
	//Module_Class_Members(FoxtrotPatterned, Foxtrot, 0);
  protected:
	virtual void dataUp(FoxtrotPacket * pkt);
	virtual bool merge(uint8_t * subset, uint8_t * delete_count);
	virtual void newData(const foxtrot_data * data);
};

#endif
