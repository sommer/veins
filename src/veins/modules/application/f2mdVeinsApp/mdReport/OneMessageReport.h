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

#ifndef __VEINS_OneMessageReport_H_
#define __VEINS_OneMessageReport_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdMessages/BasicSafetyMessage_m.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/MDReport.h>
#include <veins/modules/application/f2mdVeinsApp/mdReport/ReportPrintable.h>
#include "../BaseWaveApplLayer.h"
#include "../mdSupport/XmlWriter.h"


using namespace omnetpp;

class OneMessageReport: public MDReport {

    private:
        BsmCheck reportedCheck;
        BasicSafetyMessage reportedBsm;

    public:
        OneMessageReport(MDReport baseReport);
        void setReportedCheck(BsmCheck reportedCheck);
        void setReportedBsm(BasicSafetyMessage reportedBsm);
        std::string getReportPrintableXml();
        std::string getReportPrintableJson();

    };

#endif
