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

#ifndef __VEINS_MDReport_H_
#define __VEINS_MDReport_H_

#include <omnetpp.h>
#include <time.h>
#include "../BaseWaveApplLayer.h"

#include "../mdBase/BsmCheck.h"
#include <sys/stat.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/JsonWriter.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/XmlWriter.h>

using namespace omnetpp;


class MDReport {

protected:
    double generationTime;
    unsigned long senderPseudonym;
    unsigned long reportedPseudo;
    std::string mbType;
    std::string attackType;

    Coord senderGps;
    Coord reportedGps;


public:
    MDReport();

    void setBaseReport(MDReport baseReport);

    void setGenerationTime(double time);
    void setSenderPseudo(unsigned long pseudo);
    void setReportedPseudo(unsigned long pseudo);
    void setMbType(std::string type);
    void setAttackType(std::string type);

    void setSenderGps(Coord senderGps);
    void setReportedGps(Coord reportedGps);

    double getGenerationTime();
    unsigned long getSenderPseudo();
    unsigned long getReportedPseudo();
    std::string getMbType();
    std::string getAttackType();

    Coord getSenderGps();
    Coord getReportedGps();

    std::string getBaseReportXml();
    std::string getBaseReportJson(std::string reportType);
    bool writeStrToFile(const std::string strFileCnst, const std::string serial,
            const std::string version, const std::string outStr,const std::string curDate);
    bool writeStrToFileList(const std::string strFileCnst, const std::string serial,
            const std::string version, const std::string outStr,const std::string curDate);

};

#endif
