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

#include <stdio.h>
#include <stdlib.h>     /* atof */
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <veins/modules/application/f2md/mdChecks/CaTChChecks.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace boost;

CaTChChecks::CaTChChecks(int version, unsigned long myPseudonym, Coord myPosition,
        Coord myPositionConfidence, Coord myHeading, Coord myHeadingConfidence,
        Coord mySize, Coord myLimits, LinkControl* LinkC) {
    this->version = version;
    this->myPseudonym = myPseudonym;
    this->myPosition = myPosition;
    this->myPositionConfidence = myPositionConfidence;
    this->myHeading = myHeading;
    this->myHeadingConfidence = myHeadingConfidence;
    this->mySize = mySize;
    this->MAX_PLAUSIBLE_SPEED = myLimits.x;
    this->MAX_PLAUSIBLE_ACCEL = myLimits.y;
    this->MAX_PLAUSIBLE_DECEL = myLimits.z;
    this->LinkC = LinkC;
}

double CaTChChecks::RangePlausibilityCheck(Coord * receiverPosition,
        Coord * receiverPositionConfidence, Coord * senderPosition,
        Coord * senderPositionConfidence) {

    double distance = mdmLib.calculateDistancePtr(senderPosition,
            receiverPosition);

    double factor = mdmLib.CircleCircleFactor(distance,
            senderPositionConfidence->x, receiverPositionConfidence->x,
            MAX_PLAUSIBLE_RANGE);

    return factor;
}

double CaTChChecks::PositionConsistancyCheck(Coord * curPosition,
        Coord * curPositionConfidence, Coord * oldPosition,
        Coord * oldPositionConfidence, double time) {
    double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
    double factor = mdmLib.CircleCircleFactor(distance,
            curPositionConfidence->x, oldPositionConfidence->x,
            MAX_PLAUSIBLE_SPEED * time);

    return factor;
}

double CaTChChecks::SpeedConsistancyCheck(double curSpeed,
        double curSpeedConfidence, double oldspeed, double oldSpeedConfidence,
        double time) {

    double speedDelta = curSpeed - oldspeed;

//    double attFact = mdmLib.gaussianSum(1, 1 / 3);
//    if (time >= 1) {
//        attFact = time;
//    }

    double factor = 1;
    if (speedDelta > 0) {
        factor = mdmLib.SegmentSegmentFactor(speedDelta, curSpeedConfidence,
                oldSpeedConfidence, MAX_PLAUSIBLE_ACCEL * time);
    } else {
        factor = mdmLib.SegmentSegmentFactor(fabs(speedDelta),
                curSpeedConfidence, oldSpeedConfidence,
                MAX_PLAUSIBLE_DECEL * time);
    }

    return factor;
}

double CaTChChecks::SpeedPlausibilityCheck(double speed,
        double speedConfidence) {
    if ((fabs(speed) + fabs(speedConfidence) / 2) < MAX_PLAUSIBLE_SPEED) {
        return 1;
    } else if ((fabs(speed) - fabs(speedConfidence) / 2)
            > MAX_PLAUSIBLE_SPEED) {
        return 0;
    } else {
        double factor = (fabs(speedConfidence) / 2
                + (MAX_PLAUSIBLE_SPEED - fabs(speed))) / fabs(speedConfidence);

        return factor;
    }
}

double CaTChChecks::PositionSpeedMaxConsistancyCheck(Coord * curPosition,
        Coord * curPositionConfidence, Coord * oldPosition,
        Coord * oldPositionConfidence, double curSpeed,
        double curSpeedConfidence, double oldspeed, double oldSpeedConfidence,
        double time) {

    if (time < MAX_TIME_DELTA) {

        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        double theoreticalSpeed = distance / time;
        double maxspeed = std::max(curSpeed, oldspeed);
        double minspeed = std::min(curSpeed, oldspeed);

        double curR = curPositionConfidence->x / time + curSpeedConfidence;
        double oldR = oldPositionConfidence->x / time + oldSpeedConfidence;

        double maxfactor = mdmLib.OneSidedCircleSegmentFactor(
                maxspeed - theoreticalSpeed, curR, oldR,
                (MAX_PLAUSIBLE_DECEL + MAX_MGT_RNG) * time);

        double minfactor = mdmLib.OneSidedCircleSegmentFactor(
                theoreticalSpeed - minspeed, curR, oldR,
                (MAX_PLAUSIBLE_ACCEL + MAX_MGT_RNG) * time);

        double factor = 1;

        if (minfactor < maxfactor) {
            factor = minfactor;
        } else {
            factor = maxfactor;
        }

//        factor = (factor - 0.5) * 2;
//        factor = mdmLib.gaussianSum(factor, (1.0 / 4.5));
//        if (factor > 0.75) {
//            factor = 1;
//        }
//
//        if (factor <0.001) {
//            factor = 0;
//        }


        if (factor < 0) {
            std::cout << "=======================================" << '\n';

            std::cout << " MAX_PLAUSIBLE_DECEL:" << MAX_PLAUSIBLE_DECEL
                    << " MAX_PLAUSIBLE_ACCEL:" << MAX_PLAUSIBLE_ACCEL << '\n';

            std::cout << " time:" << time << " distance:" << distance << '\n';
            std::cout << " maxspeed:" << maxspeed << " minspeed:" << minspeed
                    << " theoreticalSpeed:" << theoreticalSpeed << '\n';
            std::cout << " curSpeed:" << curSpeed << '\n';
            std::cout << " oldspeed:" << oldspeed << '\n';
            //exit(0);

        }

        return factor;

    } else {
        return 1;
    }
}

