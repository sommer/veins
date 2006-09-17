/* -*-	mode:c++ -*- *******************************************************
 * file:        TestDecider.h
 *
 * author:      Daniel Willkomm
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
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/


#ifndef TEST_DECIDER_H
#define TEST_DECIDER_H

#include <BasicDecider.h>

/**
 * @brief A test Decider to test core functionality
 *
 * THis decider does not contain functionality and is only to test
 * core functionality
 *
 * @author Daniel Willkomm
 *
 * @ingroup decider
 */
class  TestDecider : public BasicDecider
{
  public:
    Module_Class_Members( TestDecider, BasicDecider, 0 );

  protected:
    virtual void handleLowerMsg(AirFrame*, const SnrList &);

    virtual void handleSelfMsg(cMessage* msg);
};

#endif
