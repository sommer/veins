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

#ifndef __VEINS_NODEHISTORY_H_
#define __VEINS_NODEHISTORY_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/BaseWaveApplLayer.h>
#include <veins/modules/application/f2mdVeinsApp/F2MDParameters.h>
#include <veins/modules/application/f2mdVeinsApp/mdMessages/BasicSafetyMessage_m.h>

using namespace omnetpp;

class NodeHistory {
    private:
        unsigned long nodePseudonym;
        int bsmNum;
        BasicSafetyMessage bsmList[MAX_BSM_LENGTH];
        BasicSafetyMessage* getBSMList();

    public:
        NodeHistory();
        NodeHistory(unsigned long);
        NodeHistory(unsigned long, BasicSafetyMessage);
        void addBSM(BasicSafetyMessage bsm);

        BasicSafetyMessage* getBSMAddr(int index);
        BasicSafetyMessage* getLatestBSMAddr();

        Coord getSenderPos(int);
        Coord getSenderSize(int);
        double getSenderSpeed(int);
        Coord getSenderHeading(int);
        double getDeltaTime(int, int);
        int getBSMNum();
        double getArrivalTime(int);
    };

#endif