double CaTChChecks::PositionSpeedConsistancyCheck(Coord * curPosition,
        Coord * curPositionConfidence, Coord * oldPosition,
        Coord * oldPositionConfidence, double curSpeed,
        double curSpeedConfidence, double oldspeed, double oldSpeedConfidence,
        double time) {

    if (time < MAX_TIME_DELTA) {
        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        double minSpeed = 0;
        double maxSpeed = 0;
        double minSpeedConfidence = 0;
        double maxSpeedConfidence = 0;
        if (curSpeed > oldspeed) {
            maxSpeed = curSpeed;
            maxSpeedConfidence = curSpeedConfidence;
            minSpeed = oldspeed;
            minSpeedConfidence = oldSpeedConfidence;
        } else {
            maxSpeed = oldspeed;
            maxSpeedConfidence = oldSpeedConfidence;
            minSpeed = curSpeed;
            minSpeedConfidence = curSpeedConfidence;
        }

        double addon_mgt_range = MAX_MGT_RNG_DOWN + 0.3571 * minSpeed
                - 0.01694 * minSpeed * minSpeed;
        if (addon_mgt_range < 0) {
            addon_mgt_range = 0;
        }

        double maxTest_1 = 0;
        double minTest_1 = 0;
        double maxTest_2 = 0;
        double minTest_2 = 0;

        maxTest_1 = maxSpeed + maxSpeedConfidence;
        minTest_1 = minSpeed - minSpeedConfidence;

        maxTest_2 = maxSpeed - maxSpeedConfidence;
        minTest_2 = minSpeed + minSpeedConfidence;

        if (maxTest_2 < minTest_2) {
            minTest_2 = (maxSpeed + minSpeed) / 2;
            maxTest_2 = (maxSpeed + minSpeed) / 2;
        }

        double retDistance_1[2];
        mdmLib.calculateMaxMinDist(maxTest_1, minTest_1, time,
                MAX_PLAUSIBLE_ACCEL, MAX_PLAUSIBLE_DECEL, MAX_PLAUSIBLE_SPEED,
                retDistance_1);
        double factorMin_1 = 1
                - mdmLib.CircleCircleFactor(distance, curPositionConfidence->x,
                        oldPositionConfidence->x, retDistance_1[0]);
        double factorMax_1 = mdmLib.OneSidedCircleSegmentFactor(distance,
                curPositionConfidence->x, oldPositionConfidence->x,
                retDistance_1[1] + MAX_MGT_RNG_UP);

        double retDistance_2[2];
        mdmLib.calculateMaxMinDist(maxTest_2, minTest_2, time,
                MAX_PLAUSIBLE_ACCEL, MAX_PLAUSIBLE_DECEL, MAX_PLAUSIBLE_SPEED,
                retDistance_2);
        double factorMin_2 = 1
                - mdmLib.OneSidedCircleSegmentFactor(distance,
                        curPositionConfidence->x, oldPositionConfidence->x,
                        retDistance_2[0] - addon_mgt_range);
        double factorMax_2 = mdmLib.OneSidedCircleSegmentFactor(distance,
                curPositionConfidence->x, oldPositionConfidence->x,
                retDistance_2[1] + MAX_MGT_RNG_UP);

        double retDistance_0[2];
        mdmLib.calculateMaxMinDist(curSpeed, oldspeed, time,
                MAX_PLAUSIBLE_ACCEL, MAX_PLAUSIBLE_DECEL, MAX_PLAUSIBLE_SPEED,
                retDistance_0);
        double factorMin_0 = 1
                - mdmLib.OneSidedCircleSegmentFactor(distance,
                        curPositionConfidence->x, oldPositionConfidence->x,
                        retDistance_0[0] - addon_mgt_range);
        double factorMax_0 = mdmLib.OneSidedCircleSegmentFactor(distance,
                curPositionConfidence->x, oldPositionConfidence->x,
                retDistance_0[1] + MAX_MGT_RNG_UP);

        //return std::min(factorMin_0, factorMax_0);

        double factorMin = (factorMin_1 + factorMin_0 + factorMin_2) / 3.0;
        double factorMax = (factorMax_1 + factorMin_0 + factorMax_2) / 3.0;

        return std::min(factorMin, factorMax);

    } else {
        return 1;
    }
}

