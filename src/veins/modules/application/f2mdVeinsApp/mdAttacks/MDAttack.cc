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

#include <veins/modules/application/f2mdVeinsApp/mdAttacks/MDAttack.h>

MDAttack::MDAttack() {
}

void MDAttack::init(attackTypes::Attacks myAttackType) {
    StopInitiated = false;
    DoSInitiated = false;

    SybilMyOldPseudo = (*myPseudonym);
    SybilVehSeq = 0;

    ConstPosX = genLib.RandomDouble(0, RandomPosX);
    ConstPosY = genLib.RandomDouble(0, RandomPosY);

    ConstPosOffsetX = genLib.RandomDouble(RandomPosOffsetX/5, RandomPosOffsetX);
    ConstPosOffsetY = genLib.RandomDouble(RandomPosOffsetY/5, RandomPosOffsetY);

    ConstSpeedX = genLib.RandomDouble(0, RandomSpeedX);
    ConstSpeedY = genLib.RandomDouble(0, RandomSpeedY);

    ConstSpeedOffsetX = genLib.RandomDouble(RandomSpeedOffsetX/5, RandomSpeedOffsetX);
    ConstSpeedOffsetY = genLib.RandomDouble(RandomSpeedOffsetY/5, RandomSpeedOffsetY);

    if (myAttackType == attackTypes::Sybil) {
        for (int var = 0; var < SybilVehNumber; ++var) {
            SybilPseudonyms[var] = pcPolicy->getNextPseudonym();
        }
    }
}

void MDAttack::setBeaconInterval(simtime_t* beaconInterval) {
    this->beaconInterval = beaconInterval;
}

void MDAttack::setCurHeading(Coord* curHeading) {
    this->curHeading = curHeading;
}

void MDAttack::setCurHeadingConfidence(Coord* curHeadingConfidence) {
    this->curHeadingConfidence = curHeadingConfidence;
}

void MDAttack::setCurPosition(Coord* curPosition) {
    this->curPosition = curPosition;
}

void MDAttack::setCurPositionConfidence(Coord* curPositionConfidence) {
    this->curPositionConfidence = curPositionConfidence;
}

void MDAttack::setCurSpeed(Coord* curSpeed) {
    this->curSpeed = curSpeed;
}

void MDAttack::setCurSpeedConfidence(Coord* curSpeedConfidence) {
    this->curSpeedConfidence = curSpeedConfidence;
}

void MDAttack::setDetectedNodes(NodeTable* detectedNodes) {
    this->detectedNodes = detectedNodes;
}

void MDAttack::setMyBsm(BasicSafetyMessage* myBsm) {
    this->myBsm = myBsm;
}

void MDAttack::setMyBsmNum(int* myBsmNum) {
    this->myBsmNum = myBsmNum;
}

void MDAttack::setMyLength(double* myLength) {
    this->myLength = myLength;
}

void MDAttack::setMyPseudonym(unsigned long * myPseudonym) {
    this->myPseudonym = myPseudonym;
}

void MDAttack::setMyWidth(double* myWidth) {
    this->myWidth = myWidth;
}

void MDAttack::setPcPolicy(PCPolicy* pcPolicy) {
    this->pcPolicy = pcPolicy;
}

