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
#include <unistd.h>
#include <veins/modules/application/f2md/mdChecks/LegacyChecks.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>    // std::max

using namespace std;
using namespace boost;

static double print_count = 0;

LegacyChecks::LegacyChecks(int version, unsigned long myPseudonym,
        Coord myPosition, Coord mySpeed, Coord myHeading, Coord mySize,
        Coord myLimits, LinkControl* LinkC) {
    this->version = version;
    this->myPseudonym = myPseudonym;
    this->myPosition = myPosition;
    this->mySpeed = mySpeed;
    this->mySize = mySize;
    this->myHeading = myHeading;

    this->MAX_PLAUSIBLE_SPEED = myLimits.x;
    this->MAX_PLAUSIBLE_ACCEL = myLimits.y;
    this->MAX_PLAUSIBLE_DECEL = myLimits.z;

    this->LinkC = LinkC;
}

double LegacyChecks::RangePlausibilityCheck(Coord *senderPosition,
        Coord *receiverPosition) {
    double distance = mdmLib.calculateDistancePtr(senderPosition,
            receiverPosition);

    if (distance < MAX_PLAUSIBLE_RANGE) {
        return 1;
    } else {
        return 0; //distance
    }
}

double LegacyChecks::PositionConsistancyCheck(Coord * curPosition,
        Coord * oldPosition, double time) {
    double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);

    if (distance < MAX_PLAUSIBLE_SPEED * time) {
        return 1;
    } else {
        return 0; //distance
    }
}

double LegacyChecks::SpeedConsistancyCheck(double curSpeed, double oldspeed,
        double time) {

    double speedDelta = curSpeed - oldspeed;

//    double attFact = mdmLib.gaussianSum(1, 1 / 3);
//    if (time >= 1) {
//        attFact = time;
//    }

    if (speedDelta > 0) {

        if (speedDelta < MAX_PLAUSIBLE_ACCEL * time) {
            return 1;
        } else {
            return 0; //distance
        }
    } else {

        if (fabs(speedDelta) < MAX_PLAUSIBLE_DECEL * time) {
            return 1;
        } else {
            return 0; //distance
        }
    }

}

double LegacyChecks::PositionSpeedMaxConsistancyCheck(Coord *curPosition,
        Coord * oldPosition, double curSpeed, double oldspeed, double time) {
    if (time < MAX_TIME_DELTA) {
        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        double theoreticalSpeed = distance / time;
        //double realspeed = fabs(curSpeed + oldspeed)/2;
        double maxspeed = std::max(curSpeed, oldspeed);
        double minspeed = std::min(curSpeed, oldspeed);

        double deltaMax = maxspeed - theoreticalSpeed;
        double deltaMin = theoreticalSpeed - minspeed;

        if (deltaMax > (MAX_PLAUSIBLE_DECEL + MAX_MGT_RNG) * time) {
            return 0; // deltaMax - MIN_PSS
        } else {
            if (deltaMin > (MAX_PLAUSIBLE_ACCEL + MAX_MGT_RNG) * time) {
                return 0; // deltaMin - MAX_PSS
            } else {
                return 1;
            }
        }
    } else {
        return 1;
    }
}

