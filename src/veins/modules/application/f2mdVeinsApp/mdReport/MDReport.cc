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

#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>

MDReport::MDReport() {
//    generationTime=0;
//    senderId=0;
//    reportedId=0;
//    mbType = 0;
}

void MDReport::setBaseReport(MDReport baseReport) {
    generationTime = baseReport.getGenerationTime();
    senderPseudonym = baseReport.getSenderPseudo();
    reportedPseudo = baseReport.getReportedPseudo();
    mbType = baseReport.getMbType();
    attackType = baseReport.getAttackType();
    senderGps = baseReport.getSenderGps();
    reportedGps = baseReport.getReportedGps();
}

double MDReport::getGenerationTime() {
    return generationTime;
}

unsigned long MDReport::getSenderPseudo() {
    return senderPseudonym;
}
unsigned long MDReport::getReportedPseudo() {
    return reportedPseudo;
}

std::string MDReport::getMbType() {
    return mbType;
}

std::string MDReport::getAttackType() {
    return attackType;
}

void MDReport::setSenderGps(Coord Gps){
    senderGps = Gps;
}

void MDReport::setReportedGps(Coord Gps){
    reportedGps = Gps;
}

Coord MDReport::getSenderGps(){
    return senderGps;
}

Coord MDReport::getReportedGps(){
    return reportedGps;
}

void MDReport::setGenerationTime(double time) {
    generationTime = time;
}

void MDReport::setSenderPseudo(unsigned long pseudo) {
    senderPseudonym = pseudo;
}
void MDReport::setReportedPseudo(unsigned long pseudo) {
    reportedPseudo = pseudo;
}
void MDReport::setMbType(std::string type) {
    mbType = type;
}

void MDReport::setAttackType(std::string type) {
    attackType = type;
}

std::string MDReport::getBaseReportXml() {
    std::string tempStr = "";

    XmlWriter xml;
    xml.init();

    xml.writeOpenTag("Metadata");

    xml.writeStartElementTag("senderId");
    xml.writeString(std::to_string(senderPseudonym));
    xml.writeEndElementTag();

    xml.writeStartElementTag("reportedId");
    xml.writeString(std::to_string(reportedPseudo));
    xml.writeEndElementTag();

    xml.writeStartElementTag("generationTime");
    xml.writeString(std::to_string(generationTime));
    xml.writeEndElementTag();

    xml.writeStartElementTag("mbType");
    xml.writeString(mbType);
    xml.writeEndElementTag();

    xml.writeStartElementTag("attackType");
    xml.writeString(attackType);
    xml.writeEndElementTag();

    xml.writeCloseTag();

    return xml.getOutString();
}

std::string MDReport::getBaseReportJson(std::string reportTypeStr) {

    std::string tempStr ="";

    JsonWriter jw;
    jw.openJsonElement("Metadata", false);

    tempStr = jw.getSimpleTag("senderId",std::to_string(senderPseudonym),true);
    jw.addTagToElement("Metadata", tempStr);
    tempStr = jw.getSimpleTag("reportedId",std::to_string(reportedPseudo),true);
    jw.addTagToElement("Metadata", tempStr);
    tempStr = jw.getSimpleTag("generationTime",std::to_string(generationTime),true);
    jw.addTagToElement("Metadata", tempStr);

    jw.openJsonElementList("senderGps");
    jw.addTagToElement("senderGps", std::to_string(senderGps.x));
    jw.addFinalTagToElement("senderGps", std::to_string(senderGps.y));
    tempStr = jw.getJsonElementList("senderGps");
    jw.addTagToElement("Metadata", tempStr);

    jw.openJsonElementList("reportedGps");
    jw.addTagToElement("reportedGps", std::to_string(reportedGps.x));
    jw.addFinalTagToElement("reportedGps", std::to_string(reportedGps.y));
    tempStr = jw.getJsonElementList("reportedGps");
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("mbType",mbType,false);
    jw.addTagToElement("Metadata", tempStr);
    tempStr = jw.getSimpleTag("attackType",attackType,false);
    jw.addTagToElement("Metadata", tempStr);
    tempStr = jw.getSimpleTag("reportType",reportTypeStr,false);
    jw.addFinalTagToElement("Metadata", tempStr);

    return jw.getJsonElement("Metadata");
}

bool MDReport::writeStrToFile(const std::string strFileCnst,
        const std::string serial, const std::string version,
        const std::string outStr,const std::string curDate) {
    int gentime = generationTime;
    int gentime0000 = (generationTime - gentime) * 10000;

    std::string dirnameStr = strFileCnst + serial + "/MDReports_" + curDate;
    const char* dirnameConst = dirnameStr.c_str();

    struct stat info;
    if (stat(dirnameConst, &info) != 0) {
        mkdir(dirnameConst, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(dirnameConst, 0777);
    }

    std::string strFile = strFileCnst + serial + "/MDReports_" + curDate
            + "/MDReport_" + version + "_" + std::to_string(gentime) + "-"
            + std::to_string(gentime0000) + "_" + std::to_string(senderPseudonym) + "_"
            + std::to_string(reportedPseudo) + ".rep";

    std::fstream checkFile(strFile);

    if (checkFile.is_open()) {

        std::cout << strFile << "\n";
        std::cout << "Error: File alread exists.\n";
        exit(0);
        return false;
    }

    std::ofstream outFile;

    outFile.open(strFile);
    if (outFile.is_open()) {

        outFile << outStr;

        outFile.close();
    }

    return true;

}

bool MDReport::writeStrToFileList(const std::string strFileCnst,
        const std::string serial, const std::string version,
        const std::string outStr,const std::string curDate) {
    std::string dirnameStr = strFileCnst + serial + "/MDReportsList_" + curDate;
    const char* dirnameConst = dirnameStr.c_str();

    struct stat info;
    if (stat(dirnameConst, &info) != 0) {
        mkdir(dirnameConst, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(dirnameConst, 0777);
    }

    std::string strFile = strFileCnst + serial + "/MDReportsList_" + curDate
            + "/MDReport_" + version + "_" + std::to_string(senderPseudonym) + ".lrep";


    std::fstream checkFile(strFile);
    if (checkFile.is_open()) {
        checkFile.seekp(0,ios::end);
        long pos = checkFile.tellp();
        checkFile.seekp (pos-1);
        checkFile << ",";
        checkFile << outStr << "\n";
        checkFile << "\n";
        checkFile << "]";
    }else{
        std::ofstream outFile;
        outFile.open(strFile);
        if (outFile.is_open()) {
            outFile << "[";
            outFile << outStr;
            outFile << "\n";
            outFile << "]";
            outFile.close();
        }
    }

    return true;

}





