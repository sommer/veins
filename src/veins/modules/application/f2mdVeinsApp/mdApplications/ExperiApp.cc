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
#include <veins/modules/application/f2mdVeinsApp/mdApplications/ExperiApp.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace boost;

ExperiApp::ExperiApp(int version, double deltaTrustTime,
        int maxBsmTrustNum, double Augmentation):
                MDApplication(version)  {

    this->deltaTrustTime = deltaTrustTime;
    this->maxBsmTrustNum = maxBsmTrustNum;
    this->Augmentation = Augmentation;
}

bool ExperiApp::CheckNodeForReport(unsigned long myPseudonym,
        BasicSafetyMessage * bsm, BsmCheck * bsmCheck, NodeTable * detectedNodes) {

    bool checkFailed = false;
    MDReport mbReport;

    prntApp->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);
    prntAppInst->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);

    double temp = 0;

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

    tuple<double, int> zeroNum = getZeroNumber(bsm, bsmCheck, detectedNodes);
    double zeroSum = get < 0 > (zeroNum);
    int zeroCount = get < 1 > (zeroNum);

    double factorList[bsmCheckListSize];

    //std::cout<< "RangePlausibility" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getRangePlausibility();
    }

    temp = AggregateFactorsListDouble(bsmCheck->getRangePlausibility(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }

    if (temp < Threshold) {
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

    temp = AggregateFactorsListDouble(bsmCheck->getPositionConsistancy(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }

    if (temp < Threshold) {
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
    temp = AggregateFactorsListDouble(bsmCheck->getPositionSpeedConsistancy(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }
    if (temp < Threshold) {
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
    temp = AggregateFactorsListDouble(bsmCheck->getSpeedConsistancy(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }
    if (temp < Threshold) {
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
    temp = AggregateFactorsListDouble(bsmCheck->getSpeedPlausibility(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }
    if (temp < Threshold) {
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
    temp = AggregateFactorsListDouble(bsmCheck->getPositionPlausibility(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }
    if (temp < Threshold) {
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
    temp = AggregateFactorsListDouble(bsmCheck->getBeaconFrequency(), factorList,
            bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }
    if (temp < Threshold) {
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
    temp = AggregateFactorsListDouble(bsmCheck->getSuddenAppearence(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
//        minFactor = temp;
    }
    if (temp < Threshold) {
        prntApp->incFlags(mdChecksTypes::SuddenAppearence,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SuddenAppearence,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionHeadingConsistancy" << '\n';
    for (int var = 0; var < bsmCheckListSize; ++var) {
        factorList[var] = bsmCheckList[var].getPositionHeadingConsistancy();
    }
    temp = AggregateFactorsListDouble(bsmCheck->getPositionHeadingConsistancy(),
            factorList, bsmCheckListSize, zeroSum, zeroCount);
    if (temp < minFactor) {
        minFactor = temp;
    }
    if (temp < Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionHeadingConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionHeadingConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    for (int var = 0; var < bsmCheckListSize; ++var) {
        bsmCheckList[var].getIntersection();
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
        temp = AggregateFactorsListDouble(curInferFactor, factorList,
                bsmCheckListSize, zeroSum, zeroCount);
        if (temp < minFactor) {
            minFactor = temp;
        }
        if (temp < Threshold) {
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

std::tuple<double, int> ExperiApp::getZeroNumber(BasicSafetyMessage * bsm,
        BsmCheck * bsmCheck, NodeTable * detectedNodes) {

    double zeroSum = 0;
    int zeroCount = 0;

    int senderId = bsm->getSenderPseudonym();

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

    //std::cout<< "RangePlausibility" << '\n';
    if (bsmCheck->getRangePlausibility() <= 0) {
        zeroSum = zeroSum + bsmCheck->getRangePlausibility();
        zeroCount++;
    }
    if (bsmCheck->getPositionConsistancy() <= 0) {
        zeroSum = zeroSum + bsmCheck->getPositionConsistancy();
        zeroCount++;
    }
    if (bsmCheck->getPositionSpeedConsistancy() <= 0) {
        zeroSum = zeroSum + bsmCheck->getPositionSpeedConsistancy();
        zeroCount++;
    }
    if (bsmCheck->getSpeedConsistancy() <= 0) {
        zeroSum = zeroSum + bsmCheck->getSpeedConsistancy();
        zeroCount++;
    }
    if (bsmCheck->getSpeedPlausibility() <= 0) {
        zeroSum = zeroSum + bsmCheck->getSpeedPlausibility();
        zeroCount++;
    }
    if (bsmCheck->getPositionPlausibility() <= 0) {
        zeroSum = zeroSum + bsmCheck->getPositionPlausibility();
        zeroCount++;
    }
    if (bsmCheck->getBeaconFrequency() <= 0) {
        zeroSum = zeroSum + bsmCheck->getBeaconFrequency();
        zeroCount++;
    }
//    if(bsmCheck->getSuddenAppearence()<=0){
//        zeroNumber++;
//    }
    if (bsmCheck->getPositionHeadingConsistancy() <= 0) {
        zeroSum = zeroSum + bsmCheck->getPositionHeadingConsistancy();
        zeroCount++;
    }

    for (int var = 0; var < bsmCheckListSize; ++var) {
        if (bsmCheckList[var].getRangePlausibility() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getRangePlausibility();
            zeroCount++;
        }
        if (bsmCheckList[var].getPositionConsistancy() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getPositionConsistancy();
            zeroCount++;
        }
        if (bsmCheckList[var].getPositionSpeedConsistancy() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getPositionSpeedConsistancy();
            zeroCount++;
        }
        if (bsmCheckList[var].getSpeedConsistancy() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getSpeedConsistancy();
            zeroCount++;
        }
        if (bsmCheckList[var].getSpeedPlausibility() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getSpeedPlausibility();
            zeroCount++;
        }
        if (bsmCheckList[var].getPositionPlausibility() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getPositionPlausibility();
            zeroCount++;
        }
        if (bsmCheckList[var].getBeaconFrequency() <= 0) {
            zeroSum = zeroSum + bsmCheckList[var].getBeaconFrequency();
            zeroCount++;
        }
//        if(bsmCheckList[var].getSuddenAppearence()<=0){
//            zeroNumber++;
//        }
        if (bsmCheckList[var].getPositionHeadingConsistancy() <= 0) {
            zeroSum = zeroSum
                    + bsmCheckList[var].getPositionHeadingConsistancy();
            zeroCount++;
        }
    }

    InterTest inter = bsmCheck->getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {
        double curInferFactor = inter.getInterValue(var);

        for (int i = 0; i < bsmCheckListSize; ++i) {
            double IdIndex = bsmCheckList[i].getIntersection().getIdIndex(
                    inter.getInterId(var));
            if (IdIndex != -1) {
                if (bsmCheckList[i].getIntersection().getInterValue(IdIndex)
                        <= 0) {
                    zeroSum = zeroSum
                            + bsmCheckList[i].getIntersection().getInterValue(
                                    IdIndex);
                    zeroCount++;
                }
            }
        }

        if (curInferFactor <= 0) {
            zeroSum = curInferFactor;
            zeroCount++;
        }
    }

    return std::make_tuple(zeroSum, zeroCount);
}

double ExperiApp::AggregateFactorsListDouble(double curFactor,
        double *factorList, int factorListSize, double zeroSum, int zeroCount) {

    double averageFactor = curFactor;
    for (int var = 0; var < factorListSize; ++var) {
        averageFactor = averageFactor + factorList[var];
    }
    averageFactor = averageFactor / (factorListSize + 1);
    double addPlus = mdmLib.boundedGaussianSum(-zeroCount / 2, zeroCount / 2,
            (1 / Augmentation));

    return averageFactor - addPlus;

}

//double ExperiApp::AggregateFactorsListDouble(double curFactor, double *factorList,
//        int factorListSize,double zeroSum, int zeroCount) {
//    if (version == 1) {
//        double averageFactor = curFactor;
//        for (int var = 0; var < factorListSize; ++var) {
//            averageFactor = averageFactor + factorList[var];
//        }
//        averageFactor = averageFactor / (factorListSize + 1);
//        double addPlus = mdmLib.boundedGaussianSum(-zeroCount/2,zeroCount/2, (1 / Augmentation));
//
//        return averageFactor - addPlus;
//        return averageFactor;
//    } else {
//        if (curFactor <= 0) {
//            return 0;
//        } else {
//            double averageFactor = curFactor;
//            for (int var = 0; var < factorListSize; ++var) {
//                averageFactor = averageFactor + factorList[var];
//            }
//            averageFactor = averageFactor / (factorListSize + 1);
//            double addPlus = mdmLib.boundedGaussianSum(-zeroCount/2,zeroCount/2, (1 / Augmentation));
//
//            return averageFactor-addPlus;
//        }
//    }
//}

//best rate / faulty
//bool ExperiApp::AggregateFactorsList(double curFactor, double *factorList,
//        int factorListSize, double zeroSum, int zeroCount) {
//    if (version == 1) {
//        double averageFactor = curFactor;
//
//        for (int var = 0; var < factorListSize; ++var) {
//            averageFactor = averageFactor + factorList[var];
//        }
//        averageFactor = averageFactor / (factorListSize + 1);
//
//        double newTh = 0.0;
//        double addPlus = mdmLib.boundedGaussianSum(-zeroCount/2,zeroCount/2, (1 / Augmentation));
//        newTh = newTh + addPlus;
//        if (averageFactor <= newTh) {
//            return true;
//        } else {
//            return false;
//        }
//    } else {
//        if (curFactor <= 0) {
//            return true;
//        } else {
//            double averageFactor = curFactor;
//
//            for (int var = 0; var < factorListSize; ++var) {
//                averageFactor = averageFactor + factorList[var];
//            }
//            averageFactor = averageFactor / (factorListSize + 1);
//
//            double newTh = 0.0;
//            double addPlus = mdmLib.boundedGaussianSum(-zeroCount/2,zeroCount/2, (1 / Augmentation));
//            newTh = newTh + addPlus;
//
//            if ((averageFactor) <= newTh) {
//                return true;
//            } else {
//                return false;
//            }
//        }
//    }
//}

double ExperiApp::getMinFactor() {
    return minFactor;
}

