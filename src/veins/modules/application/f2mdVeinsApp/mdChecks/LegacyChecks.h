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

#ifndef __VEINS_MDModule_H_
#define __VEINS_MDModule_H_

#include <tuple>

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeTable.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>
#include <veins/modules/application/f2mdVeinsApp/mdStats/MDStatistics.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/NetworkLinksLib/LinkControl.h>
#include "../mdSupport/MDMLib.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "../BaseWaveApplLayer.h"


using namespace Veins;
using namespace omnetpp;

class LegacyChecks {

private:

    unsigned long myPseudonym;
    Coord myPosition;
    Coord mySpeed;
    Coord mySize;
    Coord myHeading;

    MDMLib mdmLib;

    double MAX_PLAUSIBLE_SPEED = 0;
    double MAX_PLAUSIBLE_ACCEL = 0;
    double MAX_PLAUSIBLE_DECEL = 0;

    LinkControl* LinkC;

    double RangePlausibilityCheck(Coord*, Coord*);
    double PositionConsistancyCheck(Coord*, Coord*, double);
    double SpeedConsistancyCheck(double, double, double);
    double PositionSpeedConsistancyCheck(Coord*,
            Coord *, double , double , double );
    double SpeedPlausibilityCheck(double);
    double IntersectionCheck(Coord nodePosition1, Coord nodeSize1,
            Coord head1, Coord nodePosition2, Coord nodeSize2,
            Coord head2, double deltaTime);
    double SuddenAppearenceCheck(Coord*, Coord*);
    double BeaconFrequencyCheck(double, double);
    double PositionPlausibilityCheck(Coord*, double);
    double PositionHeadingConsistancyCheck(Coord* curHeading,
            Coord *curPosition, Coord *oldPosition, double deltaTime, double curSpeed);

    InterTest MultipleIntersectionCheck(NodeTable * detectedNodes,
            BasicSafetyMessage * bsm);

    void PrintBsmCheck(unsigned long senderPseudonym,
            BsmCheck bsmCheck);

public:

    LegacyChecks(unsigned long myPseudonym, Coord myPosition, Coord mySpeed, Coord myHeading,Coord mySize, Coord myLimits,  LinkControl* LinkC);
    BsmCheck CheckBSM(BasicSafetyMessage * bsm, NodeTable * detectedNodes);

};

#endif
