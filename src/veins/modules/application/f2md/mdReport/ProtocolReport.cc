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

#include <veins/modules/application/f2md/mdReport/ProtocolReport.h>

ProtocolReport::ProtocolReport(MDReport baseReport) {
    setBaseReport(baseReport);
    bsmListNum = 0;
}

void ProtocolReport::setReportedCheck(BsmCheck reportedCheck) {
    this->reportedCheck = reportedCheck;
}

void ProtocolReport::addBsmToList(BasicSafetyMessage bsm, BsmCheck check) {
    this->bsmList[bsmListNum] = bsm;
    this->checksList[bsmListNum] = check;
    bsmListNum++;
}

void ProtocolReport::addEvidence(BasicSafetyMessage myBsm, bool initial,
        BsmCheck reportedCheck, BasicSafetyMessage receivedBsm,
        NodeTable * detectedNodes, double curTime, double deltaTime,
        int version) {

    bool myBsmAdded = false;

    int histAdd = 0;
    double addedBsmTime = curTime;
    if (initial) {
        setReportedCheck(reportedCheck);
        addBsmToList(receivedBsm, reportedCheck);
        addedBsmTime = receivedBsm.getArrivalTime().dbl();
        histAdd = 1;
    } else {
        setReportedCheck(
                detectedNodes->getMDMHistoryAddr(reportedPseudo)->getBsmCheck(
                        histAdd, version));
    }

    do {
        addBsmToList(
                *detectedNodes->getNodeHistoryAddr(reportedPseudo)->getBSMAddr(
                        histAdd),
                detectedNodes->getMDMHistoryAddr(reportedPseudo)->getBsmCheck(
                        histAdd, version));
        addedBsmTime =
                detectedNodes->getNodeHistoryAddr(reportedPseudo)->getBSMAddr(
                        histAdd)->getArrivalTime().dbl();
        histAdd++;
    } while (((curTime - addedBsmTime) < deltaTime)
            && histAdd
                    < detectedNodes->getNodeHistoryAddr(reportedPseudo)->getBSMNum());

    if (reportedCheck.getRangePlausibility() < 1) {
        addBsmToList(myBsm, BsmCheck());
        myBsmAdded = true;
    }

    for (int var = 0; var < reportedCheck.getIntersection().getInterNum();
            var++) {
        if (reportedCheck.getIntersection().getInterValue(var) < 1) {

            if (senderPseudonym
                    == reportedCheck.getIntersection().getInterId(var)) {
                if (!myBsmAdded) {
                    addBsmToList(myBsm, BsmCheck());
                }
            } else {
                if (detectedNodes->includes(
                        reportedCheck.getIntersection().getInterId(var))) {

                    addBsmToList(
                            *detectedNodes->getNodeHistoryAddr(
                                    reportedCheck.getIntersection().getInterId(
                                            var))->getLatestBSMAddr(),
                            detectedNodes->getMDMHistoryAddr(
                                    reportedCheck.getIntersection().getInterId(
                                            var))->getBsmCheck(0, version));
                }
            }
        }
    }

}

std::string ProtocolReport::getReportPrintableJson() {

    ReportPrintable rp;

    JsonWriter jw;
    jw.writeHeader();
    jw.openJsonElement("Report", false);

    jw.addTagToElement("Report", getBaseReportJson("ProtocolReport"));

    jw.addTagToElement("Report", rp.getCheckJson(reportedCheck));

    jw.openJsonElementList("BSMs");

    for (int var = 0; var < bsmListNum; ++var) {
        if (var < bsmListNum - 1) {
            jw.addTagToElement("BSMs", rp.getBsmJson(bsmList[var]));
        } else {
            jw.addFinalTagToElement("BSMs", rp.getBsmJson(bsmList[var]));
        }
    }
    jw.addTagToElement("Report", jw.getJsonElementList("BSMs"));

    jw.openJsonElementList("BsmChecks");
    for (int var = 0; var < bsmListNum; ++var) {
        if (var < bsmListNum - 1) {
            jw.addTagToElement("BsmChecks",
                    rp.getCheckJsonList(checksList[var]));
        } else {
            jw.addFinalTagToElement("BsmChecks",
                    rp.getCheckJsonList(checksList[var]));
        }
    }

    jw.addFinalTagToElement("Report", jw.getJsonElementList("BsmChecks"));
    jw.addElement(jw.getJsonElement("Report"));
    jw.writeFooter();

    return jw.getOutString();
}