double CaTChChecks::IntersectionCheck(Coord * nodePosition1,
        Coord * nodePositionConfidence1, Coord * nodePosition2,
        Coord * nodePositionConfidence2, Coord * nodeHeading1,
        Coord * nodeHeading2, Coord * nodeSize1, Coord * nodeSize2,
        double deltaTime) {

//    double distance = mdmLib.calculateDistancePtr(nodePosition1, nodePosition2);
//    double intFactor = mdmLib.CircleIntersectionFactor(
//            nodePositionConfidence1->x, nodePositionConfidence2->x, distance,
//            MIN_INT_DIST);
//
//    intFactor = intFactor *  ((MAX_DELTA_INTER - deltaTime) / MAX_DELTA_INTER);
//
//    intFactor = 1 - intFactor;
//    return intFactor;

    double intFactor2 = mdmLib.EllipseEllipseIntersectionFactor(*nodePosition1,
            *nodePositionConfidence1, *nodePosition2, *nodePositionConfidence2,
            mdmLib.calculateHeadingAnglePtr(nodeHeading1),
            mdmLib.calculateHeadingAnglePtr(nodeHeading2), *nodeSize1,
            *nodeSize2);

    intFactor2 = 1.01
            - intFactor2 * ((MAX_DELTA_INTER - deltaTime) / MAX_DELTA_INTER);

    if (intFactor2 > 1) {
        intFactor2 = 1;
    }

    if (intFactor2 < 0) {
        intFactor2 = 0;
    }

    return intFactor2;

}

InterTest CaTChChecks::MultipleIntersectionCheck(NodeTable * detectedNodes,
        BasicSafetyMessage * bsm) {
    unsigned long senderPseudonym = bsm->getSenderPseudonym();
    Coord senderPos = bsm->getSenderPos();
    Coord senderPosConfidence = bsm->getSenderPosConfidence();
    Coord senderHeading = bsm->getSenderHeading();

    Coord senderSize = Coord(bsm->getSenderWidth(), bsm->getSenderLength());

    NodeHistory * varNode;

    double INTScore = 0;
    InterTest intertTest = InterTest();

    INTScore = IntersectionCheck(&myPosition, &myPositionConfidence, &senderPos,
            &senderPosConfidence, &myHeading, &senderHeading, &mySize,
            &senderSize, 0.11);
    if (INTScore < 1) {
        intertTest.addInterValue(myPseudonym, INTScore);
    }

    for (int var = 0; var < detectedNodes->getNodesNum(); ++var) {
        if (detectedNodes->getNodePseudo(var) != senderPseudonym) {
            varNode = detectedNodes->getNodeHistoryAddr(
                    detectedNodes->getNodePseudo(var));

            double deltaTime = mdmLib.calculateDeltaTime(
                    varNode->getLatestBSMAddr(), bsm);

            if (deltaTime < MAX_DELTA_INTER) {

                Coord varSize = Coord(
                        varNode->getLatestBSMAddr()->getSenderWidth(),
                        varNode->getLatestBSMAddr()->getSenderLength());

                INTScore = IntersectionCheck(
                        &varNode->getLatestBSMAddr()->getSenderPos(),
                        &varNode->getLatestBSMAddr()->getSenderPosConfidence(),
                        &senderPos, &senderPosConfidence,
                        &varNode->getLatestBSMAddr()->getSenderHeading(),
                        &senderHeading, &varSize, &senderSize, deltaTime);
                if (INTScore < 1) {
                    intertTest.addInterValue(detectedNodes->getNodePseudo(var),
                            INTScore);
                }
            }
        }
    }

    return intertTest;
}

double CaTChChecks::SuddenAppearenceCheck(Coord * receiverPosition,
        Coord * receiverPositionConfidence, Coord * senderPosition,
        Coord * senderPositionConfidence) {
    double distance = mdmLib.calculateDistancePtr(senderPosition,
            receiverPosition);

    double r2 = MAX_SA_RANGE + receiverPositionConfidence->x;

    double factor = 0;
    if (senderPositionConfidence->x <= 0) {
        if (distance < r2) {
            factor = 0;
        } else {
            factor = 1;
        }
    } else {
        double area = mdmLib.calculateCircleCircleIntersection(
                senderPositionConfidence->x, r2, distance);

        factor = area
                / (PI * senderPositionConfidence->x
                        * senderPositionConfidence->x);
        factor = 1 - factor;
    }

    return factor;
}

double CaTChChecks::PositionPlausibilityCheck(Coord * senderPosition,
        Coord * senderPositionConfidence, double senderSpeed,
        double senderSpeedConfidence) {

    double speedd = senderSpeed - senderSpeedConfidence;
    if (speedd < 0) {
        speedd = 0;
    }

    if (speedd <= MAX_NON_ROUTE_SPEED) {
        return 1;
    }

    double resolution = senderPositionConfidence->x / 10;
    if (resolution < 1) {
        resolution = 1;
    }
    double resolutionDelta = resolution / 10;

    double failedCount = 0;
    double allCount = 1;

    if (LinkC->calculateDistance(*senderPosition, 50,
            50) > MAX_DISTANCE_FROM_ROUTE) {
        failedCount++;
    }

    int resolutionTheta = 0;

    for (double r = resolution; r < senderPositionConfidence->x;
            r = r + resolution) {
        resolutionTheta = (int) (PI * r / (resolution));
        //std::cout << r<< "#" << resolution << "^" << resolutionTheta<<"-"<<"\n";
        for (int t = 0; t < resolutionTheta; ++t) {
            Coord p(senderPosition->x + r * cos(2 * PI * t / resolutionTheta),
                    senderPosition->y + r * sin(2 * PI * t / resolutionTheta));

            if (LinkC->calculateDistance(p, 50, 50) > MAX_DISTANCE_FROM_ROUTE) {
                failedCount++;
            }

            allCount++;
        }
        resolution = resolution + resolutionDelta;
    }

    return (1 - (failedCount / allCount));

}

