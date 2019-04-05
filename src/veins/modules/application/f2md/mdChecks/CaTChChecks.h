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
#include <veins/modules/application/f2md/mdBase/NodeMDMHistory.h>
#include <veins/modules/application/f2md/mdBase/NodeTable.h>
#include <veins/modules/application/f2md/mdReport/MDReport.h>
#include <veins/modules/application/f2md/mdStats/MDStatistics.h>
#include <veins/modules/application/f2md/mdSupport/networkLinksLib/LinkControl.h>
#include "../mdBase/NodeTable.h"
#include "../mdBase/InterTest.h"
#include "../mdBase/BsmCheck.h"
#include "../mdBase/InterTest.h"
#include "../mdSupport/MDMLib.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "../BaseWaveApplLayer.h"

#include "../mdSupport/kalmanLib/Kalman_SVI.h"
#include "../mdSupport/kalmanLib/Kalman_SI.h"


using namespace veins;
using namespace omnetpp;

class CaTChChecks {
private:
    int version = 0;
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
    double IntersectionCheck(Coord * nodePosition1,
            Coord * nodePositionConfidence1, Coord * nodePosition2,
            Coord* nodePositionConfidence2, Coord * nodeHeading1,
            Coord * nodeHeading2, Coord* nodeSize1, Coord * nodeSize2,
            double deltaTime);
    InterTest MultipleIntersectionCheck(NodeTable * detectedNodes,
            BasicSafetyMessage * bsm);

    double PositionSpeedMaxConsistancyCheck(Coord * curPosition,
            Coord * curPositionConfidence, Coord * oldPosition,
            Coord * oldPositionConfidence, double curSpeed,
            double curSpeedConfidence, double oldspeed,
            double oldSpeedConfidence, double time);

    double PositionSpeedConsistancyCheck(Coord * curPosition,
            Coord * curPositionConfidence, Coord * oldPosition,
            Coord * oldPositionConfidence, double curSpeed,
            double curSpeedConfidence, double oldspeed,
            double oldSpeedConfidence, double time);

    void KalmanPositionSpeedConsistancyCheck(Coord * curPosition,
            Coord * curPositionConfidence, Coord * curSpeed, Coord * curAccel,
            Coord * curSpeedConfidence, double time, Kalman_SVI * kalmanSVI,
            double retVal[]);

    void KalmanPositionSpeedScalarConsistancyCheck(Coord * curPosition,Coord * oldPosition,
            Coord * curPositionConfidence, Coord * curSpeed, Coord * curAccel,
            Coord * curSpeedConfidence, double time, Kalman_SC * kalmanSC,
            double retVal[]);

    double KalmanPositionConsistancyCheck(Coord * curPosition, Coord * oldPosition, Coord * curPosConfidence,
             double time, Kalman_SI * kalmanSI);

    double KalmanPositionAccConsistancyCheck(Coord * curPosition, Coord * curSpeed, Coord * curPosConfidence,
             double time, Kalman_SI * kalmanSI);

    double KalmanSpeedConsistancyCheck(Coord * curSpeed, Coord * curAccel, Coord * curSpeedConfidence,
            double time, Kalman_SI * kalmanSI);

    double PositionHeadingConsistancyCheck(Coord * curHeading,
            Coord * curHeadingConfidence, Coord * oldPosition,
            Coord * oldPositionConfidence, Coord * curPositionConfidence,
            Coord * curPosition, double deltaTime, double curSpeed,
            double curSpeedConfidence);

    double BeaconFrequencyCheck(double, double);
    double SuddenAppearenceCheck(Coord*, Coord*, Coord*, Coord*);

    void PrintBsmCheck(unsigned long senderPseudonym, BsmCheck bsmCheck);

    void resetAll();

public:
    CaTChChecks(int version, unsigned long myPseudonym, Coord myPosition,
            Coord myPositionConfidence, Coord myHeading,
            Coord myHeadingConfidence, Coord mySize, Coord myLimits,
            LinkControl* LinkC);
    BsmCheck CheckBSM(BasicSafetyMessage * bsm, NodeTable * detectedNodes);

};

#endif
