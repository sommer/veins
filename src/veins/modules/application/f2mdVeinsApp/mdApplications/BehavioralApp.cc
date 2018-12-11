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
#include <veins/modules/application/f2mdVeinsApp/mdApplications/BehavioralApp.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;
using namespace boost;

BehavioralApp::BehavioralApp(int version,
        double Threshold) :
        MDApplication(version) {
    this->Threshold = Threshold;
}

bool BehavioralApp::CheckNodeForReport(unsigned long myPseudonym,
        BasicSafetyMessage * bsm, BsmCheck * bsmCheck, NodeTable * detectedNodes) {

    MDReport mbReport;

    double tempFactor = 0;
    minFactor = 1;

    prntApp->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);
    prntAppInst->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);

    unsigned long senderId = bsm->getSenderPseudonym();

    //std::cout<< "RangePlausibility" << '\n';

    tempFactor = bsmCheck->getRangePlausibility();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::RangePlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::RangePlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionConsistancy" << '\n';
    tempFactor = bsmCheck->getPositionConsistancy();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionSpeedConsistancy" << '\n';

    tempFactor = bsmCheck->getPositionSpeedConsistancy();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionSpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionSpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "SpeedConsistancy" << '\n';

    tempFactor = bsmCheck->getSpeedConsistancy();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::SpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "SpeedPlausibility" << '\n';

    tempFactor = bsmCheck->getSpeedPlausibility();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::SpeedPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionPlausibility" << '\n';

    tempFactor = bsmCheck->getPositionPlausibility();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionPlausibility,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "BeaconFrequency" << '\n';

    tempFactor = bsmCheck->getBeaconFrequency();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::BeaconFrequency,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::BeaconFrequency,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "SuddenAppearence" << '\n';
    tempFactor = bsmCheck->getSuddenAppearence();
    if (tempFactor < minFactor) {
        //std::cout<< "SuddenAppearence" << '\n';
        //     minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::SuddenAppearence,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SuddenAppearence,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    //std::cout<< "PositionHeadingConsistancy" << '\n';

    tempFactor = bsmCheck->getPositionHeadingConsistancy();
    if (tempFactor < minFactor) {
        minFactor = tempFactor;
    }
    if (tempFactor < Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionHeadingConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionHeadingConsistancy,
                mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    InterTest inter = bsmCheck->getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {
        double curInferFactor = inter.getInterValue(var);
        //std::cout<< "Intersection" << '\n';
        tempFactor = curInferFactor;
        if (tempFactor < minFactor) {
            minFactor = tempFactor;
        }
        if (tempFactor < Threshold) {
            prntApp->incFlags(mdChecksTypes::Intersection,
                    mbTypes::intMbs[bsm->getSenderMbType()]);
            prntAppInst->incFlags(mdChecksTypes::Intersection,
                    mbTypes::intMbs[bsm->getSenderMbType()]);
        }
    }

    bool checkFailed = false;

    int indexTMO = getIndexTMO(senderId);

    if (minFactor < 1) {

        if (indexTMO == -1) {
            indexTMO = addPseudoTMO(senderId);
        }

        double TMOadd = 0;
        //        int TMOadd = 10*(Threshold - minFactor);
        double augFactor = exp(TimeOut[indexTMO]+2 / 2) / 2.9;

        if (augFactor > 20) {
            augFactor = 20;
        }
        augFactor = 1;
        double expAdd = 10 * (1 - minFactor);
        double expV = expAdd / 1.1 - 6;
        TMOadd = (exp(expV) + 0.5) * augFactor;


//        for (double var = 1; var >= 0; var = var - 0.1) {
//            double expAdd = 10 * (1 - var);
//            double expV = expAdd / 3 - 3;
//            int TMOadd = exp(expV) + 0.5;
//            std::cout << var << " " << expAdd << " " << expV << " " << TMOadd
//                    << "\n";
//        }
//
//        for (double var = 0; var < 10; ++var) {
//            double RealAdd = exp(var/5);
//            std::cout << var << " RealAdd:" << RealAdd <<" "<< (1- 1/RealAdd)<< "\n";
//        }
//        exit(0);

//        if(expAdd>=10){
//            TMOadd = 10;
//        }else{
//            TMOadd = 0;
//        }


        TimeOut[indexTMO] = TimeOut[indexTMO] + TMOadd;
        UpdatedTMO[TimeOutNum] = simTime().dbl();
        //std::cout<<version<<"=>"<<10*(Threshold - minFactor)<<":"<< TimeOut[indexTMO]<<"\n";
    }

    if (indexTMO >= 0) {
        if (TimeOut[indexTMO] > 0) {
            prntApp->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
            prntAppInst->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
            bsmCheck->setReported(true);
            checkFailed = true;
            TimeOut[indexTMO] = TimeOut[indexTMO] - 1;
        }
        if (TimeOut[indexTMO] == 0) {
            removePseudoTMO(indexTMO);
        }
    }
    return checkFailed;
}

int BehavioralApp::addPseudoTMO(unsigned long pseudo) {
    if (TimeOutNum >= MAX_DETECTED_NODES) {
        removeOldestPseudoTMO();
    }
    TimeOut[TimeOutNum] = 0;
    UpdatedTMO[TimeOutNum] = simTime().dbl();
    PseudonymsToTMO[TimeOutNum] = pseudo;
    TimeOutNum++;

    //std::cout<<"TimeOutNum:"<<TimeOutNum<<"\n";

    return TimeOutNum - 1;
}

void BehavioralApp::removeOldestPseudoTMO() {
    int oldestIndex = 0;
    double oldestTime = UpdatedTMO[oldestIndex];
    for (int var = 0; var < TimeOutNum; ++var) {
        if (oldestTime > UpdatedTMO[var]) {
            oldestTime = UpdatedTMO[var];
            oldestIndex = var;
        }
    }
    removePseudoTMO(oldestIndex);
}

void BehavioralApp::removePseudoTMO(int index) {
    for (int var = index; var < TimeOutNum; ++var) {
        TimeOut[var] = TimeOut[var + 1];
    }
    TimeOutNum--;
}

int BehavioralApp::getIndexTMO(unsigned long pseudo) {
    for (int var = 0; var < TimeOutNum; ++var) {
        if (PseudonymsToTMO[var] == pseudo) {
            return var;
        }
    }
    return -1;
}

double BehavioralApp::getMinFactor() {
    return minFactor;
}