double CaTChChecks::BeaconFrequencyCheck(double timeNew, double timeOld) {
    double timeDelta = timeNew - timeOld;
    if (timeDelta < MAX_BEACON_FREQUENCY) {
        return 0;
    } else {
        return 1;
    }
}

double CaTChChecks::PositionHeadingConsistancyCheck(Coord * curHeading,
        Coord * curHeadingConfidence, Coord * oldPosition,
        Coord * oldPositionConfidence, Coord * curPosition,
        Coord * curPositionConfidence, double deltaTime, double curSpeed,
        double curSpeedConfidence) {
    if (deltaTime < POS_HEADING_TIME) {
        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        if (distance < 1) {
            return 1;
        }

        if (curSpeed - curSpeedConfidence < 1) {
            return 1;
        }

        double curHeadingAngle = mdmLib.calculateHeadingAnglePtr(curHeading);

        Coord relativePos = Coord(curPosition->x - oldPosition->x,
                curPosition->y - oldPosition->y,
                curPosition->z - oldPosition->z);
        double positionAngle = mdmLib.calculateHeadingAnglePtr(&relativePos);
        double angleDelta = fabs(curHeadingAngle - positionAngle);
        if (angleDelta > 180) {
            angleDelta = 360 - angleDelta;
        }

        double angleLow = angleDelta - curHeadingConfidence->x;
        if (angleLow < 0) {
            angleLow = 0;
        }

        double angleHigh = angleDelta + curHeadingConfidence->x;
        if (angleHigh > 180) {
            angleHigh = 180;
        }

        double xLow = distance * cos(angleLow * PI / 180);

        double curFactorLow = 1;
        if (curPositionConfidence->x == 0) {
            if (angleLow <= MAX_HEADING_CHANGE) {
                curFactorLow = 1;
            } else {
                curFactorLow = 0;
            }
        } else {
            curFactorLow =
                    mdmLib.calculateCircleSegment(curPositionConfidence->x,
                            curPositionConfidence->x + xLow)
                            / (PI * curPositionConfidence->x
                                    * curPositionConfidence->x);
        }

        double oldFactorLow = 1;
        if (oldPositionConfidence->x == 0) {
            if (angleLow <= MAX_HEADING_CHANGE) {
                oldFactorLow = 1;
            } else {
                oldFactorLow = 0;
            }
        } else {
            oldFactorLow = 1
                    - mdmLib.calculateCircleSegment(oldPositionConfidence->x,
                            oldPositionConfidence->x - xLow)
                            / (PI * oldPositionConfidence->x
                                    * oldPositionConfidence->x);
        }

        double xHigh = distance * cos(angleHigh * PI / 180);
        double curFactorHigh = 1;
        if (curPositionConfidence->x == 0) {
            if (angleHigh <= MAX_HEADING_CHANGE) {
                curFactorHigh = 1;
            } else {
                curFactorHigh = 0;
            }
        } else {
            curFactorHigh =
                    mdmLib.calculateCircleSegment(curPositionConfidence->x,
                            curPositionConfidence->x + xHigh)
                            / (PI * curPositionConfidence->x
                                    * curPositionConfidence->x);
        }

        double oldFactorHigh = 1;
        if (oldPositionConfidence->x == 0) {
            if (angleHigh <= MAX_HEADING_CHANGE) {
                oldFactorHigh = 1;
            } else {
                oldFactorHigh = 0;
            }
        } else {
            oldFactorHigh = 1
                    - mdmLib.calculateCircleSegment(oldPositionConfidence->x,
                            oldPositionConfidence->x - xHigh)
                            / (PI * oldPositionConfidence->x
                                    * oldPositionConfidence->x);
        }

        double factor = (curFactorLow + oldFactorLow + curFactorHigh
                + oldFactorHigh) / 4;

//    if(factor<=0.0){
//
//        std::cout<<"curPos: "<<curPosition<<'\n';
//        std::cout<<"oldPos: "<<oldPosition<<'\n';
//
//        std::cout<<"relativePos: "<<relativePos<<'\n';
//
//        std::cout<<"curFactorLow: "<<curFactorLow<<'\n';
//        std::cout<<"oldFactorLow: "<<oldFactorLow<<'\n';
//        std::cout<<"curFactorHigh: "<<curFactorHigh<<'\n';
//        std::cout<<"oldFactorHigh: "<<oldFactorHigh<<'\n';
//        std::cout<<"positionAngle: "<<positionAngle<<'\n';
//        std::cout<<"curHeadingAngle: "<<curHeadingAngle<<'\n';
//        std::cout<<"angleDelta: "<<angleDelta<<'\n';
//        std::cout<<"distance: "<<distance<<'\n';
//        std::cout<<"distance: "<<distance<<'\n';
//        std::cout<<"xLow: "<<xLow<<'\n';
//        if(factor == 0){
//            std::cout<<"ZERO: "<<factor<<'\n';
//        }else{
//            std::cout<<"NONZ: "<<factor<<'\n';
//        }
//
//     //   exit(0);
//    }

//        factor = (factor - 0.5) * 2;
//        factor = mdmLib.gaussianSum(factor, (1.0 / 4.5));
//        if (factor > 0.75) {
//            factor = 1;
//        }
//        if (factor <0.001) {
//            factor = 0;
//        }

        return factor;
    } else {
        return 1;
    }
}


