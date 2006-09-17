/* -*- mode:c++ -*- ********************************************************
 * file:        TestSnrEval.h
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
 * description: - SnrEval class
 *              - mains tasks are to determine the SNR for a message and
 *                to simulate a transmission delay
 *
 ***************************************************************************/


#ifndef TEST_SNR_EVAL_H
#define TEST_SNR_EVAL_H

#include <BasicSnrEval.h>

/**
 * @brief An SnrEval module to test core functionality
 *
 * This module does not contain functionality and is just for core
 * testing purposes
 *
 * @author Daniel Willkomm
 * @ingroup snrEval
 **/

class TestSnrEval : public BasicSnrEval
{
public:
    Module_Class_Members( TestSnrEval, BasicSnrEval, 0 );

protected:
    /** @brief This function calculates the duration of the AirFrame */
    virtual double calcDuration(cMessage*);

    /** @brief Buffer the frame and update noise levels and snr information...*/
    virtual void handleLowerMsgStart(AirFrame*);

};

#endif
