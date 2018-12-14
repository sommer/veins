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

#include <veins/modules/application/f2md/mdStats/MDStatistics.h>

MDStatistics::MDStatistics() {
    char nameV1[32] = "mdaV1";
    char nameV2[32] = "mdaV2";
    registerNewBase(nameV1);
    registerNewBase(nameV2);
}

void MDStatistics::registerNewBase(char* baseName) {
    baseList[baseListNum].setName(baseName);
    baseListNum++;
}

void MDStatistics::addNewNode(unsigned long pseudo, mbTypes::Mbs mbType,
        double time) {
    switch (mbType) {
    case mbTypes::Genuine: {
        for (int var = 0; var < baseListNum; ++var) {
            baseList[var].addTotalGenuine(pseudo, time);
        }
    }
    break;
    case mbTypes::GlobalAttacker: {
        for (int var = 0; var < baseListNum; ++var) {
            baseList[var].addTotalGenuine(pseudo, time);
        }
    }
        break;
    case mbTypes::LocalAttacker: {
        for (int var = 0; var < baseListNum; ++var) {
            baseList[var].addTotalAttacker(pseudo, time);
        }
    }
        break;
    }
}

void MDStatistics::addReportedNode(unsigned long pseudo, mbTypes::Mbs mbType,
        double time) {

    switch (mbType) {
    case mbTypes::Genuine: {
        for (int var = 0; var < baseListNum; ++var) {
            baseList[var].addReportedGenuine(pseudo, time);
        }
    }
    break;
    case mbTypes::GlobalAttacker: {
        for (int var = 0; var < baseListNum; ++var) {
            baseList[var].addReportedGenuine(pseudo, time);
        }
    }
        break;
    case mbTypes::LocalAttacker: {
        for (int var = 0; var < baseListNum; ++var) {
            baseList[var].addReportedAttacker(pseudo, time);
        }
    }
        break;
    }

}

void MDStatistics::getReport(const char* baseName, MDReport report) {
    int index = -1;
    for (int var = 0; var < baseListNum; ++var) {
        if (strcmp(baseList[var].getName(), baseName) == 0) {
            index = var;
            break;
        }
    }
    if (index != -1) {
        treatReport(baseList, index, report);
    } else {
        baseList[baseListNum].setName(baseName);
        treatReport(baseList, baseListNum, report);
        baseListNum++;
    }
}

void MDStatistics::treatReport(MDSBase *base, int index, MDReport report) {
    if (!report.getMbType().compare("Genuine")) {
        if (!base[index].alreadyReportedGenuine(report.getReportedPseudo())) {
            base[index].addReportedGenuine(report.getReportedPseudo(),
                    report.getGenerationTime());
        }
    }

    if (!report.getMbType().compare("LocalAttacker")) {
        if (!base[index].alreadyReportedAttacker(report.getReportedPseudo())) {
            base[index].addReportedAttacker(report.getReportedPseudo(),
                    report.getGenerationTime());
        }
    }

}

void MDStatistics::saveLine(std::string path, std::string serial, double time, bool printOut) {

    char outChar[1024];
    char directoryPathGen[1024] = "";
    char filePathGen[1024] = "";
    const char * pathChar = path.c_str();
    const char * serialChar = serial.c_str();
    strcat(directoryPathGen, pathChar);
    strcat(directoryPathGen, serialChar);

    struct stat info;

    if (stat(directoryPathGen, &info) != 0) {
        mkdir(directoryPathGen, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(directoryPathGen, 0777);
    }

    for (int var = 0; var < baseListNum; ++var) {
        char fileName[64];
        strcpy(fileName, baseList[var].getName());
        strcat(fileName, ".dat");

        strcpy(filePathGen, directoryPathGen);
        strcat(filePathGen, "/");
        strcat(filePathGen, fileName);

        baseList[var].getPrintable(outChar, time, printOut);
        baseList[var].writeFile(filePathGen, outChar);
    }

}

void MDStatistics::resetAll() {
    for (int var = 0; var < baseListNum; ++var) {
        baseList[var].resetAll();
    }
}
