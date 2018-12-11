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
#include <veins/modules/application/f2mdVeinsApp/mdApplications/AggregationApp.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;
using namespace boost;

AggrigationApp::AggrigationApp(int version, double devValue, double deltaTrustTime,
        int maxBsmTrustNum) :
        MDApplication(version) {

    this->devValue = devValue;
    this->deltaTrustTime = deltaTrustTime;
    this->maxBsmTrustNum = maxBsmTrustNum;
}

bool AggrigationApp::CheckNodeForReport(unsigned long myPseudonym,
        BasicSafetyMessage * bsm, BsmCheck * bsmCheck, NodeTable * detectedNodes) {

    bool checkFailed = false;
    MDReport mbReport;

    double tempFactor = 0;
    minFactor = 1;

    prntApp->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);
    prntAppInst->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);

    unsigned long senderId = bsm->getSenderPseudonym();

    MDMHistory * mdmHist = detectedNodes->getMDMHistoryAddr(senderId);
    NodeHistory * nodeHist = detectedNodes->getNodeHistoryAddr(senderId);

    BsmCheck bsmCheckList[maxBsmTrustNum];
    int bsmCheckListSize = 0;

    for (int var = 0; var < nodeHist->getBSMNum(); ++var) {
        if (bsmCheckListSize < maxBsmTrustNum) {
            if (mdmLib.calculateDeltaTime(bsm, nodeHist->getBSMAddr(var))
                    < deltaTrustTime) {
                bsmCheckList[bsmCheckListSize] = mdmHist->getBsmCheck(var,
                        version);
                bsmCheckListSize++;
            }
        }
    }

    double factorList[bsmCheckListSize];

    //std::cout<< "RangePlausibility" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getRangePlausibility();
    }

    tempFactor = AggregateFactorsListDouble(bsmCheck->getRangePlausibility(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;

        prntApp->incFlags(mdChecksTypes::RangePlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::RangePlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionConsistancy" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getPositionConsistancy();
    }
    tempFactor = AggregateFactorsListDouble(bsmCheck->getPositionConsistancy(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionSpeedConsistancy" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getPositionSpeedConsistancy();
    }
    tempFactor = AggregateFactorsListDouble(
            bsmCheck->getPositionSpeedConsistancy(), factorList,
            bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionSpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionSpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "SpeedConsistancy" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getSpeedConsistancy();
    }
    tempFactor = AggregateFactorsListDouble(bsmCheck->getSpeedConsistancy(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::SpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "SpeedPlausibility" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getSpeedPlausibility();
    }
    tempFactor = AggregateFactorsListDouble(bsmCheck->getSpeedPlausibility(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::SpeedPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionPlausibility" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getPositionPlausibility();
    }
    tempFactor = AggregateFactorsListDouble(bsmCheck->getPositionPlausibility(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "BeaconFrequency" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getBeaconFrequency();
    }
    tempFactor = AggregateFactorsListDouble(bsmCheck->getBeaconFrequency(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::BeaconFrequency,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::BeaconFrequency,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "SuddenAppearence" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getSuddenAppearence();
    }
    tempFactor = AggregateFactorsListDouble(bsmCheck->getSuddenAppearence(),
            factorList, bsmCheckListSize);
    if (tempFactor < minFactor) {
        //     temp = minFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::SuddenAppearence,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SuddenAppearence,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionHeadingConsistancy" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getPositionHeadingConsistancy();
    }
    tempFactor = AggregateFactorsListDouble(
            bsmCheck->getPositionHeadingConsistancy(), factorList,
            bsmCheckListSize);
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionHeadingConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionHeadingConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    InterTest inter = bsmCheck->getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {
        double curInferFactor = inter.getInterValue(var);

        for (int i = 0; i < bsmCheckListSize; ++i) {
            double IdIndex = bsmCheckList[i].getIntersection().getIdIndex(
                    inter.getInterId(var));
            if (IdIndex != -1) {
                factorList[i] = bsmCheckList[i].getIntersection().getInterValue(
                        IdIndex);
            } else {
                factorList[i] = 1;
            }
        }
        //std::cout<< "Intersection" << '\n';
        tempFactor = AggregateFactorsListDouble(curInferFactor, factorList,
                bsmCheckListSize);
        if (tempFactor < minFactor) {
            minFactor = tempFactor;
        }
        if (tempFactor < Threshold) {
            checkFailed = true;
            prntApp->incFlags(mdChecksTypes::Intersection,
                    mbTypes::intMbs[bsm->getSenderMbType()]);
            prntAppInst->incFlags(mdChecksTypes::Intersection,
                    mbTypes::intMbs[bsm->getSenderMbType()]);
        }
    }

    if (checkFailed) {
        prntApp->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
        bsmCheck->setReported(true);
    }
    return checkFailed;
}

//double AggrigationApp::AggregateFactorsListDouble(double curFactor,
//        double *factorList, int factorListSize) {
//    double averageFactor = curFactor;
//    double divValue = 1;
//    for (int var = 0; var < factorListSize; ++var) {
//        double decValue =  pow(devValue, var+1);
//        averageFactor = averageFactor + factorList[var]*decValue;
//        divValue = divValue + decValue;
//    }
//    averageFactor = averageFactor / divValue;
//    return averageFactor;
//}

double AggrigationApp::getMinFactor() {
    return minFactor;
}

double AggrigationApp::AggregateFactorsListDouble(double curFactor, double *factorList,
        int factorListSize) {
    if (version == 1) {
        double averageFactor = curFactor;
        for (int var = 0; var < factorListSize; ++var) {
            averageFactor = averageFactor + factorList[var];
        }
        averageFactor = averageFactor / (factorListSize + 1);

        return averageFactor;
    } else {
        if (curFactor <= 0) {
            return 0;
        } else {
            double averageFactor = curFactor;
            for (int var = 0; var < factorListSize; ++var) {
                averageFactor = averageFactor + factorList[var];
            }
            averageFactor = averageFactor / (factorListSize + 1);
            return averageFactor;
        }
    }
}

//best rate / faulty
//bool AggrigationApp::AggregateFactorsList(double curFactor, double *factorList,
//        int factorListSize) {
//    if (version == 1) {
//        double averageFactor = curFactor;
//        for (int var = 0; var < factorListSize; ++var) {
//            averageFactor = averageFactor + factorList[var];
//        }
//        averageFactor = averageFactor / (factorListSize + 1);
//        if ((averageFactor) <= 0.5) {
//            return true;
//        } else {
//            return false;
//        }
//    } else {
//        if (curFactor <= 0) {
//            return true;
//        } else {
//            double averageFactor = curFactor;
//            for (int var = 0; var < factorListSize; ++var) {
//                averageFactor = averageFactor + factorList[var];
//            }
//            averageFactor = averageFactor / (factorListSize + 1);
//
//            if ((averageFactor) <= 0.5) {
//                return true;
//            } else {
//                return false;
//            }
//        }
//    }
//}

