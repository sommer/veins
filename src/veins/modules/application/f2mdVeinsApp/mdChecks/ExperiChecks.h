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

#ifndef __VEINS_MDModuleV3_H_
#define __VEINS_MDModuleV3_H_

#include <tuple>

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/BaseWaveApplLayer.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/BsmCheck.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/InterTest.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeMDMHistory.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeTable.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>
#include <veins/modules/application/f2mdVeinsApp/mdStats/MDStatistics.h>
#include "../mdBase/InterTest.h"
#include "../mdSupport/MDMLib.h"


using namespace Veins;
using namespace omnetpp;

class ExperiChecks {
private:

    unsigned long myPseudonym;
    Coord myPosition;
    Coord myPositionConfidence;

    Coord myHeading;
    Coord myHeadingConfidence;

    Coord mySize;

    double MAX_PLAUSIBLE_SPEED = 0;
    double MAX_PLAUSIBLE_ACCEL = 0;
    double MAX_PLAUSIBLE_DECEL = 0;

    MDMLib mdmLib;

    double RangePlausibilityCheck(Coord, Coord, Coord, Coord);
    double PositionPlausibilityCheck(Coord, Coord, double, double);
    double SpeedPlausibilityCheck(double, double);
    double PositionConsistancyCheck(Coord curPosition,
            Coord curPositionConfidence, Coord oldPosition,
            Coord oldPositionConfidence, double time);
    double SpeedConsistancyCheck(double, double, double, double, double);
    double IntersectionCheck(Coord nodePosition1,
            Coord nodePositionConfidence1, Coord nodePosition2,
            Coord nodePositionConfidence2, Coord nodeHeading1, Coord nodeHeading2,
            Coord nodeSize1, Coord nodeSize2);
    InterTest MultipleIntersectionCheck(NodeTable * detectedNodes,
            BasicSafetyMessage * bsm);

    double PositionSpeedConsistancyCheck(Coord curPosition,
            Coord curPositionConfidence, Coord oldPosition,
            Coord oldPositionConfidence, double curSpeed,
            double curSpeedConfidence, double oldspeed,
            double oldSpeedConfidence, double time);


    double PositionHeadingConsistancyCheck(Coord curHeading,
            Coord curHeadingConfidence, Coord oldPosition,
            Coord oldPositionConfidence, Coord curPositionConfidence,
            Coord curPosition, double deltaTime, double curSpeed, double curSpeedConfidence);

    double BeaconFrequencyCheck(double, double);
    double SuddenAppearenceCheck(Coord, Coord, Coord, Coord);

    void PrintBsmCheck(unsigned long myPseudonym, BsmCheck bsmCheck);


    void resetAll();

public:
    ExperiChecks(unsigned long myPseudonym, Coord myPosition, Coord myPositionConfidence, Coord myHeading, Coord myHeadingConfidence, Coord mySize, Coord myLimits);
    BsmCheck CheckBSM(BasicSafetyMessage * bsm, NodeTable * detectedNodes);

};

#endif
