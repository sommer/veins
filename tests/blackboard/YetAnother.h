/* -*- mode:c++ -*- ********************************************************
 * file:        YetAnother.h
 *
 * author:      Andreas Koepke
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
 *
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
 * description: Blackboard parameter for test purposes
 ***************************************************************************
 * changelog:   $Revision: 1.3 $
 *              last modified:   $Date: 2004/02/09 13:59:33 $
 *              by:              $Author: omfw-willkomm $
 **************************************************************************/


#ifndef YETANOTHER_H
#define YETANOTHER_H

#include "TestParam.h"

class YetAnother : public TestParam
{
 public:    
    YetAnother() : TestParam() { }
    const char* getDesc() {
        return "YetAnother";
    }
};

#endif
