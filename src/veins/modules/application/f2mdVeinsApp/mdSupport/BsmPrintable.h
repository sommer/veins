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

#ifndef __VEINS_BsmPrintable_H_
#define __VEINS_BsmPrintable_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdEnumTypes/AttackTypes.h>
#include <veins/modules/application/f2mdVeinsApp/mdMessages/BasicSafetyMessage_m.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/ReportPrintable.h>
#include "../BaseWaveApplLayer.h"
#include "../mdSupport/XmlWriter.h"
#include "../mdSupport/JsonWriter.h"
#include "../mdEnumTypes/MbTypes.h"

using namespace omnetpp;
using namespace Veins;

class BsmPrintable {

private:
    BsmCheck bsmCheck;
    BasicSafetyMessage bsm;
    LAddress::L2Type receiverId;
    unsigned long receiverPseudo;

    std::string getBsmPrintHead();

    std::string getSelfBsmPrintHead();

public:

    BsmPrintable();
    void setBsmCheck(BsmCheck check);
    void setBsm(BasicSafetyMessage bsm);
    void setReceiverId(LAddress::L2Type receiverId);
    void setReceiverPseudo(unsigned long receiverPseudo);


    std::string getBsmPrintableXml();
    std::string getBsmPrintableJson();

    bool writeStrToFile(const std::string strFileCnst, const std::string serial,
            const std::string version, const std::string outStr,
            const std::string curDate);


    std::string getSelfBsmPrintableJson();

    bool writeSelfStrToFile(const std::string strFileCnst,
            const std::string serial, const std::string outStr,
            const std::string curDate);

    bool writeStrToFileList(const std::string strFileCnst,
            const std::string serial, const std::string version,
            const std::string outStr, const std::string curDate);

};

#endif
