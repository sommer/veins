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

#include <veins/modules/application/f2md/mdBase/BsmCheck.h>

BsmCheck::BsmCheck() {
     rangePlausibility = 1;
     speedConsistancy = 1;
     positionConsistancy = 1;
     speedPlausibility = 1;
     positionSpeedConsistancy = 1;
     positionSpeedMaxConsistancy = 1;
     intersection = InterTest();
     suddenAppearence = 1;
     beaconFrequency = 1;
     positionPlausibility = 1;
     positionHeadingConsistancy = 1;
     kalmanPSCP=1;
     kalmanPSCS=1;
     kalmanPSCSP=1;
     kalmanPSCSS=1;
     kalmanPCC=1;
     kalmanPACS=1;
     kalmanSCC=1;

     reported = false;
}

double BsmCheck::getRangePlausibility() {
    return rangePlausibility;
}

double BsmCheck::getPositionSpeedConsistancy() {
    return positionSpeedConsistancy;
}

double BsmCheck::getPositionSpeedMaxConsistancy() {
    return positionSpeedMaxConsistancy;
}

double BsmCheck::getPositionConsistancy() {
    return positionConsistancy;
}

double BsmCheck::getSpeedConsistancy() {
    return speedConsistancy;
}

double BsmCheck::getSpeedPlausibility() {
    return speedPlausibility;
}

InterTest BsmCheck::getIntersection() {
    return intersection;
}

double BsmCheck::getSuddenAppearence() {
    return suddenAppearence;
}

double BsmCheck::getBeaconFrequency() {
    return beaconFrequency;
}

double BsmCheck::getPositionPlausibility() {
    return positionPlausibility;
}

double BsmCheck::getPositionHeadingConsistancy() {
    return positionHeadingConsistancy;
}

bool BsmCheck::getReported() {
    return reported;
}

void BsmCheck::setRangePlausibility(double rangePlausibility) {
    this->rangePlausibility = rangePlausibility;
}

void BsmCheck::setPositionConsistancy(double positionConsistancy) {
    this->positionConsistancy = positionConsistancy;
}

void BsmCheck::setSpeedConsistancy(double speedConsistancy) {
    this->speedConsistancy = speedConsistancy;
}

void BsmCheck::setSpeedPlausibility(double speedPlausibility) {
    this->speedPlausibility = speedPlausibility;
}

void BsmCheck::setIntersection(InterTest intersection) {
    this->intersection = intersection;
}

void BsmCheck::setSuddenAppearence(double suddenAppearence) {
    this->suddenAppearence = suddenAppearence;
}

void BsmCheck::setBeaconFrequency(double beaconFrequency) {
    this->beaconFrequency = beaconFrequency;
}

void BsmCheck::setPositionPlausibility(double positionPlausibility) {
    this->positionPlausibility = positionPlausibility;
}

void BsmCheck::setPositionSpeedConsistancy(double positionSpeedConsistancy) {
    this->positionSpeedConsistancy = positionSpeedConsistancy;
}

void BsmCheck::setPositionSpeedMaxConsistancy(double positionSpeedMaxConsistancy) {
    this->positionSpeedMaxConsistancy = positionSpeedMaxConsistancy;
}

void BsmCheck::setPositionHeadingConsistancy(double positionHeadingConsistancy) {
    this->positionHeadingConsistancy = positionHeadingConsistancy;
}

void BsmCheck::setReported(bool reported) {
    this->reported = reported;
}

double BsmCheck::getKalmanPACS() {
    return kalmanPACS;
}

void BsmCheck::setKalmanPACS(double kalmanPacs) {
    kalmanPACS = kalmanPacs;
}

double BsmCheck::getKalmanPCC() {
    return kalmanPCC;
}

void BsmCheck::setKalmanPCC(double kalmanPcc) {
    kalmanPCC = kalmanPcc;
}

double BsmCheck::getKalmanPSCP() {
    return kalmanPSCP;
}

void BsmCheck::setKalmanPSCP(double kalmanPscp) {
    kalmanPSCP = kalmanPscp;
}

double BsmCheck::getKalmanPSCS() {
    return kalmanPSCS;
}

void BsmCheck::setKalmanPSCS(double kalmanPscs) {
    kalmanPSCS = kalmanPscs;
}

double BsmCheck::getKalmanPSCSP() {
    return kalmanPSCSP;
}

void BsmCheck::setKalmanPSCSP(double kalmanPscsp) {
    kalmanPSCSP = kalmanPscsp;
}

double BsmCheck::getKalmanPSCSS() {
    return kalmanPSCSS;
}

void BsmCheck::setKalmanPSCSS(double kalmanPscss) {
    kalmanPSCSS = kalmanPscss;
}

double BsmCheck::getKalmanSCC() {
    return kalmanSCC;
}

void BsmCheck::setKalmanSCC(double kalmanScc) {
    kalmanSCC = kalmanScc;
}
