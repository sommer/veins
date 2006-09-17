/* -*- mode:c++ -*- ********************************************************
 * file:        YourMobilityModule.h
 *
 * author:      Your Name
 *
 * copyright:   (C) 2004 Your Institution
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     Your Simulation
 * description: - Your Description
 *
 ***************************************************************************
 * changelog:   $Revision: 67 $
 *              last modified:   $Date: 2004-07-21 00:50:14 +0200 (Mi, 21 Jul 2004) $
 *              by:              $Author: omfw-willkomm $
 ***************************************************************************/


#ifndef YOUR_MOBILITY_MODULE_H
#define YOUR_MOBILITY_MODULE_H

#include <BasicMobility.h>

class YourMobilityModule : public BasicMobility
{
 protected:

 public:
  Module_Class_Members( YourMobilityModule , BasicMobility , 0 );

  /** @brief Initializes mobility model parameters.*/
  virtual void initialize(int);

 protected:
  /** @brief Called upon arrival of a self messages*/
  virtual void handleSelfMsg(cMessage *msg);

  // uncomment the following lines to use the blackboard
  /*
    virtual bool blackboardItemChanged( BBItemRef );
    virtual bool blackboardItemPublished( BBItemRef );
    virtual bool blackboardItemWithdrawn( BBItemRef );
  */
		
};

#endif

