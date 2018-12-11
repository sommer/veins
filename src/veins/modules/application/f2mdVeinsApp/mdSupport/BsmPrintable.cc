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

#include <veins/modules/application/f2mdVeinsApp/mdSupport/BsmPrintable.h>

BsmPrintable::BsmPrintable() {

}
void BsmPrintable::setBsmCheck(BsmCheck Check) {
    this->bsmCheck = Check;
}

void BsmPrintable::setBsm(BasicSafetyMessage bsm) {
    this->bsm = bsm;
}

void BsmPrintable::setReceiverId(LAddress::L2Type receiverId) {
    this->receiverId = receiverId;
}

void BsmPrintable::setReceiverPseudo(unsigned long receiverPseudo) {
    this->receiverPseudo = receiverPseudo;
}

std::string BsmPrintable::getBsmPrintableXml() {

    ReportPrintable rp;

    XmlWriter xml;
    xml.init();
    xml.writeHeader();

    std::string tempStr = "receiverId=\"";
    tempStr = tempStr + std::to_string(receiverId);
    tempStr = tempStr + " receiverPseudo=\"";
    tempStr = tempStr + std::to_string(receiverPseudo);
    tempStr = tempStr + " mbType=\"";
    tempStr = tempStr + mbTypes::mbNames[bsm.getSenderMbType()];
    tempStr = tempStr + " attackType=\"";
    tempStr = tempStr + attackTypes::AttackNames[bsm.getSenderAttackType()];
    tempStr = tempStr + "\"";
    xml.writeOpenTagWithAttribute("BsmPrint", tempStr);
    xml.writeWholeElement(rp.getCheckXml(bsmCheck));
    xml.writeWholeElement(rp.getBsmXml(bsm));

    xml.writeCloseTag();

    return xml.getOutString();
}

std::string BsmPrintable::getBsmPrintableJson() {
    ReportPrintable rp;

    JsonWriter jw;
    jw.writeHeader();
    jw.openJsonElement("BsmPrint", false);
    jw.addTagToElement("BsmPrint", getBsmPrintHead());
    jw.addTagToElement("BsmPrint", rp.getCheckJson(bsmCheck));
    jw.openJsonElementList("BSMs");
    jw.addFinalTagToElement("BSMs",rp.getBsmJson(bsm));
    jw.addFinalTagToElement("BsmPrint",jw.getJsonElementList("BSMs"));
    jw.addElement(jw.getJsonElement("BsmPrint"));
    jw.writeFooter();



    return jw.getOutString();
}

std::string BsmPrintable::getSelfBsmPrintableJson() {
    ReportPrintable rp;

    JsonWriter jw;
    jw.writeHeader();
    jw.openJsonElement("BsmPrint", false);
    jw.addTagToElement("BsmPrint", getBsmPrintHead());
    jw.openJsonElementList("BSMs");
    jw.addFinalTagToElement("BSMs",rp.getBsmJson(bsm));
    jw.addFinalTagToElement("BsmPrint",jw.getJsonElementList("BSMs"));
    jw.addElement(jw.getJsonElement("BsmPrint"));
    jw.writeFooter();

    return jw.getOutString();
}

std::string BsmPrintable::getSelfBsmPrintHead() {

    std::string tempStr = "";
    JsonWriter jw;

    jw.openJsonElement("Metadata", false);

    tempStr = jw.getSimpleTag("receiverId", std::to_string(receiverId),
            true);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("receiverPseudo", std::to_string(receiverPseudo),
            true);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("mbType", mbTypes::mbNames[bsm.getSenderMbType()], false);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("attackType", attackTypes::AttackNames[bsm.getSenderAttackType()], false);
    jw.addFinalTagToElement("Metadata", tempStr);

    return jw.getJsonElement("Metadata");

}