static double max_f1 = 1;
static double max_f2 = 1;
static double max_f3 = 1;
static double max_f4 = 1;
static double max_f5 = 1;
static double max_f6 = 1;
static double max_f7 = 1;

void CaTChChecks::KalmanPositionSpeedConsistancyCheck(Coord * curPosition,
        Coord * curPositionConfidence, Coord * curSpeed,Coord * curAccel,
        Coord * curSpeedConfidence, double time, Kalman_SVI * kalmanSVI,
        double retVal[]) {

    if (!kalmanSVI->isInit()) {
        retVal[0] = 1;
        retVal[1] = 1;
    } else {
        if(time < MAX_KALMAN_TIME){
            float Delta[4];

            double Ax = curAccel->x;
            double Ay = curAccel->y;

//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("X0a", kalmanSVI->kalmanFilterJ_SVI.X0, 4);
//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("Xa", kalmanSVI->kalmanFilterJ_SVI.X, 4);

            kalmanSVI->getDeltaPos(time, curPosition->x, curPosition->y,
                    curSpeed->x, curSpeed->y,Ax,Ay, curPositionConfidence->x,
                    curPositionConfidence->y, curSpeedConfidence->x,
                    curSpeedConfidence->y, Delta);


            double ret_1 = 1 - sqrt(pow(Delta[0], 2.0) + pow(Delta[2], 2.0)) / (KALMAN_POS_RANGE*curPositionConfidence->x*time);
            if(isnan(ret_1)){
                ret_1 = 0;
            }
            if(ret_1< max_f1){
                max_f1 = ret_1;
            }

            if(ret_1<0){
                ret_1 = 0;
            }

            double ret_2 = 1 - sqrt(pow(Delta[1], 2.0) + pow(Delta[3], 2.0)) / (KALMAN_SPEED_RANGE*curSpeedConfidence->x*time);
            if(isnan(ret_2)){
                ret_2 = 0;
            }
            if(ret_2< max_f2){
                  max_f2 = ret_2;
              }

            if(ret_2<0){
                ret_2 = 0;
            }


//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("Xb", kalmanSVI->kalmanFilterJ_SVI.X, 4);
//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("Delta", Delta, 4);

            retVal[0] = ret_1;
            retVal[1] = ret_2;

        }else{
            retVal[0] = 1;
            retVal[1] = 1;
            kalmanSVI->setInitial(curPosition->x, curPosition->y, curSpeed->x, curSpeed->y);
        }
    }
}

void CaTChChecks::KalmanPositionSpeedScalarConsistancyCheck(Coord * curPosition, Coord * oldPosition,
        Coord * curPositionConfidence, Coord * curSpeed, Coord * curAccel,
        Coord * curSpeedConfidence, double time, Kalman_SC * kalmanSC,
        double retVal[]) {

    if (!kalmanSC->isInit()) {
        retVal[0] = 1;
        retVal[1] = 1;
    } else {
        if(time < MAX_KALMAN_TIME){


            float Delta[2];

            double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
            double curspd = mdmLib.calculateSpeedPtr(curSpeed);
            double curacl = mdmLib.calculateSpeedPtr(curAccel);

            kalmanSC->getDeltaPos(time, distance,
                    curspd,curacl, curacl, curPositionConfidence->x, curSpeedConfidence->x, Delta);

            double ret_1 = 1 - (Delta[0] / (KALMAN_POS_RANGE*curPositionConfidence->x*time));
            if(isnan(ret_1)){
                ret_1 = 0;
            }
            if(ret_1< max_f3){
                max_f3 = ret_1;
            }

            if(ret_1<0){
                ret_1 = 0;
            }
            double ret_2 = 1 - (Delta[1] / (KALMAN_SPEED_RANGE*curSpeedConfidence->x*time));
            if(isnan(ret_2)){
                ret_2 = 0;
            }
            if(ret_2< max_f4){
                max_f4 = ret_2;
            }
            if(ret_2<0){
                ret_2 = 0;
            }
//            kalmanSC->kalmanFilterJ_SC.matrixOp_SC.printVec("Xb", kalmanSC->kalmanFilterJ_SC.X, 2);
//            kalmanSC->kalmanFilterJ_SC.matrixOp_SC.printVec("Delta", Delta, 2);

            retVal[0] = ret_1;
            retVal[1] = ret_2;

        }else{
            retVal[0] = 1;
            retVal[1] = 1;
            double curspd = mdmLib.calculateSpeedPtr(curSpeed);
            kalmanSC->setInitial(0, curspd);
        }
    }
}

