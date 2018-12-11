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

#ifndef __VEINS_MDABase_H_
#define __VEINS_MDABase_H_

#include <string.h>
#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdStats/MDSBase.h>
#include "../BaseWaveApplLayer.h"
#include "../mdReport/MDReport.h"


using namespace omnetpp;

#define TOTCARNUM 100

class MDSBase {
private:

    char name[32];

    bool init = true;

    unsigned long totalGenuineIdList[TOTCARNUM];
    double totalGenuineTimeList[TOTCARNUM];
    int totalGenuineNum = 0;

    unsigned long totalAttackerIdList[TOTCARNUM];
    double totalAttackerTimeList[TOTCARNUM];
    int totalAttackerNum = 0;

    unsigned long reportedAttackerIdList[TOTCARNUM];
    double reportedAttackerTimeList[TOTCARNUM];
    int reportedAttackerNum = 0;

    unsigned long reportedGenuineIdList[TOTCARNUM];
    double reportedGenuineTimeList[TOTCARNUM];
    int reportedGenuineNum = 0;


    unsigned long CountTotalGenuine = 0;
    unsigned long CountTotalAttacker = 0;
    unsigned long CountReportedGenuine = 0;
    unsigned long CountReportedAttacker = 0;


    double genuineAverageReportDelay = 0;
    double attackerAverageReportDelay = 0;

public:
    MDSBase();
    MDSBase(const char *);
    void addTotalGenuine(unsigned long id, double time);
    void addReportedGenuine(unsigned long id, double time);

    void addTotalAttacker(unsigned long id, double time);
    void addReportedAttacker(unsigned long id, double time);

    void getTotalGenuine(unsigned long id);
    void getReportedGenuineIdList(unsigned long id);

    void getTotalAttackerIdList(unsigned long id);
    void getReportedAttackerIdList(unsigned long id);

    bool alreadyReportedGenuine(unsigned long id);
    bool alreadyReportedAttacker(unsigned long id);

    int totalGenuineIndex(unsigned long id);
    int totalAttackerIndex(unsigned long id);

    void getPrintable(char* outStr, double time, bool printOut);
    void writeFile(std::string path, char* printStr);

    char* getName();

    void setName(const char*);

    void resetAll();

    void removeOldestAttacker();
    void removeOldestGenuine();

//    int oldestReportedGenuine();
    int oldestTotalGenuine();

//    int oldestReportedAttacker();
    int oldestTotalAttacker();

    int reportedGenuineIndex(unsigned long pseudo);
    int reportedAttackerIndex(unsigned long pseudo);

    void removeTotalGenuine(int index);
    void removeTotalAttacker(int index);
    void removeReportedGenuine(int index);
    void removeReportedAttacker(int index);



};

#endif
