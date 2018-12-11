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
#include <veins/modules/application/f2mdVeinsApp/mdApplications/ThresholdApp.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;
using namespace boost;

ThresholdApp::ThresholdApp(int version ,double Threshold):
                MDApplication(version) {
    this->Threshold = Threshold;
}

bool ThresholdApp::CheckNodeForReport(unsigned long myPseudonym, BasicSafetyMessage * bsm,
        BsmCheck * bsmCheck, NodeTable * detectedNodes){

    bool checkFailed = false;

    MDReport mbReport;

    minFactor = 1;

    prntApp->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);
    prntAppInst->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);

    if(bsmCheck->getRangePlausibility()<minFactor){
        minFactor = bsmCheck->getRangePlausibility();
    }
    if (bsmCheck->getRangePlausibility() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::RangePlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::RangePlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getRangePlausibility()<minFactor){
        minFactor = bsmCheck->getRangePlausibility();
    }
    if (bsmCheck->getPositionConsistancy() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }


    if(bsmCheck->getPositionSpeedConsistancy()<minFactor){
        minFactor = bsmCheck->getPositionSpeedConsistancy();
    }
    if (bsmCheck->getPositionSpeedConsistancy() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionSpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionSpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getSpeedConsistancy()<minFactor){
        minFactor = bsmCheck->getSpeedConsistancy();
    }
    if (bsmCheck->getSpeedConsistancy() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::SpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getSpeedPlausibility()<minFactor){
        minFactor = bsmCheck->getSpeedPlausibility();
    }
    if (bsmCheck->getSpeedPlausibility() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::SpeedPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getPositionPlausibility()<minFactor){
        minFactor = bsmCheck->getPositionPlausibility();
    }
    if (bsmCheck->getPositionPlausibility() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getBeaconFrequency()<minFactor){
        minFactor = bsmCheck->getBeaconFrequency();
    }
    if (bsmCheck->getBeaconFrequency() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::BeaconFrequency, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::BeaconFrequency, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getSuddenAppearence()<minFactor){
   //     minFactor = bsmCheck->getSuddenAppearence();
    }
    if (bsmCheck->getSuddenAppearence() <= Threshold) {
   //     checkFailed = true;
        prntApp->incFlags(mdChecksTypes::SuddenAppearence, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SuddenAppearence, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if( bsmCheck->getPositionHeadingConsistancy()<minFactor){
        minFactor = bsmCheck->getPositionHeadingConsistancy();
    }
    if (bsmCheck->getPositionHeadingConsistancy() <= Threshold) {
        checkFailed = true;
        prntApp->incFlags(mdChecksTypes::PositionHeadingConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionHeadingConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    bool minInterFound = false;

    InterTest inter = bsmCheck->getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {

        double IT = inter.getInterValue(var);

        if(IT<minFactor){
            minFactor = IT;
        }

        if (IT <= Threshold) {
            checkFailed = true;
            if (!minInterFound) {
                prntApp->incFlags(mdChecksTypes::Intersection, mbTypes::intMbs[bsm->getSenderMbType()]);
                prntAppInst->incFlags(mdChecksTypes::Intersection, mbTypes::intMbs[bsm->getSenderMbType()]);
                minInterFound = true;
            }
        }

    }

    if (checkFailed) {
        prntApp->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);

//        if(bsm->getSenderMbType() == 0){
//            prntApp->printOutDebug();
//        }

    }

    return checkFailed;
}

double ThresholdApp::getMinFactor(){
    return minFactor;
}