double LegacyChecks::PositionSpeedConsistancyCheck(Coord *curPosition,
        Coord * oldPosition, double curSpeed, double oldspeed, double time) {
    if (time < MAX_TIME_DELTA) {
        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        double curminspeed = std::min(curSpeed, oldspeed);
        //double theoreticalSpeed = distance / time;
        double retDistance[2];
        mdmLib.calculateMaxMinDist(curSpeed, oldspeed, time,
                MAX_PLAUSIBLE_ACCEL, MAX_PLAUSIBLE_DECEL, MAX_PLAUSIBLE_SPEED,
                retDistance);

        double addon_mgt_range = MAX_MGT_RNG_DOWN + 0.3571 * curminspeed
                - 0.01694 * curminspeed * curminspeed;
        if (addon_mgt_range < 0) {
            addon_mgt_range = 0;
        }

        double deltaMin = distance - retDistance[0] + addon_mgt_range;
        double deltaMax = retDistance[1] - distance + MAX_MGT_RNG_UP;

        if (deltaMin < 0 || deltaMax < 0) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

double LegacyChecks::SpeedPlausibilityCheck(double speed) {

    if (fabs(speed) < MAX_PLAUSIBLE_SPEED) {
        return 1;
    } else {
        return 0; // fabs(speed)
    }
}

double LegacyChecks::IntersectionCheck(Coord nodePosition1, Coord nodeSize1,
        Coord head1, Coord nodePosition2, Coord nodeSize2, Coord head2,
        double deltaTime) {

    double heading1 = mdmLib.calculateHeadingAnglePtr(&head1);
    double heading2 = mdmLib.calculateHeadingAnglePtr(&head2);

    //double distance = mdmLib.calculateDistancePtr(nodePosition1, nodePosition2);

    double inter = mdmLib.RectRectFactor(nodePosition1, nodePosition2, heading1,
            heading2, Coord(nodeSize1.x, nodeSize1.y),
            Coord(nodeSize2.x, nodeSize2.y));

    inter = inter * ((MAX_DELTA_INTER - deltaTime) / MAX_DELTA_INTER);

//    if (inter > 0.5){
//
//        std::cout<<"nodePosition1:"<<nodePosition1<<"\n";
//        std::cout<<"nodePosition2:"<<nodePosition2<<"\n";
//
//        std::cout<<"heading1:"<<heading1<<"\n";
//        std::cout<<"heading2:"<<heading2<<"\n";
//
//        std::cout<<"nodeSize1:"<<nodeSize1<<"\n";
//        std::cout<<"nodeSize2:"<<nodeSize2<<"\n";
//
//        std::cout<<"x:"<<nodePosition2.x-nodePosition1.x<<"\n";
//        std::cout<<"y:"<<nodePosition2.y-nodePosition1.y<<"\n";
//        std::cout<<"h:"<<heading2-heading1<<"\n";
//
//        std::cout<<"deltaTime:"<<deltaTime<<"\n";
//
//        std::cout<<"((MAX_DELTA_INTER-deltaTime)/MAX_DELTA_INTER):"<<((MAX_DELTA_INTER-deltaTime)/MAX_DELTA_INTER)<<"\n";
//
//        std::cout<<"inter:"<<inter<<"\n";
//
//        Coord conf = Coord(0,0,0);
//
//        double intFactor2 = mdmLib.EllipseEllipseIntersectionFactor(nodePosition1,
//                conf, nodePosition2, conf,
//                heading1, heading2 , nodeSize1, nodeSize2);
//        intFactor2 = intFactor2 *  ((MAX_DELTA_INTER - deltaTime) / MAX_DELTA_INTER);
//        std::cout<<"intFactor2:"<<intFactor2<<"\n";
////
////        if (intFactor2 > 0.5) {
////            return 0; //inter+1.0
////        } else {
////            return 1;
////        }
//        exit(0);
//    }

    if (inter > 0.5) {
        return 0; //inter+1.0
    } else {
        return 1;
    }

}

InterTest LegacyChecks::MultipleIntersectionCheck(NodeTable * detectedNodes,
        BasicSafetyMessage * bsm) {

    unsigned long senderPseudonym = bsm->getSenderPseudonym();

    NodeHistory *varNode;

    double INTScore = 0;
    InterTest intertTest = InterTest();

    INTScore = IntersectionCheck(myPosition, mySize, myHeading,
            bsm->getSenderPos(),
            Coord(bsm->getSenderWidth(), bsm->getSenderLength()),
            bsm->getSenderHeading(), 0.11);

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

                INTScore = IntersectionCheck(varNode->getSenderPos(0),
                        varNode->getSenderSize(0), varNode->getSenderHeading(0),
                        bsm->getSenderPos(),
                        Coord(bsm->getSenderWidth(), bsm->getSenderLength()),
                        bsm->getSenderHeading(), deltaTime);

                if (INTScore < 1) {
                    intertTest.addInterValue(myPseudonym, INTScore);
                }
            }
        }
    }

    return intertTest;
}

