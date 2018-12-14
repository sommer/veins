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

#ifndef __VEINS_PseudoChangeTypes_H_
#define __VEINS_PseudoChangeTypes_H_

#include <iostream>

namespace pseudoChangeTypes {

enum PseudoChange {
    NoChange = 0,
    Periodical,
    Disposable,
    DistanceBased,
    Random,
    Car2car,
    SIZE_OF_ENUM
};

static const char* PseudoChangeNames[] = { "NoChange", "Periodical", "Disposable",
        "DistanceBased", "Random" , "Car2car"};

static_assert(sizeof(pseudoChangeTypes::PseudoChangeNames)/sizeof(char*) == pseudoChangeTypes::SIZE_OF_ENUM
        , "sizes dont match");
}

#endif