std::string BsmPrintable::getBsmPrintHead() {

    std::string tempStr = "";
    JsonWriter jw;

    jw.openJsonElement("Metadata", false);

    tempStr = jw.getSimpleTag("receiverId", std::to_string(receiverId),
            true);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("receiverPseudo", std::to_string(receiverPseudo),
            true);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("generationTime", std::to_string(simTime().dbl()),
            true);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("mbType", mbTypes::mbNames[bsm.getSenderMbType()], false);
    jw.addTagToElement("Metadata", tempStr);

    tempStr = jw.getSimpleTag("attackType", attackTypes::AttackNames[bsm.getSenderAttackType()], false);
    jw.addFinalTagToElement("Metadata", tempStr);

    return jw.getJsonElement("Metadata");

}

bool BsmPrintable::writeSelfStrToFile(const std::string strFileCnst,
        const std::string serial,
        const std::string outStr, const std::string curDate) {

    int gentime = bsm.getArrivalTime().dbl();
    int gentime0000 = (bsm.getArrivalTime().dbl() - gentime) * 10000;

    std::string upperDirName = strFileCnst + serial;
    const char* upperDirNameConst = upperDirName.c_str();
    struct stat info;
    if (stat(upperDirNameConst, &info) != 0) {
        mkdir(upperDirNameConst, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(upperDirNameConst, 0777);
    }

    std::string dirnameStr = strFileCnst + serial + "/SelfBsms_" + curDate;
    const char* dirnameConst = dirnameStr.c_str();

    struct stat info2;
    if (stat(dirnameConst, &info2) != 0) {
        mkdir(dirnameConst, 0777);
    } else if (info2.st_mode & S_IFDIR) {
    } else {
        mkdir(dirnameConst, 0777);
    }

    std::string strFile = strFileCnst + serial + "/SelfBsms_" + curDate
            + "/MDBsm_" + std::to_string(gentime) + "-"
            + std::to_string(gentime0000) + "_" + std::to_string(receiverPseudo)+ ".bsm";

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

bool BsmPrintable::writeStrToFile(const std::string strFileCnst,
        const std::string serial, const std::string version,
        const std::string outStr, const std::string curDate) {

    int gentime = bsm.getArrivalTime().dbl();
    int gentime0000 = (bsm.getArrivalTime().dbl() - gentime) * 10000;

    std::string upperDirName = strFileCnst + serial;
    const char* upperDirNameConst = upperDirName.c_str();
    struct stat info;
    if (stat(upperDirNameConst, &info) != 0) {
        mkdir(upperDirNameConst, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(upperDirNameConst, 0777);
    }

    std::string dirnameStr = strFileCnst + serial + "/MDBsms_" + curDate;
    const char* dirnameConst = dirnameStr.c_str();

    struct stat info2;
    if (stat(dirnameConst, &info2) != 0) {
        mkdir(dirnameConst, 0777);
    } else if (info2.st_mode & S_IFDIR) {
    } else {
        mkdir(dirnameConst, 0777);
    }

    std::string strFile = strFileCnst + serial + "/MDBsms_" + curDate
            + "/MDBsm_" + version + "_" + std::to_string(gentime) + "-"
            + std::to_string(gentime0000) + "_" + std::to_string(receiverPseudo)
            + "_" + std::to_string(bsm.getSenderPseudonym()) + ".bsm";

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


bool BsmPrintable::writeStrToFileList(const std::string strFileCnst,
        const std::string serial, const std::string version,
        const std::string outStr, const std::string curDate) {

    std::string upperDirName = strFileCnst + serial;
    const char* upperDirNameConst = upperDirName.c_str();
    struct stat info;
    if (stat(upperDirNameConst, &info) != 0) {
        mkdir(upperDirNameConst, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(upperDirNameConst, 0777);
    }

    std::string dirnameStr = strFileCnst + serial + "/MDBsmsList_" + curDate;
    const char* dirnameConst = dirnameStr.c_str();

    struct stat info2;
    if (stat(dirnameConst, &info2) != 0) {
        mkdir(dirnameConst, 0777);
    } else if (info2.st_mode & S_IFDIR) {
    } else {
        mkdir(dirnameConst, 0777);
    }

    std::string strFile = strFileCnst + serial + "/MDBsmsList_" + curDate
            + "/MDBsm_" + version + "_" + std::to_string(receiverPseudo)
            + ".lbsm";

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