double LegacyChecks::SuddenAppearenceCheck(Coord *senderPosition,
        Coord *receiverPosition) {
    double distance = mdmLib.calculateDistancePtr(senderPosition,
            receiverPosition);

    if (distance < MAX_SA_RANGE) {
        return 0; //distance
    } else {
        return 1;
    }
}

double LegacyChecks::PositionPlausibilityCheck(Coord *senderPosition,
        double senderSpeed) {
    if (senderSpeed <= MAX_NON_ROUTE_SPEED) {
        return 1;
    }

    double distance = LinkC->calculateDistance(*senderPosition, 50, 50);

    if (distance > MAX_DISTANCE_FROM_ROUTE) {
        return 0;
    } else {
        return 1;
    }
}

double LegacyChecks::BeaconFrequencyCheck(double timeNew, double timeOld) {
    double timeDelta = timeNew - timeOld;

    if (timeDelta < MAX_BEACON_FREQUENCY) {
        return 0;
    } else {
        return 1;
    }
}

double LegacyChecks::PositionHeadingConsistancyCheck(Coord * curHeading,
        Coord * curPosition, Coord * oldPosition, double deltaTime,
        double curSpeed) {
    if (deltaTime < POS_HEADING_TIME) {
        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        if (distance < 1) {
            return 1;
        }

        if (curSpeed < 1) {
            return 1;
        }

        double curHeadingAngle = mdmLib.calculateHeadingAnglePtr(curHeading);
        Coord relativePos = Coord(curPosition->x - oldPosition->x,
                curPosition->y - oldPosition->y,
                curPosition->z - oldPosition->z);
        double positionAngle = mdmLib.calculateHeadingAngle(relativePos);
        double angleDelta = fabs(curHeadingAngle - positionAngle);
        if (angleDelta > 180) {
            angleDelta = 360 - angleDelta;
        }

        if (MAX_HEADING_CHANGE < angleDelta) {
            return 0; //  angleDelta - MAX_HEADING_CHANGE
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

void LegacyChecks::KalmanPositionSpeedConsistancyCheck(Coord * curPosition,
        Coord * curPositionConfidence, Coord * curSpeed, Coord * curAccel,
        Coord * curSpeedConfidence, double time, Kalman_SVI * kalmanSVI,
        double retVal[]) {

    if (!kalmanSVI->isInit()) {
        retVal[0] = 1;
        retVal[1] = 1;
    } else {
        if (time < MAX_KALMAN_TIME) {
            float Delta[4];

            double Ax = curAccel->x;
            double Ay = curAccel->y;

            double curPosConfX = curPositionConfidence->x;
            if (curPosConfX < KALMAN_MIN_POS_RANGE) {
                curPosConfX = KALMAN_MIN_POS_RANGE;
            }

            double curPosConfY = curPositionConfidence->y;
            if (curPosConfY < KALMAN_MIN_POS_RANGE) {
                curPosConfY = KALMAN_MIN_POS_RANGE;
            }

            double curSpdConfX = curSpeedConfidence->x;
            if (curSpdConfX < KALMAN_MIN_SPEED_RANGE) {
                curSpdConfX = KALMAN_MIN_SPEED_RANGE;
            }

            double curSpdConfY = curSpeedConfidence->y;
            if (curSpdConfY < KALMAN_MIN_SPEED_RANGE) {
                curSpdConfY = KALMAN_MIN_SPEED_RANGE;
            }

//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("X0a", kalmanSVI->kalmanFilterJ_SVI.X0, 4);
//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("Xa", kalmanSVI->kalmanFilterJ_SVI.X, 4);

            kalmanSVI->getDeltaPos(time, curPosition->x, curPosition->y,
                    curSpeed->x, curSpeed->y, Ax, Ay, curPosConfX, curPosConfY,
                    curSpdConfX, curSpdConfY, Delta);

            double ret_1 = 1
                    - sqrt(pow(Delta[0], 2.0) + pow(Delta[2], 2.0))
                            / (KALMAN_POS_RANGE * curPosConfX * time);
            if (isnan(ret_1)) {
                ret_1 = 0;
            }

            if (ret_1 < 0.5) {
                ret_1 = 0;
            } else {
                ret_1 = 1;
            }

            double ret_2 = 1
                    - sqrt(pow(Delta[1], 2.0) + pow(Delta[3], 2.0))
                            / (KALMAN_SPEED_RANGE * curSpdConfX * time);
            if (isnan(ret_2)) {
                ret_2 = 0;
            }

            if (ret_2 < 0.5) {
                ret_2 = 0;
            } else {
                ret_2 = 1;
            }

//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("Xb", kalmanSVI->kalmanFilterJ_SVI.X, 4);
//            kalmanSVI->kalmanFilterJ_SVI.matrixOp_SVI.printVec("Delta", Delta, 4);

            retVal[0] = ret_1;
            retVal[1] = ret_2;

        } else {
            retVal[0] = 1;
            retVal[1] = 1;
            kalmanSVI->setInitial(curPosition->x, curPosition->y, curSpeed->x,
                    curSpeed->y);
        }
    }
}

void LegacyChecks::KalmanPositionSpeedScalarConsistancyCheck(
        Coord * curPosition, Coord * oldPosition, Coord * curPositionConfidence,
        Coord * curSpeed, Coord * curAccel, Coord * curSpeedConfidence,
        double time, Kalman_SC * kalmanSC, double retVal[]) {

    if (!kalmanSC->isInit()) {
        retVal[0] = 1;
        retVal[1] = 1;
    } else {
        if (time < MAX_KALMAN_TIME) {

            float Delta[2];

            double distance = mdmLib.calculateDistancePtr(curPosition,
                    oldPosition);
            double curspd = mdmLib.calculateSpeedPtr(curSpeed);
            double curacl = mdmLib.calculateSpeedPtr(curAccel);

            double curPosConfX = curPositionConfidence->x;
            if (curPosConfX < KALMAN_MIN_POS_RANGE) {
                curPosConfX = KALMAN_MIN_POS_RANGE;
            }

            double curSpdConfX = curSpeedConfidence->x;
            if (curSpdConfX < KALMAN_MIN_SPEED_RANGE) {
                curSpdConfX = KALMAN_MIN_SPEED_RANGE;
            }

            kalmanSC->getDeltaPos(time, distance, curspd, curacl, curacl,
                    curPosConfX, curSpdConfX, Delta);

            double ret_1 = 1
                    - (Delta[0] / (KALMAN_POS_RANGE * curPosConfX * time));
            if (isnan(ret_1)) {
                ret_1 = 0;
            }

            if (ret_1 < 0.5) {
                ret_1 = 0;
            } else {
                ret_1 = 1;
            }
            double ret_2 = 1
                    - (Delta[1] / (KALMAN_SPEED_RANGE * curSpdConfX * time));
            if (isnan(ret_2)) {
                ret_2 = 0;
            }
            if (ret_2 < 0.5) {
                ret_2 = 0;
            } else {
                ret_2 = 1;
            }
//            kalmanSC->kalmanFilterJ_SC.matrixOp_SC.printVec("Xb", kalmanSC->kalmanFilterJ_SC.X, 2);
//            kalmanSC->kalmanFilterJ_SC.matrixOp_SC.printVec("Delta", Delta, 2);

            retVal[0] = ret_1;
            retVal[1] = ret_2;

        } else {
            retVal[0] = 1;
            retVal[1] = 1;
            double curspd = mdmLib.calculateSpeedPtr(curSpeed);
            kalmanSC->setInitial(0, curspd);
        }
    }
}

double LegacyChecks::KalmanPositionConsistancyCheck(Coord * curPosition,
        Coord * oldPosition, Coord * curPosConfidence, double time,
        Kalman_SI * kalmanSI) {
    if (!kalmanSI->isInit()) {
        return 1;
    } else {
        if (time < MAX_KALMAN_TIME) {
            float Delta[2];
            double Ax = (curPosition->x - oldPosition->x) / time;
            double Ay = (curPosition->y - oldPosition->y) / time;

            double curPosConfX = curPosConfidence->x;
            if (curPosConfX < KALMAN_MIN_POS_RANGE) {
                curPosConfX = KALMAN_MIN_POS_RANGE;
            }

            double curPosConfY = curPosConfidence->y;
            if (curPosConfY < KALMAN_MIN_POS_RANGE) {
                curPosConfY = KALMAN_MIN_POS_RANGE;
            }

            kalmanSI->getDeltaPos(time, curPosition->x, curPosition->y,
                    curPosConfX, curPosConfY, Delta);

            double ret_1 = 1
                    - sqrt(pow(Delta[0], 2.0) + pow(Delta[1], 2.0))
                            / (4 * KALMAN_POS_RANGE * curPosConfX * time);

            if (isnan(ret_1)) {
                ret_1 = 0;
            }
            if (ret_1 < 0.5) {
                ret_1 = 0;
            } else {
                ret_1 = 1;
            }

            return ret_1;
        } else {
            kalmanSI->setInitial(curPosition->x, curPosition->y);
            return 1;
        }
    }
}

double LegacyChecks::KalmanPositionAccConsistancyCheck(Coord * curPosition,
        Coord * curSpeed, Coord * curPosConfidence, double time,
        Kalman_SI * kalmanSI) {
    if (!kalmanSI->isInit()) {
        return 1;
    } else {
        if (time < MAX_KALMAN_TIME) {
            float Delta[2];
            double Ax = curSpeed->x;
            double Ay = curSpeed->y;

            double curPosConfX = curPosConfidence->x;
            if (curPosConfX < KALMAN_MIN_POS_RANGE) {
                curPosConfX = KALMAN_MIN_POS_RANGE;
            }

            double curPosConfY = curPosConfidence->y;
            if (curPosConfY < KALMAN_MIN_POS_RANGE) {
                curPosConfY = KALMAN_MIN_POS_RANGE;
            }

            kalmanSI->getDeltaPos(time, curPosition->x, curPosition->y, Ax, Ay,
                    curPosConfX, curPosConfY, Delta);

            double ret_1 = 1
                    - sqrt(pow(Delta[0], 2.0) + pow(Delta[1], 2.0))
                            / (4 * KALMAN_POS_RANGE * curPosConfX * time);
            if (isnan(ret_1)) {
                ret_1 = 0;
            }

            if (ret_1 < 0.5) {
                ret_1 = 0;
            } else {
                ret_1 = 1;
            }

            return ret_1;
        } else {
            kalmanSI->setInitial(curPosition->x, curPosition->y);
            return 1;
        }
    }
}
double LegacyChecks::KalmanSpeedConsistancyCheck(Coord * curSpeed,
        Coord *curAccel, Coord * curSpeedConfidence, double time,
        Kalman_SI * kalmanSI) {
    if (!kalmanSI->isInit()) {
        return 1;
    } else {
        if (time < MAX_KALMAN_TIME) {
            float Delta[2];
            double curSpdConfX = curSpeedConfidence->x;
            if (curSpdConfX < KALMAN_MIN_SPEED_RANGE) {
                curSpdConfX = KALMAN_MIN_SPEED_RANGE;
            }

            double curSpdConfY = curSpeedConfidence->y;
            if (curSpdConfY < KALMAN_MIN_SPEED_RANGE) {
                curSpdConfY = KALMAN_MIN_SPEED_RANGE;
            }

            kalmanSI->getDeltaPos(time, curSpeed->x, curSpeed->y, curAccel->x,
                    curAccel->y, curSpdConfX, curSpdConfY, Delta);

            double ret_1 = 1
                    - sqrt(pow(Delta[0], 2.0) + pow(Delta[1], 2.0))
                            / (KALMAN_SPEED_RANGE * curSpdConfX * time);
            if (isnan(ret_1)) {
                ret_1 = 0;
            }
            if (ret_1 < 0.5) {
                ret_1 = 0;
            } else {
                ret_1 = 1;
            }

            return ret_1;
        } else {
            kalmanSI->setInitial(curSpeed->x, curSpeed->y);
            return 1;
        }
    }
}

BsmCheck LegacyChecks::CheckBSM(BasicSafetyMessage *bsm,
        NodeTable *detectedNodes) {
    BsmCheck bsmCheck = BsmCheck();

    unsigned long senderPseudonym = bsm->getSenderPseudonym();
    Coord senderPos = bsm->getSenderPos();
    Coord senderPosConfidence = bsm->getSenderPosConfidence();

    NodeHistory * senderNode = detectedNodes->getNodeHistoryAddr(
            senderPseudonym);

    MDMHistory * senderMDM = detectedNodes->getMDMHistoryAddr(senderPseudonym);

    bsmCheck.setRangePlausibility(
            RangePlausibilityCheck(&myPosition, &bsm->getSenderPos()));

    bsmCheck.setSpeedPlausibility(
            SpeedPlausibilityCheck(
                    mdmLib.calculateSpeed(bsm->getSenderSpeed())));

    bsmCheck.setIntersection(MultipleIntersectionCheck(detectedNodes, bsm));

    bsmCheck.setPositionPlausibility(
            PositionPlausibilityCheck(&senderPos,
                    mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed())));

    if (detectedNodes->getNodeHistoryAddr(senderPseudonym)->getBSMNum() > 0) {

        bsmCheck.setPositionConsistancy(
                PositionConsistancyCheck(&senderPos,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        bsmCheck.setSpeedConsistancy(
                SpeedConsistancyCheck(
                        mdmLib.calculateSpeed(bsm->getSenderSpeed()),
                        mdmLib.calculateSpeed(
                                senderNode->getLatestBSMAddr()->getSenderSpeed()),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));
        bsmCheck.setBeaconFrequency(
                BeaconFrequencyCheck(bsm->getArrivalTime().dbl(),
                        senderNode->getLatestBSMAddr()->getArrivalTime().dbl()));

        bsmCheck.setPositionHeadingConsistancy(
                PositionHeadingConsistancyCheck(&bsm->getSenderHeading(),
                        &bsm->getSenderPos(),
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr()),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed())));

        bsmCheck.setPositionSpeedConsistancy(
                PositionSpeedConsistancyCheck(&senderPos,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeed()),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        bsmCheck.setPositionSpeedMaxConsistancy(
                PositionSpeedMaxConsistancyCheck(&senderPos,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeed()),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr())));

        if (mdmLib.calculateDeltaTime(bsm,
                senderNode->getLatestBSMAddr()) > MAX_SA_TIME) {
            bsmCheck.setSuddenAppearence(
                    SuddenAppearenceCheck(&senderPos, &myPosition));
        }

        double retVal[2];
        double retValSC[2];

        Kalman_SVI * kalmanSVI;
        if (version == 2) {
            kalmanSVI = senderMDM->getKalmanSviv2();
        } else {
            kalmanSVI = senderMDM->getKalmanSviv1();
        }

        Kalman_SI * kalmanSI;
        if (version == 2) {
            kalmanSI = senderMDM->getKalmanSiv2();
        } else {
            kalmanSI = senderMDM->getKalmanSiv1();
        }

        Kalman_SC * kalmanSC;
        if (version == 2) {
            kalmanSC = senderMDM->getKalmanSvsiv2();
        } else {
            kalmanSC = senderMDM->getKalmanSvsiv1();
        }

        Kalman_SI * kalmanSAI;
        if (version == 2) {
            kalmanSAI = senderMDM->getKalmanSaiv2();
        } else {
            kalmanSAI = senderMDM->getKalmanSaiv1();
        }

        Kalman_SI * kalmanVI;
        if (version == 2) {
            kalmanVI = senderMDM->getKalmanViv2();
        } else {
            kalmanVI = senderMDM->getKalmanViv1();
        }

        KalmanPositionSpeedConsistancyCheck(&senderPos, &senderPosConfidence,
                &bsm->getSenderSpeed(), &bsm->getSenderAccel(),
                &bsm->getSenderSpeedConfidence(),
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()),
                kalmanSVI, retVal);
        bsmCheck.setKalmanPSCP(retVal[0]);
        bsmCheck.setKalmanPSCS(retVal[1]);

        KalmanPositionSpeedScalarConsistancyCheck(&senderPos,
                &senderNode->getLatestBSMAddr()->getSenderPos(),
                &senderPosConfidence, &bsm->getSenderSpeed(),
                &bsm->getSenderAccel(), &bsm->getSenderSpeedConfidence(),
                mdmLib.calculateDeltaTime(bsm, senderNode->getLatestBSMAddr()),
                kalmanSC, retValSC);

        bsmCheck.setKalmanPSCSP(retValSC[0]);
        bsmCheck.setKalmanPSCSS(retValSC[1]);

        bsmCheck.setKalmanPCC(
                KalmanPositionConsistancyCheck(&senderPos,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        &senderPosConfidence,
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr()), kalmanSI));

        bsmCheck.setKalmanPACS(
                KalmanPositionAccConsistancyCheck(&senderPos,
                        &senderNode->getLatestBSMAddr()->getSenderSpeed(),
                        &senderPosConfidence,
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr()), kalmanSI));

        bsmCheck.setKalmanSCC(
                KalmanSpeedConsistancyCheck(&bsm->getSenderSpeed(),
                        &bsm->getSenderAccel(),
                        &bsm->getSenderSpeedConfidence(),
                        mdmLib.calculateDeltaTime(bsm,
                                senderNode->getLatestBSMAddr()), kalmanVI));

    } else {
        bsmCheck.setSuddenAppearence(
                SuddenAppearenceCheck(&senderPos, &myPosition));
    }

    //PrintBsmCheck(senderPseudonym, bsmCheck);

    return bsmCheck;
}

