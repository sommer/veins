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

#include <veins/modules/application/f2md/mdReport/BasicCheckReport.h>

BasicCheckReport::BasicCheckReport(MDReport baseReport) {
    setBaseReport(baseReport);
}
void BasicCheckReport::setReportedCheck(BsmCheck reportedCheck) {
    this->reportedCheck = reportedCheck;
}

std::string BasicCheckReport::getReportPrintableXml() {

    ReportPrintable rp;

    XmlWriter xml;
    xml.init();
    xml.writeHeader();

    std::string tempStr = "Type=\"";
    tempStr = tempStr + "BasicCheckReport";
    tempStr = tempStr + "\"";
    xml.writeOpenTagWithAttribute("Report", tempStr);

    xml.writeWholeElement(getBaseReportXml());
    xml.writeWholeElement(rp.getCheckXml(reportedCheck));

    xml.writeCloseTag();

    return xml.getOutString();
}

std::string BasicCheckReport::getReportPrintableJson() {
    ReportPrintable rp;

    JsonWriter jw;
    jw.writeHeader();
    jw.openJsonElement("Report",false);
    jw.addTagToElement("Report",getBaseReportJson("BasicCheckReport"));
    jw.addFinalTagToElement("Report",rp.getCheckJson(reportedCheck));

    jw.addElement(jw.getJsonElement("Report"));
    jw.writeFooter();

    return jw.getOutString();
}