double CaTChChecks::KalmanPositionConsistancyCheck(Coord * curPosition, Coord * oldPosition, Coord * curPosConfidence,
         double time, Kalman_SI * kalmanSI){
    if (!kalmanSI->isInit()) {
        return 1;
    } else {
        if(time < MAX_KALMAN_TIME){
            float Delta[2];
            double Ax = (curPosition->x - oldPosition->x)/time;
            double Ay = (curPosition->y - oldPosition->y)/time;

            kalmanSI->getDeltaPos(time, curPosition->x, curPosition->y,
                    curPosConfidence->x,
                    curPosConfidence->y, Delta);

            double ret_1 = 1 - sqrt(pow(Delta[0], 2.0) + pow(Delta[1], 2.0)) / (4*KALMAN_POS_RANGE* curPosConfidence->x*time);
            if(isnan(ret_1)){
                ret_1 = 0;
            }
            if(ret_1< max_f5){
                max_f5 = ret_1;
            }

            if(ret_1<0){
                ret_1 = 0;
            }

            return ret_1;
        }else{
            kalmanSI->setInitial(curPosition->x, curPosition->y);
            return 1;
        }
    }
}


double CaTChChecks::KalmanPositionAccConsistancyCheck(Coord * curPosition, Coord * curSpeed, Coord * curPosConfidence,
         double time, Kalman_SI * kalmanSI){
    if (!kalmanSI->isInit()) {
        return 1;
    } else {
        if(time < MAX_KALMAN_TIME){
            float Delta[2];
            double Ax = curSpeed->x;
            double Ay = curSpeed->y;

            kalmanSI->getDeltaPos(time, curPosition->x, curPosition->y,Ax,Ay,
                    curPosConfidence->x,
                    curPosConfidence->y, Delta);

            double ret_1 = 1 - sqrt(pow(Delta[0], 2.0) + pow(Delta[1], 2.0)) / (4*KALMAN_POS_RANGE*curPosConfidence->x*time);
            if(isnan(ret_1)){
                ret_1 = 0;
            }
            if(ret_1< max_f6){
                max_f6 = ret_1;
            }

            if(ret_1<0){
                ret_1 = 0;
            }

            return ret_1;
        }else{
            kalmanSI->setInitial(curPosition->x, curPosition->y);
            return 1;
        }
    }
}
double CaTChChecks::KalmanSpeedConsistancyCheck(Coord * curSpeed, Coord *curAccel, Coord * curSpeedConfidence,
         double time, Kalman_SI * kalmanSI){
    if (!kalmanSI->isInit()) {
        return 1;
    } else {
        if(time < MAX_KALMAN_TIME){
            float Delta[2];
            kalmanSI->getDeltaPos(time, curSpeed->x, curSpeed->y,curAccel->x,curAccel->y,
                    curSpeedConfidence->x,
                    curSpeedConfidence->y, Delta);

            double ret_1 = 1 - sqrt(pow(Delta[0], 2.0) + pow(Delta[1], 2.0)) / (KALMAN_SPEED_RANGE*curSpeedConfidence->x*time);
            if(isnan(ret_1)){
                ret_1 = 0;
            }
            if(ret_1< max_f7){
                max_f7 = ret_1;
            }
            if(ret_1<0){
                ret_1 = 0;
            }

            return ret_1;
        }else{
            kalmanSI->setInitial(curSpeed->x, curSpeed->y);
            return 1;
        }
    }
}



