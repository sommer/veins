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

#include <veins/modules/application/f2mdVeinsApp/mdApplications/PyBridgeApp.h>


using namespace std;
using namespace boost;

PyBridgeApp::PyBridgeApp(int version,int port, std::string host):
                MDApplication(version)  {
    this->port = port;
    this->host = host;
    httpr = HTTPRequest(port, "localhost");
    bsmPrint = BsmPrintable();
}

void PyBridgeApp::setMyId(LAddress::L2Type myId){
    this->myId = myId;
}

bool PyBridgeApp::CheckNodeForReport(unsigned long myPseudonym,
        BasicSafetyMessage * bsm, BsmCheck * bsmCheck, NodeTable * detectedNodes) {

    minFactor = 1;
    int Threshold = 0.5;
    prntApp->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);
    prntAppInst->incAll(mbTypes::intMbs[bsm->getSenderMbType()]);

    if(bsmCheck->getRangePlausibility()<minFactor){
        minFactor = bsmCheck->getRangePlausibility();
    }
    if (bsmCheck->getRangePlausibility() <= Threshold) {
        prntApp->incFlags(mdChecksTypes::RangePlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::RangePlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getRangePlausibility()<minFactor){
        minFactor = bsmCheck->getRangePlausibility();
    }
    if (bsmCheck->getPositionConsistancy() <= Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getPositionSpeedConsistancy()<minFactor){
        minFactor = bsmCheck->getPositionSpeedConsistancy();
    }
    if (bsmCheck->getPositionSpeedConsistancy() <= Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionSpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionSpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getSpeedConsistancy()<minFactor){
        minFactor = bsmCheck->getSpeedConsistancy();
    }
    if (bsmCheck->getSpeedConsistancy() <= Threshold) {
        prntApp->incFlags(mdChecksTypes::SpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getSpeedPlausibility()<minFactor){
        minFactor = bsmCheck->getSpeedPlausibility();
    }
    if (bsmCheck->getSpeedPlausibility() <= Threshold) {
        prntApp->incFlags(mdChecksTypes::SpeedPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::SpeedPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getPositionPlausibility()<minFactor){
        minFactor = bsmCheck->getPositionPlausibility();
    }
    if (bsmCheck->getPositionPlausibility() <= Threshold) {
        prntApp->incFlags(mdChecksTypes::PositionPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionPlausibility, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    if(bsmCheck->getBeaconFrequency()<minFactor){
        minFactor = bsmCheck->getBeaconFrequency();
    }
    if (bsmCheck->getBeaconFrequency() <= Threshold) {
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
        prntApp->incFlags(mdChecksTypes::PositionHeadingConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incFlags(mdChecksTypes::PositionHeadingConsistancy, mbTypes::intMbs[bsm->getSenderMbType()]);
    }

    InterTest inter = bsmCheck->getIntersection();
    for (int var = 0; var < inter.getInterNum(); ++var) {

        double IT = inter.getInterValue(var);

        if(IT<minFactor){
            minFactor = IT;
        }
    }

    bsmPrint.setReceiverId(myId);
    bsmPrint.setReceiverPseudo(myPseudonym);
    bsmPrint.setBsm(*bsm);
    bsmPrint.setBsmCheck(*bsmCheck);

    std::string s = bsmPrint.getBsmPrintableJson();

    s.erase(std::remove(s.begin(), s.end(), '\t'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());

    std::string response = httpr.Request(s);

    //std::cout << "response:" << response << "\n";

    if (!response.compare("True")) {
        prntApp->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
        prntAppInst->incCumulFlags(mbTypes::intMbs[bsm->getSenderMbType()]);
        return true;
    }
    return false;
}

double PyBridgeApp::getMinFactor() {
    return minFactor;
}
