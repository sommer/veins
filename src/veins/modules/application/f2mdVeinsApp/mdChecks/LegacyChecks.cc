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
#include <veins/modules/application/f2mdVeinsApp/mdChecks/LegacyChecks.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>    // std::max

using namespace std;
using namespace boost;

LegacyChecks::LegacyChecks(unsigned long myPseudonym, Coord myPosition,
        Coord mySpeed, Coord myHeading, Coord mySize,Coord myLimits, LinkControl* LinkC) {
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
double S_MaxAccel = 0;
double S_MaxDecel = 0;
double LegacyChecks::SpeedConsistancyCheck(double curSpeed, double oldspeed,
        double time) {

    double speedDelta = curSpeed - oldspeed;

//    double attFact = mdmLib.gaussianSum(1, 1 / 3);
//    if (time >= 1) {
//        attFact = time;
//    }

//    std::cout<<"S_MaxAccel:"<<S_MaxAccel<<"\n";
//    std::cout<<"S_MaxDecel:"<<S_MaxDecel<<"\n";
    if (speedDelta > 0) {

        if(speedDelta>S_MaxAccel*time){
            S_MaxAccel = speedDelta/time;
        }

        if (speedDelta < MAX_PLAUSIBLE_ACCEL * time) {
            return 1;
        } else {
            return 0; //distance
        }
    } else {

        if(fabs(speedDelta)>S_MaxDecel*time){
            S_MaxDecel = fabs(speedDelta)/time;
        }


        if (fabs(speedDelta) < MAX_PLAUSIBLE_DECEL * time) {
            return 1;
        } else {
            return 0; //distance
        }
    }

}

double LegacyChecks::PositionSpeedConsistancyCheck(Coord *curPosition,
        Coord * oldPosition, double curSpeed, double oldspeed, double time) {
    if (time < MAX_TIME_DELTA) {
        double distance = mdmLib.calculateDistancePtr(curPosition, oldPosition);
        double theoreticalSpeed = distance / time;
        //double realspeed = fabs(curSpeed + oldspeed)/2;
        double maxspeed = std::max(curSpeed, oldspeed);
        double minspeed = std::min(curSpeed, oldspeed);

        double deltaMax = maxspeed - theoreticalSpeed;
        double deltaMin = theoreticalSpeed - minspeed ;

        if (deltaMax >  (MAX_PLAUSIBLE_DECEL+MAX_MGT_RNG)*time) {
               return 0; // deltaMax - MIN_PSS
        } else {
            if (deltaMin >  MAX_PLAUSIBLE_ACCEL*time) {
                return 0; // deltaMin - MAX_PSS
            } else {
                return 1;
            }
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
        Coord head1, Coord  nodePosition2, Coord nodeSize2, Coord  head2, double deltaTime) {

    double heading1 = mdmLib.calculateHeadingAnglePtr(&head1);
    double heading2 = mdmLib.calculateHeadingAnglePtr(&head2);

    //double distance = mdmLib.calculateDistancePtr(nodePosition1, nodePosition2);

    double inter = mdmLib.RectRectFactor(nodePosition1, nodePosition2, heading1,
            heading2, nodeSize1, nodeSize2);
    inter = inter * ((MAX_DELTA_INTER-deltaTime)/MAX_DELTA_INTER);

//    if (inter > 0.5){
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
//        exit(0);
//    }

    if (inter > 0.25) {
        return 0; //inter
    } else {
        return 1;
    }

}


InterTest LegacyChecks::MultipleIntersectionCheck(NodeTable * detectedNodes,
        BasicSafetyMessage * bsm) {

    unsigned long senderPseudonym = bsm->getSenderPseudonym();

    NodeHistory *varNode;

    const  int maxInterNum = 100;
     unsigned long resultPseudo[maxInterNum];
     double resultCheck[maxInterNum];

    int INTNum = 0;
    double INTScore = 1;

    INTScore = IntersectionCheck(myPosition, mySize, myHeading, bsm->getSenderPos(),
            Coord(bsm->getSenderWidth(), bsm->getSenderLength()),
            bsm->getSenderHeading(),0.11);


    if (INTScore < 1 && INTNum<maxInterNum) {
        resultPseudo[INTNum] = myPseudonym;
        resultCheck[INTNum] = INTScore;
        INTNum++;
    }

    for (int var = 0; var < detectedNodes->getNodesNum(); ++var) {
        if (detectedNodes->getNodePseudo(var) != senderPseudonym) {
            varNode = detectedNodes->getNodeHistoryAddr(
                    detectedNodes->getNodePseudo(var));
            double deltaTime =mdmLib.calculateDeltaTime(varNode->getLatestBSMAddr(),
                    bsm);
            if (deltaTime < MAX_DELTA_INTER) {

                INTScore = IntersectionCheck(varNode->getSenderPos(0),
                        varNode->getSenderSize(0), varNode->getSenderHeading(0),
                        bsm->getSenderPos(),
                        Coord(bsm->getSenderWidth(), bsm->getSenderLength()),
                        bsm->getSenderHeading(), deltaTime);

                if (INTScore < 1 && INTNum<maxInterNum) {
                    resultPseudo[INTNum] = detectedNodes->getNodePseudo(var);
                    resultCheck[INTNum] = INTScore;
                    INTNum++;
                }
            }
        }
    }

    InterTest intertTest = InterTest(INTNum);

    for (int var = 0; var < INTNum; ++var) {
        intertTest.addInterValue(resultPseudo[var], resultCheck[var]);
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
                curPosition->y - oldPosition->y, curPosition->z - oldPosition->z);
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

BsmCheck LegacyChecks::CheckBSM(BasicSafetyMessage *bsm,
        NodeTable *detectedNodes) {
    BsmCheck bsmCheck = BsmCheck();

    unsigned long senderPseudonym = bsm->getSenderPseudonym();
    Coord senderPos = bsm->getSenderPos();
    Coord senderPosConfidence = bsm->getSenderPosConfidence();

    NodeHistory * senderNode = detectedNodes->getNodeHistoryAddr(senderPseudonym);

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

        bsmCheck.setPositionSpeedConsistancy(
                PositionSpeedConsistancyCheck(&senderPos,
                        &senderNode->getLatestBSMAddr()->getSenderPos(),
                        mdmLib.calculateSpeedPtr(&bsm->getSenderSpeed()),
                        mdmLib.calculateSpeedPtr(
                                &senderNode->getLatestBSMAddr()->getSenderSpeed()),
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

