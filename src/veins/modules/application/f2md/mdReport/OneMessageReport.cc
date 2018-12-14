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

#include <veins/modules/application/f2md/mdReport/OneMessageReport.h>

OneMessageReport::OneMessageReport(MDReport baseReport) {
    setBaseReport(baseReport);
}
void OneMessageReport::setReportedCheck(BsmCheck reportedCheck) {
    this->reportedCheck = reportedCheck;
}

void OneMessageReport::setReportedBsm(BasicSafetyMessage reportedBsm) {
    this->reportedBsm = reportedBsm;
}

std::string OneMessageReport::getReportPrintableXml() {

    ReportPrintable rp;

    XmlWriter xml;
    xml.init();
    xml.writeHeader();

    std::string tempStr = "Type=\"";
    tempStr = tempStr + "OneMessageReport";
    tempStr = tempStr + "\"";
    xml.writeOpenTagWithAttribute("Report", tempStr);

    xml.writeWholeElement(getBaseReportXml());
    xml.writeWholeElement(rp.getCheckXml(reportedCheck));

    xml.writeWholeElement(rp.getBsmXml(reportedBsm));

    xml.writeCloseTag();

    return xml.getOutString();
}

std::string OneMessageReport::getReportPrintableJson() {
    ReportPrintable rp;

    JsonWriter jw;
    jw.writeHeader();
    jw.openJsonElement("Report",false);
    jw.addTagToElement("Report",getBaseReportJson("OneMessageReport"));
    jw.addTagToElement("Report",rp.getCheckJson(reportedCheck));

    jw.openJsonElementList("BSMs");
    jw.addFinalTagToElement("BSMs",rp.getBsmJson(reportedBsm));
    jw.addFinalTagToElement("Report",jw.getJsonElementList("BSMs"));
    jw.addElement(jw.getJsonElement("Report"));
    jw.writeFooter();

    return jw.getOutString();
}


