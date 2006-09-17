/* -*- mode:c++ -*- ********************************************************
 * file:        YourNetwLayer.h
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
 * changelog:   $Revision: 116 $
 *              last modified:   $Date: 2004-10-12 18:57:45 +0200 (Di, 12 Okt 2004) $
 *              by:              $Author: omfw-willkomm $
 ***************************************************************************/


#ifndef YOUR_NETW_LAYER_H
#define YOUR_NETW_LAYER_H

#include <BasicNetwLayer.h>

class YourNetwLayer : public BasicNetwLayer
{
public:
    Module_Class_Members( YourNetwLayer, BasicNetwLayer, 0 );
    virtual void initialize(int);

protected:

    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(NetwPkt*);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(NetwPkt*);

  // uncomment the following lines to use the blackboard
  /*
    virtual bool blackboardItemChanged( BBItemRef );
    virtual bool blackboardItemPublished( BBItemRef );
    virtual bool blackboardItemWithdrawn( BBItemRef );
  */
		
};

#endif
