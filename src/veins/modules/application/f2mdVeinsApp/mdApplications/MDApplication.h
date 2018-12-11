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

#ifndef __VEINS_MDApplication_H_
#define __VEINS_MDApplication_H_

#include <tuple>
#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeMDMHistory.h>
#include <veins/modules/application/f2mdVeinsApp/mdStats/MDStatistics.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/AppPrintable.h>
#include "../mdBase/NodeTable.h"
#include "../mdBase/InterTest.h"
#include "../mdBase/BsmCheck.h"
#include "../mdBase/InterTest.h"
#include "../mdSupport/MDMLib.h"
#include "../BaseWaveApplLayer.h"

#include "../mdReport/MDReport.h"

using namespace Veins;
using namespace omnetpp;


static AppPrintable prntAppV1;
static AppPrintable prntAppInstV1;

static AppPrintable prntAppV2;
static AppPrintable prntAppInstV2;


class MDApplication {

protected:
    MDMLib mdmLib;

    int version;

    char const *AppV1Name = "AppV1";
    char const *AppV2Name = "AppV2";

    AppPrintable* prntApp;
    AppPrintable* prntAppInst;
public:

    MDApplication(int version);

    virtual bool CheckNodeForReport(unsigned long myPseudonym,
            BasicSafetyMessage * bsm, BsmCheck * bsmCheck,
            NodeTable * detectedNodes)= 0;

    virtual double getMinFactor()= 0;

    void saveLine( std::string path, std::string serial, double density,
            double deltaT, bool printOut);
    void resetInstFlags();
    void resetAllFlags();
};



#endif
