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

#ifndef __VEINS_MDGlobalAttack_H_
#define __VEINS_MDGlobalAttack_H_

#include <tuple>
#include <omnetpp.h>

#include "../mdBase/NodeTable.h"
#include "../mdPCPolicies/PCPolicy.h"

class MDGlobalAttack {
protected:
    GeneralLib genLib = GeneralLib();
    unsigned long* myPseudonym;
    TraCICommandInterface* traci;

    Coord* curPosition;
    Coord* curPositionConfidence;
    Coord* curSpeed;
    Coord* curSpeedConfidence;
    Coord* curHeading;
    Coord* curHeadingConfidence;

public:

    MDGlobalAttack();

    void init(attackTypes::Attacks myAttackType);

    MDReport launchAttack(attackTypes::Attacks myAttackType, BasicSafetyMessage* reportedBsm);


    void setMyPseudonym(unsigned long * myPseudonym);
    void setTraci(TraCICommandInterface* traci);

    void setCurHeading(Coord* curHeading);
    void setCurHeadingConfidence(Coord* curHeadingConfidence);
    void setCurPosition(Coord* curPosition);
    void setCurPositionConfidence(Coord* curPositionConfidence);
    void setCurSpeed(Coord* curSpeed);
    void setCurSpeedConfidence(Coord* curSpeedConfidence);
};

#endif
