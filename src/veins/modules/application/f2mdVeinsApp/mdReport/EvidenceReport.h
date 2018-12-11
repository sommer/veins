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

#ifndef __VEINS_EvidenceReport_H_
#define __VEINS_EvidenceReport_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdMessages/BasicSafetyMessage_m.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/ReportPrintable.h>
#include "../BaseWaveApplLayer.h"
#include "../mdSupport/XmlWriter.h"
#include "../mdSupport/JsonWriter.h"
#include "../mdBase/NodeTable.h"

using namespace omnetpp;

#define MAX_EVI_BSM 25

class EvidenceReport: public MDReport {

private:
    void setReportedCheck(BsmCheck reportedCheck);
    void addBsmToList(BasicSafetyMessage bsm);

    BsmCheck reportedCheck;
    BasicSafetyMessage bsmList[MAX_EVI_BSM];
    int bsmListNum;

public:
    EvidenceReport(MDReport baseReport);
    void addEvidence(BasicSafetyMessage myBsm, BsmCheck reportedCheck, BasicSafetyMessage receivedBsm,
            NodeTable detectedNodes);
    std::string getReportXml();
    std::string getReportPrintableJson();
};

#endif
