/* -*- mode:c++ -*- ********************************************************
 * file:        YourApplLayer.h
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
 ***************************************************************************/


#ifndef YOUR_APPL_LAYER_H
#define YOUR_APPL_LAYER_H

#include <BasicApplLayer.h>

class YourApplLayer : public BasicApplLayer
{
  public:
    Module_Class_Members( YourApplLayer, BasicApplLayer, 0 );
    virtual void initialize(int);

  protected:
    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(ApplPkt*);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
		
};

#endif
 
