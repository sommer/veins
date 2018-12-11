/*******************************************************************************
 * @author  Joseph Kamel
 * @email   josephekamel@gmail.com
 * @date    28/11/2018
 * @version 2.0
 *
 * SCA (Secure Cooperative Autonomous systems)
 * Copyright (c) 2013, 2018 Institut de Recherche Technologique SystemX
 * All rights reserved.
 *******************************************************************************/

#ifndef __VEINS_MdChecksVersionTypes_H_
#define __VEINS_MdChecksVersionTypes_H_

#include <iostream>

namespace mdChecksVersionTypes {

enum ChecksVersion {
    LegacyChecks = 0,
    CatchChecks,
    SIZE_OF_ENUM
};

static const char* ChecksVersionNames[] = { "LegacyChecks",
        "CatchChecks"};

static_assert(sizeof(mdChecksVersionTypes::ChecksVersionNames)/sizeof(char*) == mdChecksVersionTypes::SIZE_OF_ENUM
        , "sizes dont match");

}

#endif
