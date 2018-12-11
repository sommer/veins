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

#include <veins/modules/application/f2mdVeinsApp/mdPCPolicies/PCPolicy.h>

PCPolicy::PCPolicy() {
    messageToleranceBuffer = 0;
    lastChangeTime = simTime().dbl();
    cumulativeDistance = 0;
    lastPos = Coord(0,0,0);
}


PCPolicy::PCPolicy(Coord curPos) {
    messageToleranceBuffer = 0;
    lastChangeTime = simTime().dbl();
    cumulativeDistance = 0;
    lastPos = curPos;

}

void PCPolicy::setMbType(mbTypes::Mbs mbType) {
    this->mbType = mbType;
}

void PCPolicy::setMdAuthority(MDStatistics* mdAuthority) {
    this->mdAuthority = mdAuthority;
}

void PCPolicy::setCurPosition(Coord* curPosition) {
    this->curPosition = curPosition;
}

void PCPolicy::setMyId(LAddress::L2Type* myId) {
    this->myId = myId;
}

void PCPolicy::setMyPseudonym(unsigned long* myPseudonym) {
    this->myPseudonym = myPseudonym;
}

void PCPolicy::setPseudoNum(int* pseudoNum) {
    this->pseudoNum = pseudoNum;
}

unsigned long PCPolicy::getNextPseudonym() {
    (*pseudoNum)++;
    double simTimeDbl = simTime().dbl();
    while (simTimeDbl > 9) {
        simTimeDbl = simTimeDbl / 10;
    }
    simTimeDbl = (int) simTimeDbl;
    unsigned long pseudo = (*myId) * 10 + simTimeDbl;
    unsigned long digitNumber = (unsigned long) (log10(pseudo) + 1);
    unsigned long pseudoNumAdd = (*pseudoNum) * pow(10, digitNumber + 1);
    pseudo = pseudo + pseudoNumAdd;

    mdAuthority->addNewNode(pseudo, mbType, simTime().dbl());

    return pseudo;
}

void PCPolicy::checkPseudonymChange(pseudoChangeTypes::PseudoChange myPcType) {
    switch (myPcType) {
    case pseudoChangeTypes::Periodical:
        periodicalPCP();
        break;
    case pseudoChangeTypes::Disposable:
        disposablePCP();
        break;
    case pseudoChangeTypes::DistanceBased:
        distanceBasedPCP();
        break;
    case pseudoChangeTypes::Random:
        randomPCP();
        break;
    case pseudoChangeTypes::Car2car:
        car2carPCP();
        break;
    default:
        break;
    }
}

void PCPolicy::periodicalPCP() {
    if ((simTime().dbl() - lastChangeTime) > Period_Change_Time) {
        lastChangeTime = simTime().dbl();
        (*myPseudonym) = getNextPseudonym();
    }
}

void PCPolicy::disposablePCP() {
    if (messageToleranceBuffer > Tolerance_Buffer) {
        messageToleranceBuffer = 0;
        (*myPseudonym) = getNextPseudonym();
    } else {
        messageToleranceBuffer++;
    }
}

void PCPolicy::distanceBasedPCP() {
    double stepDistance = mdmLib.calculateDistance(lastPos, (*curPosition));
    lastPos = (*curPosition);

    cumulativeDistance = cumulativeDistance + stepDistance;
    if (cumulativeDistance > Period_Change_Distance) {
        (*myPseudonym) = getNextPseudonym();
        cumulativeDistance = 0;
    }
}

void PCPolicy::car2carPCP() {
    double stepDistance = mdmLib.calculateDistance(lastPos, (*curPosition));
    lastPos = (*curPosition);
    cumulativeDistance = cumulativeDistance + stepDistance;
    if(firstChange){
        if(!randDistanceSet){
           randDistance = genLib.RandomDouble(800, 1500);
           randDistanceSet = true;
        }
        if (cumulativeDistance > randDistance) {
            (*myPseudonym) = getNextPseudonym();
            cumulativeDistance = 0;
            firstChange = false;
        }
    }else{
        if (cumulativeDistance > 800) {
            if(!randTimeSet){
               randTime = genLib.RandomDouble(120, 360);
               changeTimerStart = simTime().dbl();
               randTimeSet = true;
            }
            if ((simTime().dbl() - changeTimerStart) > randTime) {
                (*myPseudonym) = getNextPseudonym();
                cumulativeDistance = 0;
                randTimeSet = false;
            }
        }
    }
}

void PCPolicy::randomPCP() {
    double rnd = genLib.RandomDouble(0, 1);
    if (rnd < Random_Change_Chance) {
        (*myPseudonym) = getNextPseudonym();
    }
}
