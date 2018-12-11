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

#ifndef __VEINS_PCPolicy_H_
#define __VEINS_PCPolicy_H_

#include <tuple>
#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdEnumTypes/PseudoChangeTypes.h>
#include <veins/modules/application/f2mdVeinsApp/mdStats/MDStatistics.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/MDMLib.h>
#include "../mdBase/NodeTable.h"
#include "../mdBase/InterTest.h"
#include "../mdBase/BsmCheck.h"
#include "../mdBase/InterTest.h"

#include "../mdEnumTypes/MbTypes.h"

using namespace Veins;
using namespace omnetpp;


class PCPolicy {
protected:
    GeneralLib genLib = GeneralLib();
    MDMLib mdmLib = MDMLib();

    Coord* curPosition;
    LAddress::L2Type* myId;
    int* pseudoNum;
    unsigned long* myPseudonym;


    mbTypes::Mbs mbType;
    MDStatistics* mdAuthority;

public:
    unsigned long getNextPseudonym();

public:
    PCPolicy();
    PCPolicy(Coord curPos);
    void setCurPosition(Coord* curPosition);
    void setMyId(LAddress::L2Type* myId);
    void setMyPseudonym(unsigned long* myPseudonym);
    void setPseudoNum(int* pseudoNum);

    void checkPseudonymChange(pseudoChangeTypes::PseudoChange);

    double messageToleranceBuffer = 0;
    void disposablePCP();

    double lastChangeTime = 0;
    void periodicalPCP();

    double cumulativeDistance = 0;
    Coord lastPos = Coord(0, 0, 0);
    void distanceBasedPCP();

    bool firstChange = true;
    bool randDistanceSet = false;
    double randDistance = 800;
    bool randTimeSet = false;
    double randTime = 120;
    double changeTimerStart = 0;
    void car2carPCP();


    void randomPCP();

    void setMbType(mbTypes::Mbs mbType);
    void setMdAuthority(MDStatistics* mdAuthority);

};

#endif
