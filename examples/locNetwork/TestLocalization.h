/* -*- mode:c++ -*- ********************************************************
 * file:        TestLocalization.h
 *
 * author:      Peterpaul Klein Haneveld
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
 * description: test class for the localization module
 **************************************************************************/

#ifndef TEST_LOCALIZATION_H
#define TEST_LOCALIZATION_H

#include "BaseLocalization.h"

/**
 * @brief Test class for the localization module
 *
 * @author Peterpaul Klein Haneveld
 */
class TestLocalization:public BaseLocalization {
      public:
	Module_Class_Members(TestLocalization, BaseLocalization, 0);
};

#endif				/* TEST_LOCALIZATION_H */