void LegacyChecks::PrintBsmCheck(unsigned long senderPseudonym,
        BsmCheck bsmCheck) {

    if (bsmCheck.getRangePlausibility() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "ART FAILED => "
                << bsmCheck.getRangePlausibility() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getPositionConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "MGTD FAILED => "
                << bsmCheck.getPositionConsistancy() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getSpeedConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "MGTS FAILED => "
                << bsmCheck.getSpeedConsistancy() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }

    if (bsmCheck.getPositionSpeedConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "MGTSV FAILED => "
                << bsmCheck.getPositionSpeedConsistancy() << " A:"
                << myPseudonym << " B:" << senderPseudonym << '\n';
    }

    if (bsmCheck.getPositionSpeedMaxConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "MGTSVM FAILED => "
                << bsmCheck.getPositionSpeedMaxConsistancy() << " A:"
                << myPseudonym << " B:" << senderPseudonym << '\n';
    }

    if (bsmCheck.getSpeedPlausibility() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "MAXS FAILED => "
                << bsmCheck.getSpeedPlausibility() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }
    if (bsmCheck.getPositionPlausibility() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "MAP FAILED => "
                << bsmCheck.getPositionPlausibility() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }

//    if (bsmCheck.getSuddenAppearence() < 0.5) {
//        std::cout << "^^^^^^^^^^^V1 " << "SAW FAILED => "
//                << bsmCheck.getSuddenAppearence() << " A:" << myPseudonym
//                << " B:" << senderPseudonym << '\n';
//    }

    if (bsmCheck.getPositionHeadingConsistancy() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "PHC FAILED => "
                << bsmCheck.getPositionHeadingConsistancy() << " A:"
                << myPseudonym << " B:" << senderPseudonym << '\n';

    }

    if (bsmCheck.getBeaconFrequency() < 0.5) {
        std::cout << "^^^^^^^^^^^V1 " << "FREQ FAILED => "
                << bsmCheck.getBeaconFrequency() << " A:" << myPseudonym
                << " B:" << senderPseudonym << '\n';

    }

    InterTest inter = bsmCheck.getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {
        if (inter.getInterValue(var) < 0.5) {
            std::cout << "^^^^^^^^^^^V1 " << "INT FAILED => "
                    << inter.getInterValue(var) << " A:" << myPseudonym << " B:"
                    << senderPseudonym << " C:" << inter.getInterId(var)
                    << '\n';

        }
    }

}