BasicSafetyMessage MDAttack::launchAttack(attackTypes::Attacks myAttackType) {

    targetNode = 0;
    BasicSafetyMessage attackBsm = BasicSafetyMessage();

    switch (myAttackType) {
    case attackTypes::StaleMessages: {
        if (*myBsmNum >= StaleMessages_Buffer) {
            attackBsm = myBsm[StaleMessages_Buffer];
            attackBsm.setSenderPseudonym(*myPseudonym);
        } else {
            if (*myBsmNum > 0) {
                attackBsm = myBsm[0];
                attackBsm.setSenderPseudonym(*myPseudonym);
            } else {
                attackBsm.setSenderPseudonym(0);
            }
        }
    }
        break;

    case attackTypes::ConstPos: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        attackBsm.setSenderPos(Coord(ConstPosX, ConstPosY, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(*curSpeed);
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::ConstPosOffset: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        attackBsm.setSenderPos(
                Coord((*curPosition).x + ConstPosOffsetX,
                        (*curPosition).y + ConstPosOffsetY, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(*curSpeed);
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);

    }
        break;

    case attackTypes::RandomPos: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        double randPosX = genLib.RandomDouble(0, RandomPosX);
        double randPosY = genLib.RandomDouble(0, RandomPosY);

        attackBsm.setSenderPos(Coord(randPosX, randPosY, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(*curSpeed);
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::RandomPosOffset: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        double randPosOffsetx = genLib.RandomDouble(0, RandomPosOffsetX)
                - ((double) RandomPosOffsetX) / 2;
        double randPosOffsety = genLib.RandomDouble(0, RandomPosOffsetY)
                - ((double) RandomPosOffsetY) / 2;

        attackBsm.setSenderPos(
                Coord((*curPosition).x + randPosOffsetx,
                        (*curPosition).y + randPosOffsety, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(*curSpeed);
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::ConstSpeed: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        attackBsm.setSenderPos(*curPosition);
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(
                Coord(ConstSpeedX, ConstSpeedY, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::ConstSpeedOffset: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        attackBsm.setSenderPos(*curPosition);
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(
                Coord((*curSpeed).x + ConstSpeedX, (*curSpeed).y + ConstSpeedY,
                        (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);

    }
        break;

    case attackTypes::RandomSpeed: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        double sx = genLib.RandomDouble(0, RandomSpeedX);
        double sy = genLib.RandomDouble(0, RandomSpeedY);

        attackBsm.setSenderPos(*curPosition);
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(Coord(sx, sy, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::RandomSpeedOffset: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        double sx = genLib.RandomDouble(0, RandomSpeedOffsetX)
                - ((double) RandomSpeedOffsetX) / 2;
        double sy = genLib.RandomDouble(0, RandomSpeedOffsetY)
                - ((double) RandomSpeedOffsetY) / 2;

        attackBsm.setSenderPos(*curPosition);
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(
                Coord((*curSpeed).x + sx, (*curSpeed).y + sy, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::EventualStop: {
        if (StopInitiated) {
            attackBsm = StopBsm;
            attackBsm.setSenderPseudonym(*myPseudonym);
        } else {
            double prob = genLib.RandomDouble(0, 1);
            if (prob < StopProb) {
                StopBsm = myBsm[0];
                StopBsm.setSenderPos(*curPosition);
                StopBsm.setSenderPosConfidence(*curPositionConfidence);

                StopBsm.setSenderSpeed(Coord(0, 0, 0));
                StopBsm.setSenderSpeedConfidence(*curSpeedConfidence);

                StopBsm.setSenderHeading(*curHeading);
                StopBsm.setSenderHeadingConfidence(*curHeadingConfidence);

                StopBsm.setSenderWidth(*myWidth);
                StopBsm.setSenderLength(*myLength);
                StopInitiated = true;
            }
            attackBsm.setSenderPseudonym(0);
        }
    }
        break;

    case attackTypes::Disruptive: {
        if (detectedNodes->getNodesNum() > 0) {
            attackBsm = nextAttackBsm;
            attackBsm.setSenderPseudonym(*myPseudonym);
            nextAttackBsm = *detectedNodes->getRandomBSM();
            targetNode = nextAttackBsm.getSenderPseudonym();
        }
    }
        break;

    case attackTypes::DataReplay: {
        if (detectedNodes->getNodesNum() > 0) {
            attackBsm = nextAttackBsm;
            attackBsm.setSenderPseudonym(*myPseudonym);
            nextAttackBsm = *detectedNodes->getNextAttackedBsm(*curPosition,
                    nextAttackBsm.getSenderPseudonym(),
                    nextAttackBsm.getArrivalTime().dbl());
            targetNode = nextAttackBsm.getSenderPseudonym();
        }
    }
        break;

    case attackTypes::DoS: {
        if (!DoSInitiated) {
            beaconInterval->setRaw(beaconInterval->raw() / DosMultipleFreq);
            DoSInitiated = true;
        }

        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        attackBsm.setSenderPos(*curPosition);
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(*curSpeed);
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::DoSRandom: {
        if (!DoSInitiated) {
            beaconInterval->setRaw(beaconInterval->raw() / DosMultipleFreq);
            DoSInitiated = true;
        }
        attackBsm = myBsm[0];

        attackBsm.setSenderPseudonym(*myPseudonym);

        double x = genLib.RandomDouble(0, RandomPosX);
        double y = genLib.RandomDouble(0, RandomPosY);

        double sx = genLib.RandomDouble(0, RandomSpeedX);
        double sy = genLib.RandomDouble(0, RandomSpeedY);

        double hx = genLib.RandomDouble(-1, 1);
        double hy = genLib.RandomDouble(-1, 1);

        attackBsm.setSenderPos(Coord(x, y, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(Coord(sx, sy, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderHeading(Coord(hx, hy, (*curHeading).z));
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::DoSDisruptive: {

        if (!DoSInitiated) {
            beaconInterval->setRaw(beaconInterval->raw() / DosMultipleFreq);
            DoSInitiated = true;
        }

        if (detectedNodes->getNodesNum() > 0) {
            attackBsm = nextAttackBsm;
            attackBsm.setSenderPseudonym(*myPseudonym);
            nextAttackBsm = *detectedNodes->getRandomBSM();
            targetNode = nextAttackBsm.getSenderPseudonym();
        }
    }
        break;

    case attackTypes::Sybil: {

        if (!DoSInitiated) {
            beaconInterval->setRaw(
                    beaconInterval->raw() / (SybilVehNumber + 1));
            DoSInitiated = true;
        }

        if (SybilMyOldPseudo != *myPseudonym) {
            SybilMyOldPseudo = *myPseudonym;
            for (int var = 0; var < SybilVehNumber; ++var) {
                SybilPseudonyms[var] = pcPolicy->getNextPseudonym();
            }
        }

        int SquareX = SybilVehSeq / 2;
        int SquareY = SybilVehSeq % 2;

        if(!SelfSybil){
            if (detectedNodes->getNodesNum() > 0) {
                attackBsm = nextAttackBsm;
                attackBsm.setSenderPseudonym(*myPseudonym);
                nextAttackBsm = *detectedNodes->getNextAttackedBsm(*curPosition,
                        nextAttackBsm.getSenderPseudonym(),
                        nextAttackBsm.getArrivalTime().dbl());
                targetNode = nextAttackBsm.getSenderPseudonym();

                double sybDistXrand = genLib.RandomDouble(-0.5, 0.5);
                double sybDistYrand = genLib.RandomDouble(-0.5, 0.5);

                double XOffset = -SquareX * (attackBsm.getSenderLength() + SybilDistanceX) + sybDistXrand;
                double YOffset = -SquareY * (attackBsm.getSenderWidth() + SybilDistanceY) + sybDistYrand;
                MDMLib mdmLib = MDMLib();
                double curHeadingAngle = mdmLib.calculateHeadingAnglePtr(&attackBsm.getSenderHeading());

                Coord relativePos = Coord(XOffset, YOffset, 0);
                double DeltaAngle = mdmLib.calculateHeadingAnglePtr(&relativePos);

                double newAngle = curHeadingAngle + DeltaAngle;
                newAngle = std::fmod(newAngle, 360);

                newAngle = 360 - newAngle;

                double DOffset = sqrt((pow(XOffset, 2)) + (pow(YOffset, 2)));

                double relativeX = DOffset * cos(newAngle * PI / 180);
                double relativeY = DOffset * sin(newAngle * PI / 180);

                attackBsm.setSenderPos(
                        Coord(attackBsm.getSenderPos().x + relativeX,
                                attackBsm.getSenderPos().y + relativeY, attackBsm.getSenderPos().z));
            }
        }else{

            double sybDistXrand = genLib.RandomDouble(-0.5, +0.5);
            double sybDistYrand = genLib.RandomDouble(-0.5, +0.5);

            double XOffset = -SquareX * (*myLength + SybilDistanceX)+ sybDistXrand;
            double YOffset = -SquareY * (*myWidth + SybilDistanceY) + sybDistYrand;
            MDMLib mdmLib = MDMLib();
            double curHeadingAngle = mdmLib.calculateHeadingAnglePtr(curHeading);

            Coord relativePos = Coord(XOffset, YOffset, 0);
            double DeltaAngle = mdmLib.calculateHeadingAnglePtr(&relativePos);

            double newAngle = curHeadingAngle + DeltaAngle;
            newAngle = std::fmod(newAngle, 360);

            newAngle = 360 - newAngle;

            double DOffset = sqrt((pow(XOffset, 2)) + (pow(YOffset, 2)));

            double relativeX = DOffset * cos(newAngle * PI / 180);
            double relativeY = DOffset * sin(newAngle * PI / 180);

            attackBsm = myBsm[0];

            attackBsm.setSenderPos(
                    Coord((*curPosition).x + relativeX,
                            (*curPosition).y + relativeY, (*curPosition).z));
            attackBsm.setSenderPosConfidence(*curPositionConfidence);

            attackBsm.setSenderSpeed(*curSpeed);
            attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

            attackBsm.setSenderHeading(*curHeading);
            attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

            attackBsm.setSenderWidth(*myWidth);
            attackBsm.setSenderLength(*myLength);
        }

        if (SybilVehSeq > 0) {
            attackBsm.setSenderPseudonym(SybilPseudonyms[SybilVehSeq - 1]);
        } else {
            attackBsm.setSenderPseudonym(*myPseudonym);
        }

        if (SybilVehSeq < SybilVehNumber) {
            SybilVehSeq++;
        } else {
            SybilVehSeq = 0;
        }
    }
        break;

    }

    return attackBsm;
}

unsigned long MDAttack::getTargetNode() {
    return targetNode;
}

