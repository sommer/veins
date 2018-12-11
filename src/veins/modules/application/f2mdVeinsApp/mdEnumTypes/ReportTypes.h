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

#ifndef __VEINS_ReportTypes_H_
#define __VEINS_ReportTypes_H_

#include <iostream>

namespace reportTypes {

enum Report {
    OneMessageReport = 0,
    EvidenceReport,
    BasicCheckReport,
    SIZE_OF_ENUM
};

static const char* ReportNames[] = { "OneMessageReport", "EvidenceReport", "BasicCheckReport"};

static_assert(sizeof(reportTypes::ReportNames)/sizeof(char*) == reportTypes::SIZE_OF_ENUM
        , "sizes dont match");
}

#endif
