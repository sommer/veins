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

#ifndef __VEINS_MdChecksTypes_H_
#define __VEINS_MdChecksTypes_H_

#include <iostream>

namespace mdChecksTypes {

enum Checks {
    RangePlausibility = 0,
    PositionPlausibility,
    SpeedPlausibility,
    PositionConsistancy,
    PositionSpeedConsistancy,
    SpeedConsistancy,
    BeaconFrequency,
    Intersection,
    SuddenAppearence,
    PositionHeadingConsistancy,
    SIZE_OF_ENUM
};

static const char* ChecksNames[] = { "RangePlausibility",
        "PositionPlausibility", "SpeedPlausibility", "PositionConsistancy",
        "PositionSpeedConsistancy", "SpeedConsistancy", "BeaconFrequency",
        "Intersection", "SuddenAppearence", "PositionHeadingConsistancy" };

static_assert(sizeof(mdChecksTypes::ChecksNames)/sizeof(char*) == mdChecksTypes::SIZE_OF_ENUM
        , "sizes dont match");

}

#endif
