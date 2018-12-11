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

#ifndef __VEINS_MDModuleV2_H_
#define __VEINS_MDModuleV2_H_

#include <tuple>

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeMDMHistory.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeTable.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>
#include <veins/modules/application/f2mdVeinsApp/mdStats/MDStatistics.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/NetworkLinksLib/LinkControl.h>
#include "../mdBase/NodeTable.h"
#include "../mdBase/InterTest.h"
#include "../mdBase/BsmCheck.h"
#include "../mdBase/InterTest.h"
#include "../mdSupport/MDMLib.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "../BaseWaveApplLayer.h"



using namespace Veins;
using namespace omnetpp;

class CaTChChecks {
private:

    unsigned long myPseudonym;
    Coord myPosition;
    Coord myPositionConfidence;

    Coord myHeading;
    Coord myHeadingConfidence;

    Coord mySize;

    MDMLib mdmLib;

    LinkControl* LinkC;

    double MAX_PLAUSIBLE_SPEED = 0;
    double MAX_PLAUSIBLE_ACCEL = 0;
    double MAX_PLAUSIBLE_DECEL = 0;

    double RangePlausibilityCheck(Coord*, Coord*, Coord*, Coord*);
    double PositionPlausibilityCheck(Coord*, Coord*, double, double);
    double SpeedPlausibilityCheck(double, double);
    double PositionConsistancyCheck(Coord * curPosition,
            Coord * curPositionConfidence, Coord * oldPosition,
            Coord * oldPositionConfidence, double time);
    double SpeedConsistancyCheck(double, double, double, double, double);
    double IntersectionCheck(Coord *  nodePosition1,
            Coord *  nodePositionConfidence1, Coord *  nodePosition2,
            Coord*  nodePositionConfidence2, Coord * nodeHeading1, Coord * nodeHeading2,
            Coord*  nodeSize1, Coord * nodeSize2, double deltaTime);
    InterTest MultipleIntersectionCheck(NodeTable * detectedNodes,
            BasicSafetyMessage * bsm);

    double PositionSpeedConsistancyCheck(Coord * curPosition,
            Coord * curPositionConfidence, Coord * oldPosition,
            Coord * oldPositionConfidence, double curSpeed,
            double curSpeedConfidence, double oldspeed,
            double oldSpeedConfidence, double time);

    double PositionHeadingConsistancyCheck(Coord * curHeading,
            Coord * curHeadingConfidence, Coord * oldPosition,
            Coord * oldPositionConfidence, Coord * curPositionConfidence,
            Coord * curPosition, double deltaTime, double curSpeed, double curSpeedConfidence);

    double BeaconFrequencyCheck(double, double);
    double SuddenAppearenceCheck(Coord*, Coord*, Coord*, Coord*);

    void PrintBsmCheck(unsigned long senderPseudonym, BsmCheck bsmCheck);


    void resetAll();

public:
    CaTChChecks(unsigned long myPseudonym, Coord myPosition, Coord myPositionConfidence, Coord myHeading, Coord myHeadingConfidence, Coord mySize,Coord myLimits, LinkControl* LinkC);
    BsmCheck CheckBSM(BasicSafetyMessage * bsm, NodeTable * detectedNodes);

};

#endif