BsmCheck CaTChChecks::CheckBSM(BasicSafetyMessage * bsm,
        NodeTable * detectedNodes) {
    BsmCheck bsmCheck = BsmCheck();

    unsigned long senderPseudonym = bsm->getSenderPseudonym();
    Coord senderPos = bsm->getSenderPos();
    Coord senderPosConfidence = bsm->getSenderPosConfidence();

    NodeHistory * senderNode = detectedNodes->getNodeHistoryAddr(
            senderPseudonym);

    MDMHistory * senderMDM = detectedNodes->getMDMHistoryAddr(senderPseudonym);

    bsmCheck.setRangePlausibility(
            RangePlausibilityCheck(&myPosition, &myPositionConfidence,
                    &senderPos, &senderPosConfidence));

    bsmCheck.setSpeedPlausibility(
            SpeedPlausibilityCheck(
                    mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                    mdmLib.calculateSpeedPtr(
                            &bsm->getSenderSpeedConfidence())));

    bsmCheck.setIntersection(MultipleIntersectionCheck(detectedNodes, bsm));

    bsmCheck.setPositionPlausibility(
            PositionPlausibilityCheck(&senderPos, &senderPosConfidence,
                    mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                    mdmLib.calculateSpeedPtr(
                            &bsm->getSenderSpeedConfidence())));

    if (detectedNodes->getNodeHistoryAddr(senderPseudonym)->getBSMNum() > 0) {
        bsmCheck.setPositionConsistancy(
                PositionConsistancyCheck(&senderPos, &senderPosConfidence,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        &senderNode->getLatestBSMAddr()->getSenderPosConfidence(),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        bsmCheck.setSpeedConsistancy(
                SpeedConsistancyCheck(
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &bsm->getSenderSpeedConfidence()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeedConfidence()),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        bsmCheck.setBeaconFrequency(
                BeaconFrequencyCheck(bsm->getArrivalTime().dbl(),
                        senderNode->getLatestBSMAddr()->getArrivalTime().dbl()));

        bsmCheck.setPositionHeadingConsistancy(
                PositionHeadingConsistancyCheck(&bsm->getSenderHeading(),
                        &bsm->getSenderHeadingConfidence(),
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        &senderNode->getLatestBSMAddr()->getSenderPosConfidence(),
                        &bsm->getSenderPos(), &bsm->getSenderPosConfidence(),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr()),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &bsm->getSenderSpeedConfidence())));

        bsmCheck.setPositionSpeedConsistancy(
                PositionSpeedConsistancyCheck(&senderPos, &senderPosConfidence,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        &senderNode->getLatestBSMAddr()->getSenderPosConfidence(),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &bsm->getSenderSpeedConfidence()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeedConfidence()),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        bsmCheck.setPositionSpeedMaxConsistancy(
                PositionSpeedMaxConsistancyCheck(&senderPos,
                        &senderPosConfidence,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        &senderNode->getLatestBSMAddr()->getSenderPosConfidence(),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &bsm->getSenderSpeedConfidence()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeedConfidence()),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        if (mdmLib.calculateDeltaTime(bsm,
                senderNode->getLatestBSMAddr())> MAX_SA_TIME) {
            bsmCheck.setSuddenAppearence(
                    SuddenAppearenceCheck(&senderPos, &senderPosConfidence,
                            &myPosition, &myPositionConfidence));
        }

        double retVal[2];
        double retValSC[2];

        Kalman_SVI * kalmanSVI;
        if(version == 2){
            kalmanSVI = senderMDM->getKalmanSviv2();
        }else{
            kalmanSVI = senderMDM->getKalmanSviv1();
        }

        Kalman_SI * kalmanSI;
        if(version == 2){
            kalmanSI = senderMDM->getKalmanSiv2();
        }else{
            kalmanSI = senderMDM->getKalmanSiv1();
        }

        Kalman_SC * kalmanSC;
        if(version == 2){
            kalmanSC = senderMDM->getKalmanSvsiv2();
        }else{
            kalmanSC = senderMDM->getKalmanSvsiv1();
        }


        Kalman_SI * kalmanSAI;
        if(version == 2){
            kalmanSAI = senderMDM->getKalmanSaiv2();
        }else{
            kalmanSAI = senderMDM->getKalmanSaiv1();
        }

        Kalman_SI * kalmanVI;
        if(version == 2){
            kalmanVI = senderMDM->getKalmanViv2();
        }else{
            kalmanVI = senderMDM->getKalmanViv1();
        }

        KalmanPositionSpeedConsistancyCheck(&senderPos, &senderPosConfidence,
                &bsm->getSenderSpeed(),&bsm->getSenderAccel(), &bsm->getSenderSpeedConfidence(),
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()),
                kalmanSVI , retVal);
        bsmCheck.setKalmanPSCP(retVal[0]);
        bsmCheck.setKalmanPSCS(retVal[1]);

        KalmanPositionSpeedScalarConsistancyCheck(&senderPos,&senderNode->getLatestBSMAddr()->getSenderPos(), &senderPosConfidence,
                &bsm->getSenderSpeed(),&bsm->getSenderAccel(), &bsm->getSenderSpeedConfidence(),
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()),
                kalmanSC , retValSC);

        bsmCheck.setKalmanPSCSP(retValSC[0]);
        bsmCheck.setKalmanPSCSS(retValSC[1]);

        bsmCheck.setKalmanPCC(KalmanPositionConsistancyCheck(&senderPos,&senderNode->getLatestBSMAddr()->getSenderPos(), &senderPosConfidence,
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()), kalmanSI));

        bsmCheck.setKalmanPACS( KalmanPositionAccConsistancyCheck(&senderPos,&senderNode->getLatestBSMAddr()->getSenderSpeed(), &senderPosConfidence,
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()), kalmanSI));

        bsmCheck.setKalmanSCC(KalmanSpeedConsistancyCheck(&bsm->getSenderSpeed(),&bsm->getSenderAccel(), &bsm->getSenderSpeedConfidence(),
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()), kalmanVI));


    } else {
        bsmCheck.setSuddenAppearence(
                SuddenAppearenceCheck(&senderPos, &senderPosConfidence,
                        &myPosition, &myPositionConfidence));
    }
