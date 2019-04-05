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
#include <veins/modules/application/f2md/mdBase/NodeTable.h>
#include <veins/modules/application/f2md/mdReport/MDReport.h>
#include <veins/modules/application/f2md/mdStats/MDStatistics.h>
#include <veins/modules/application/f2md/mdSupport/networkLinksLib/LinkControl.h>
#include "../mdSupport/MDMLib.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "../BaseWaveApplLayer.h"

using namespace veins;
using namespace omnetpp;

class LegacyChecks {

private:
    int version = 0;
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
    double PositionSpeedConsistancyCheck(Coord*, Coord *, double, double,
            double);
    double PositionSpeedMaxConsistancyCheck(Coord*, Coord *, double, double,
            double);
    double SpeedPlausibilityCheck(double);
    double IntersectionCheck(Coord nodePosition1, Coord nodeSize1, Coord head1,
            Coord nodePosition2, Coord nodeSize2, Coord head2,
            double deltaTime);
    double SuddenAppearenceCheck(Coord*, Coord*);
    double BeaconFrequencyCheck(double, double);
    double PositionPlausibilityCheck(Coord*, double);
    double PositionHeadingConsistancyCheck(Coord* curHeading,
            Coord *curPosition, Coord *oldPosition, double deltaTime,
            double curSpeed);

    void KalmanPositionSpeedConsistancyCheck(Coord * curPosition,
            Coord * curPositionConfidence, Coord * curSpeed, Coord * oldSpeed,
            Coord * curSpeedConfidence, double time, Kalman_SVI * kalmanSVI,
            double retVal[]);

    void KalmanPositionSpeedScalarConsistancyCheck(Coord * curPosition,Coord * oldPosition,
            Coord * curPositionConfidence, Coord * curSpeed, Coord * oldSpeed,
            Coord * curSpeedConfidence, double time, Kalman_SC * kalmanSC,
            double retVal[]);

    double KalmanPositionConsistancyCheck(Coord * curPosition, Coord * oldPosition, Coord * curPosConfidence,
             double time, Kalman_SI * kalmanSI);

    double KalmanPositionAccConsistancyCheck(Coord * curPosition, Coord * curSpeed, Coord * curPosConfidence,
             double time, Kalman_SI * kalmanSI);

    double KalmanSpeedConsistancyCheck(Coord * curSpeed, Coord * oldSpeed, Coord * curSpeedConfidence,
            double time, Kalman_SI * kalmanSI);

    InterTest MultipleIntersectionCheck(NodeTable * detectedNodes,
            BasicSafetyMessage * bsm);

    void PrintBsmCheck(unsigned long senderPseudonym, BsmCheck bsmCheck);

public:

    LegacyChecks(int version, unsigned long myPseudonym, Coord myPosition, Coord mySpeed,
            Coord myHeading, Coord mySize, Coord myLimits, LinkControl* LinkC);
    BsmCheck CheckBSM(BasicSafetyMessage * bsm, NodeTable * detectedNodes);

};

#endif