//    std::cout << "======================================="<<"\n";
//    std::cout << "max_f1 => " << max_f1 << '\n';
//    std::cout << "max_f2 => " << max_f2 << '\n';
//    std::cout << "max_f3 => " << max_f3 << '\n';
//    std::cout << "max_f4 => " << max_f4 << '\n';
//    std::cout << "max_f5 => " << max_f5 << '\n';
//    std::cout << "max_f6 => " << max_f6 << '\n';
//    std::cout << "max_f7 => " << max_f7 << '\n';

//    if(bsm->getSenderMbType() == 1){
//    PrintBsmCheck(senderPseudonym, bsmCheck);
//    }

    return bsmCheck;
}

void CaTChChecks::PrintBsmCheck(unsigned long senderPseudonym,
        BsmCheck bsmCheck) {

    if (bsmCheck.getRangePlausibility() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "ART FAILED => "
                << bsmCheck.getRangePlausibility() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getPositionConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "MGTD FAILED => "
                << bsmCheck.getPositionConsistancy() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getSpeedConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "MGTS FAILED => "
                << bsmCheck.getSpeedConsistancy() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }

    if (bsmCheck.getPositionSpeedConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "MGTSV FAILED => "
                << bsmCheck.getPositionSpeedConsistancy() << " A:"
                << myPseudonym << " B:" << senderPseudonym << '\n';

    }

    if (bsmCheck.getPositionSpeedMaxConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "MGTSVM FAILED => "
                << bsmCheck.getPositionSpeedMaxConsistancy() << " A:"
                << myPseudonym << " B:" << senderPseudonym << '\n';

    }

    if (bsmCheck.getSpeedPlausibility() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "MAXS FAILED => "
                << bsmCheck.getSpeedPlausibility() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getPositionPlausibility() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "MAP FAILED => "
                << bsmCheck.getPositionPlausibility() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }

//    if (bsmCheck.getSuddenAppearence() < 0.5) {
//        std::cout << "^^^^^^^^^^^V2 " << "SAW FAILED => "
//                << bsmCheck.getSuddenAppearence() << " A:" << myPseudonym
//                << " B:" << senderPseudonym << '\n';
//    }

    if (bsmCheck.getPositionHeadingConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "PHC FAILED => "
                << bsmCheck.getPositionHeadingConsistancy() << " A:"
                << myPseudonym << " B:" << senderPseudonym << '\n';

    }

    if (bsmCheck.getBeaconFrequency() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "FREQ FAILED => "
                << bsmCheck.getBeaconFrequency() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getKalmanSCC() < 0.5) {
        std::cout << "^^^^^^^^^^^V2 " << "KalmanSCC FAILED => "
                << bsmCheck.getKalmanSCC() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';
    }
    if (bsmCheck.getKalmanSCC() < 0.5) {
    std::cout << "^^^^^^^^^^^V2 " << "KalmanPACS FAILED => "
            << bsmCheck.getKalmanSCC() << " A:" << myPseudonym
            << " B:" << senderPseudonym << '\n';
    }
    if (bsmCheck.getKalmanPCC() < 0.5) {
    std::cout << "^^^^^^^^^^^V2 " << "KalmanPCC FAILED => "
            << bsmCheck.getKalmanPCC() << " A:" << myPseudonym
            << " B:" << senderPseudonym << '\n';
    }
    if (bsmCheck.getKalmanPSCP() < 0.5) {
    std::cout << "^^^^^^^^^^^V2 " << "KalmanPSCP FAILED => "
            << bsmCheck.getKalmanPSCP() << " A:" << myPseudonym
            << " B:" << senderPseudonym << '\n';
    }
    if (bsmCheck.getKalmanPSCS() < 0.5) {
    std::cout << "^^^^^^^^^^^V2 " << "KalmanPSCS FAILED => "
            << bsmCheck.getKalmanPSCS() << " A:" << myPseudonym
            << " B:" << senderPseudonym << '\n';
    }
    if (bsmCheck.getKalmanPSCSP() < 0.5) {
    std::cout << "^^^^^^^^^^^V2 " << "KalmanPSCSP FAILED => "
            << bsmCheck.getKalmanPSCSP() << " A:" << myPseudonym
            << " B:" << senderPseudonym << '\n';
    }
    if (bsmCheck.getKalmanPSCSP() < 0.5) {
    std::cout << "^^^^^^^^^^^V2 " << "KalmanPSCSS FAILED => "
            << bsmCheck.getKalmanPSCSP() << " A:" << myPseudonym
            << " B:" << senderPseudonym << '\n';
    }
    InterTest inter = bsmCheck.getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {
        if (inter.getInterValue(var) < 0.5) {
            std::cout << "^^^^^^^^^^^V2 " << "INT FAILED => "
                    << inter.getInterValue(var) << " A:" << myPseudonym << " B:"
                    << senderPseudonym << " C:" << inter.getInterId(var)
                    << '\n';

        }
    }

}

