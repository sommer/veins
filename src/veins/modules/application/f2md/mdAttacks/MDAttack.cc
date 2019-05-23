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

#include <veins/modules/application/f2md/mdAttacks/MDAttack.h>

MDAttack::MDAttack() {
}

void MDAttack::init(attackTypes::Attacks myAttackType) {
    StopInitiated = false;
    DoSInitiated = false;

    SybilMyOldPseudo = (*myPseudonym);
    SybilVehSeq = 0;

    ConstPosX = genLib.RandomDouble(0, RandomPosX);
    ConstPosY = genLib.RandomDouble(0, RandomPosY);

    ConstPosOffsetX = genLib.RandomDouble(0.8 * RandomPosOffsetX,
            1.2 * RandomPosOffsetX);
    ConstPosOffsetY = genLib.RandomDouble(0.8 * RandomPosOffsetY,
            1.2 * RandomPosOffsetY);

    ConstSpeedX = genLib.RandomDouble(0, RandomSpeedX);
    ConstSpeedY = genLib.RandomDouble(0, RandomSpeedY);

    ConstSpeedOffsetX = genLib.RandomDouble(0.8 * RandomSpeedOffsetX,
            1.2 * RandomSpeedOffsetX);
    ConstSpeedOffsetY = genLib.RandomDouble(0.8 * RandomSpeedOffsetY,
            1.2 * RandomSpeedOffsetY);

    if (myAttackType == attackTypes::GridSybil) {
        for (int var = 0; var < SybilVehNumber; ++var) {
            SybilPseudonyms[var] = pcPolicy->getNextPseudonym();
        }
    }

    ReplaySeq = 0;

//    if (myAttackType == attackTypes::DataReplaySybil) {
//        for (int var = 0; var < SybilVehNumber; ++var) {
//            SybilPseudonyms[var] = pcPolicy->getNextPseudonym();
//        }
//    }
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

void MDAttack::setCurAccel(Coord* curAccel) {
    this->curAccel = curAccel;
}

void MDAttack::setCurAccelConfidence(Coord* curAccelConfidence) {
    this->curAccelConfidence = curAccelConfidence;
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

BasicSafetyMessage MDAttack::launchAttack(attackTypes::Attacks myAttackType,
        LinkControl* LinkC) {

    targetNode = 0;
    BasicSafetyMessage attackBsm = BasicSafetyMessage();

    switch (myAttackType) {
    case attackTypes::StaleMessages: {
        if (*myBsmNum >= StaleMessages_Buffer) {
            attackBsm = myBsm[StaleMessages_Buffer];
            attackBsm.setSenderPseudonym(*myPseudonym);
        } else {
            if (*myBsmNum > 0) {
                attackBsm = myBsm[*myBsmNum - 1];
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

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

        attackBsm.setSenderSpeed(Coord(ConstSpeedX, ConstSpeedY, 0));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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
                Coord((*curSpeed).x + ConstSpeedOffsetX,
                        (*curSpeed).y + ConstSpeedOffsetY, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

        attackBsm.setSenderHeading(*curHeading);
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::RandomSpeedOffset: {
        attackBsm = myBsm[0];
        attackBsm.setSenderPseudonym(*myPseudonym);

        double signX = genLib.RandomInt(0, 1);
        double signY = genLib.RandomInt(0, 1);

        double sx = genLib.RandomDouble(0.8 * RandomSpeedOffsetX,
                1.2 * RandomSpeedOffsetX);
        double sy = genLib.RandomDouble(0.8 * RandomSpeedOffsetX,
                1.2 * RandomSpeedOffsetX);

        sx = sx - 2 * signX * sx;
        sy = sy - 2 * signY * sy;

        attackBsm.setSenderPos(*curPosition);
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(
                Coord((*curSpeed).x + sx, (*curSpeed).y + sy, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

                StopBsm.setSenderAccel(Coord(0, 0, 0));
                StopBsm.setSenderAccelConfidence(*curAccelConfidence);

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
            attackBsm = *detectedNodes->getRandomBSM();
            targetNode = attackBsm.getSenderPseudonym();
            attackBsm.setSenderPseudonym(*myPseudonym);
        }
    }
        break;

    case attackTypes::DataReplay: {
        if (detectedNodes->getNodesNum() > 0) {
            if(ReplaySeq< ReplaySeqNum){
                attackBsm = *detectedNodes->getNextAttackedBsm(*curPosition,
                        saveAttackBsm.getSenderPseudonym(),
                        saveAttackBsm.getArrivalTime().dbl());
                ReplaySeq ++;
            }else{
                attackBsm = *detectedNodes->getRandomBSM();
                ReplaySeq = 0;
            }

            saveAttackBsm = attackBsm;
            targetNode = attackBsm.getSenderPseudonym();
            attackBsm.setSenderPseudonym(*myPseudonym);
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

        attackBsm.setSenderAccel(*curAccel);
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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

        double ax = genLib.RandomDouble(0, RandomAccelX);
        double ay = genLib.RandomDouble(0, RandomAccelY);

        double hx = genLib.RandomDouble(-1, 1);
        double hy = genLib.RandomDouble(-1, 1);

        attackBsm.setSenderPos(Coord(x, y, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(Coord(sx, sy, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderAccel(Coord(ax, ay, (*curAccel).z));
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

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
            attackBsm = *detectedNodes->getRandomBSM();
            targetNode = attackBsm.getSenderPseudonym();
            attackBsm.setSenderPseudonym(*myPseudonym);
        }
    }
        break;

    case attackTypes::GridSybil: {

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

        if (!SelfSybil) {
            if (detectedNodes->getNodesNum() > 0) {
                attackBsm = *detectedNodes->getNextAttackedBsm(*curPosition,
                        saveAttackBsm.getSenderPseudonym(),
                        saveAttackBsm.getArrivalTime().dbl());
                saveAttackBsm = attackBsm;
                targetNode = attackBsm.getSenderPseudonym();

                SquareX = (SybilVehSeq + 1) / 2;
                SquareY = (SybilVehSeq + 1) % 2;

                double sybDistXrand = genLib.RandomDouble(
                        -(SybilDistanceX / 10.0), (SybilDistanceX / 10.0));
                double sybDistYrand = genLib.RandomDouble(
                        -(SybilDistanceY / 10.0), (SybilDistanceY / 10.0));

                double XOffset = -SquareX
                        * (attackBsm.getSenderLength() + SybilDistanceX)
                        + sybDistXrand;
                double YOffset = -SquareY
                        * (attackBsm.getSenderWidth() + SybilDistanceY)
                        + sybDistYrand;
                MDMLib mdmLib = MDMLib();
                double curHeadingAngle = mdmLib.calculateHeadingAnglePtr(
                        &attackBsm.getSenderHeading());

                Coord relativePos = Coord(XOffset, YOffset, 0);
                double DeltaAngle = mdmLib.calculateHeadingAnglePtr(
                        &relativePos);

                double newAngle = curHeadingAngle + DeltaAngle;
                newAngle = std::fmod(newAngle, 360);

                newAngle = 360 - newAngle;

                double DOffset = sqrt((pow(XOffset, 2)) + (pow(YOffset, 2)));

                double relativeX = DOffset * cos(newAngle * PI / 180);
                double relativeY = DOffset * sin(newAngle * PI / 180);

                double stearingAngle = std::fmod(
                        (saveHeading - curHeadingAngle), 360);
                if (stearingAngle > 180) {
                    stearingAngle = 360 - stearingAngle;
                }
                if (stearingAngle > 5) {
                    attackBsm.setSenderPseudonym(1);
                }

                saveHeading = curHeadingAngle;

                Coord newSenderPos = Coord(
                        attackBsm.getSenderPos().x + relativeX,
                        attackBsm.getSenderPos().y + relativeY,
                        attackBsm.getSenderPos().z);

                double mapdistance = LinkC->calculateDistance(newSenderPos, 50,
                        50);
                if (mapdistance > MAX_DISTANCE_FROM_ROUTE) {
                    attackBsm.setSenderPseudonym(1);
                }

                attackBsm.setSenderPos(newSenderPos);

                double PosConfDelta = genLib.RandomDouble(
                        -0.1 * attackBsm.getSenderPosConfidence().x,
                        +0.1 * attackBsm.getSenderPosConfidence().x);
                if(PosConfDelta<0){
                    PosConfDelta = 0;
                }
                Coord newPosConf = Coord(
                        attackBsm.getSenderPosConfidence().x + PosConfDelta,
                        attackBsm.getSenderPosConfidence().y + PosConfDelta,
                        attackBsm.getSenderPosConfidence().z);

                attackBsm.setSenderPos(newSenderPos);
                attackBsm.setSenderPosConfidence(newPosConf);

                double randSpeedX = genLib.RandomDouble(
                        -0.05 * attackBsm.getSenderSpeed().x,
                        +0.05 * attackBsm.getSenderSpeed().x);
                double randSpeedY = genLib.RandomDouble(
                        -0.05 * attackBsm.getSenderSpeed().x,
                        +0.05 * attackBsm.getSenderSpeed().x);
                Coord newSpeed = Coord(
                        attackBsm.getSenderSpeed().x + randSpeedX,
                        attackBsm.getSenderSpeed().y + randSpeedY,
                        attackBsm.getSenderSpeed().z);

                double SpeedConfDelta = genLib.RandomDouble(
                        -0.1 * attackBsm.getSenderSpeedConfidence().x,
                        +0.1 * attackBsm.getSenderSpeedConfidence().x);
                if(SpeedConfDelta<0){
                    SpeedConfDelta = 0;
                }
                Coord newSpeedConf = Coord(
                        attackBsm.getSenderSpeedConfidence().x + SpeedConfDelta,
                        attackBsm.getSenderSpeedConfidence().y + SpeedConfDelta,
                        attackBsm.getSenderSpeedConfidence().z);

                attackBsm.setSenderSpeed(newSpeed);
                attackBsm.setSenderSpeedConfidence(newSpeedConf);

                double randAccelx = genLib.RandomDouble(
                        -0.05 * attackBsm.getSenderAccel().x,
                        +0.05 * attackBsm.getSenderAccel().x);
                double randAccely = genLib.RandomDouble(
                        -0.05 * attackBsm.getSenderAccel().x,
                        +0.05 * attackBsm.getSenderAccel().x);
                Coord newAccel = Coord(
                        attackBsm.getSenderAccel().x + randAccelx,
                        attackBsm.getSenderAccel().y + randAccely,
                        attackBsm.getSenderAccel().z);

                double AccelConfDelta = genLib.RandomDouble(
                        -0.1 * attackBsm.getSenderAccelConfidence().x,
                        +0.1 * attackBsm.getSenderAccelConfidence().x);
                if(AccelConfDelta<0){
                    AccelConfDelta = 0;
                }
                Coord newAccelConf = Coord(
                        attackBsm.getSenderAccelConfidence().x + AccelConfDelta,
                        attackBsm.getSenderAccelConfidence().y + AccelConfDelta,
                        attackBsm.getSenderAccelConfidence().z);

                attackBsm.setSenderSpeed(newAccel);
                attackBsm.setSenderSpeedConfidence(newAccelConf);

                double deltaAngle = curHeadingAngle
                        + genLib.RandomDouble(-curHeadingAngle / 360,
                                +curHeadingAngle / 360);
                deltaAngle = std::fmod(deltaAngle, 360);
                deltaAngle = 360 - deltaAngle;
                Coord newHeading = Coord(cos(deltaAngle * PI / 180),
                        sin(deltaAngle * PI / 180), 0);

                double HeadingConfDelta = genLib.RandomDouble(
                        -0.1 * attackBsm.getSenderHeadingConfidence().x,
                        +0.1 * attackBsm.getSenderHeadingConfidence().x);
                if(HeadingConfDelta<0){
                    HeadingConfDelta = 0;
                }
                Coord newHeadingConf = Coord(
                        attackBsm.getSenderHeadingConfidence().x
                                + HeadingConfDelta,
                        attackBsm.getSenderHeadingConfidence().y
                                + HeadingConfDelta,
                        attackBsm.getSenderHeadingConfidence().z);

                attackBsm.setSenderHeading(newHeading);
                attackBsm.setSenderHeadingConfidence(newHeadingConf);
            }
        } else {

            double sybDistXrand = genLib.RandomDouble(-(SybilDistanceX / 10.0),
                    (SybilDistanceX / 10.0));
            double sybDistYrand = genLib.RandomDouble(-(SybilDistanceY / 10.0),
                    (SybilDistanceY / 10.0));

            double XOffset = -SquareX * (*myLength + SybilDistanceX)
                    + sybDistXrand;
            double YOffset = -SquareY * (*myWidth + SybilDistanceY)
                    + sybDistYrand;
            MDMLib mdmLib = MDMLib();
            double curHeadingAngle = mdmLib.calculateHeadingAnglePtr(
                    curHeading);
            Coord relativePos = Coord(XOffset, YOffset, 0);
            double DeltaAngle = mdmLib.calculateHeadingAnglePtr(&relativePos);
            double newAngle = curHeadingAngle + DeltaAngle;

            newAngle = std::fmod(newAngle, 360);

            newAngle = 360 - newAngle;

            double DOffset = sqrt((pow(XOffset, 2)) + (pow(YOffset, 2)));

            double relativeX = DOffset * cos(newAngle * PI / 180);
            double relativeY = DOffset * sin(newAngle * PI / 180);

            attackBsm = myBsm[0];

            Coord newSenderPos = Coord((*curPosition).x + relativeX,
                    (*curPosition).y + relativeY, (*curPosition).z);

            double PosConfDelta = genLib.RandomDouble(
                    -0.1 * curPositionConfidence->x,
                    +0.1 * curPositionConfidence->x);
            if(PosConfDelta<0){
                PosConfDelta = 0;
            }
            Coord newPosConf = Coord(curPositionConfidence->x + PosConfDelta,
                    curPositionConfidence->y + PosConfDelta,
                    curPositionConfidence->z);

            attackBsm.setSenderPos(newSenderPos);
            attackBsm.setSenderPosConfidence(newPosConf);

            double randSpeedX = genLib.RandomDouble(-0.05 * curSpeed->x,
                    +0.05 * curSpeed->x);
            double randSpeedY = genLib.RandomDouble(-0.05 * curSpeed->x,
                    +0.05 * curSpeed->x);
            Coord newSpeed = Coord(curSpeed->x + randSpeedX,
                    curSpeed->y + randSpeedY, curSpeed->z);

            double SpeedConfDelta = genLib.RandomDouble(
                    -0.1 * curSpeedConfidence->x, +0.1 * curSpeedConfidence->x);
            if(SpeedConfDelta<0){
                SpeedConfDelta = 0;
            }
            Coord newSpeedConf = Coord(curSpeedConfidence->x + SpeedConfDelta,
                    curSpeedConfidence->y + SpeedConfDelta,
                    curSpeedConfidence->z);

            attackBsm.setSenderSpeed(newSpeed);
            attackBsm.setSenderSpeedConfidence(newSpeedConf);

            double randAccelx = genLib.RandomDouble(-0.05 * curAccel->x,
                    +0.05 * curAccel->x);
            double randAccely = genLib.RandomDouble(-0.05 * curAccel->x,
                    +0.05 * curAccel->x);
            Coord newAccel = Coord(curAccel->x + randAccelx,
                    curAccel->y + randAccely, curAccel->z);

            double AccelConfDelta = genLib.RandomDouble(
                    -0.1 * curAccelConfidence->x, +0.1 * curAccelConfidence->x);
            if(AccelConfDelta<0){
                AccelConfDelta = 0;
            }
            Coord newAccelConf = Coord(curAccelConfidence->x + AccelConfDelta,
                    curAccelConfidence->y + AccelConfDelta,
                    curAccelConfidence->z);

            attackBsm.setSenderAccel(newAccel);
            attackBsm.setSenderAccelConfidence(newAccelConf);

            double deltaAngle = curHeadingAngle
                    + genLib.RandomDouble(-curHeadingAngle / 360,
                            +curHeadingAngle / 360);
            deltaAngle = std::fmod(deltaAngle, 360);
            deltaAngle = 360 - deltaAngle;
            Coord newHeading = Coord(cos(deltaAngle * PI / 180),
                    sin(deltaAngle * PI / 180), 0);

            double HeadingConfDelta = genLib.RandomDouble(
                    -0.1 * curHeadingConfidence->x,
                    +0.1 * curHeadingConfidence->x);
            if(HeadingConfDelta<0){
                HeadingConfDelta = 0;
            }
            Coord newHeadingConf = Coord(
                    curHeadingConfidence->x + HeadingConfDelta,
                    curHeadingConfidence->y + HeadingConfDelta,
                    curHeadingConfidence->z);

            attackBsm.setSenderHeading(newHeading);
            attackBsm.setSenderHeadingConfidence(newHeadingConf);

            attackBsm.setSenderWidth(*myWidth);
            attackBsm.setSenderLength(*myLength);

            double stearingAngle = std::fmod((saveHeading - curHeadingAngle),
                    360);
            if (stearingAngle > 180) {
                stearingAngle = 360 - stearingAngle;
            }
            if (stearingAngle > 5 && SybilVehSeq > 0) {
                attackBsm.setSenderPseudonym(1);
            }
            saveHeading = curHeadingAngle;

            double mapdistance = LinkC->calculateDistance(newSenderPos, 50, 50);
            if (mapdistance > MAX_DISTANCE_FROM_ROUTE) {
                attackBsm.setSenderPseudonym(1);
            }
//             std::cout<<"SybilVehSeq:"<<SybilVehSeq<<"\n";
//             std::cout<<"SquareX:"<<SquareX<<"\n";
//             std::cout<<"SquareY:"<<SquareY<<"\n";
//             std::cout<<"XOffset:"<<XOffset<<"\n";
//             std::cout<<"YOffset:"<<YOffset<<"\n";

        }

        if (attackBsm.getSenderPseudonym() > 1) {
            if (SybilVehSeq > 0) {
                attackBsm.setSenderPseudonym(SybilPseudonyms[SybilVehSeq - 1]);
            } else {
                attackBsm.setSenderPseudonym(*myPseudonym);
            }
        }

        if (SybilVehSeq < SybilVehNumber) {
            SybilVehSeq++;
        } else {
            SybilVehSeq = 0;
        }
    }
        break;

    case attackTypes::DoSRandomSybil: {
        if (!DoSInitiated) {
            beaconInterval->setRaw(
                    beaconInterval->raw() / DosMultipleFreqSybil);
            DoSInitiated = true;
        }
        attackBsm = myBsm[0];

        attackBsm.setSenderPseudonym(pcPolicy->getNextPseudonym());

        double x = genLib.RandomDouble(0, RandomPosX);
        double y = genLib.RandomDouble(0, RandomPosY);

        double sx = genLib.RandomDouble(0, RandomSpeedX);
        double sy = genLib.RandomDouble(0, RandomSpeedY);

        double ax = genLib.RandomDouble(0, RandomAccelX);
        double ay = genLib.RandomDouble(0, RandomAccelY);

        double hx = genLib.RandomDouble(-1, 1);
        double hy = genLib.RandomDouble(-1, 1);

        attackBsm.setSenderPos(Coord(x, y, (*curPosition).z));
        attackBsm.setSenderPosConfidence(*curPositionConfidence);

        attackBsm.setSenderSpeed(Coord(sx, sy, (*curSpeed).z));
        attackBsm.setSenderSpeedConfidence(*curSpeedConfidence);

        attackBsm.setSenderAccel(Coord(ax, ay, (*curAccel).z));
        attackBsm.setSenderAccelConfidence(*curAccelConfidence);

        attackBsm.setSenderHeading(Coord(hx, hy, (*curHeading).z));
        attackBsm.setSenderHeadingConfidence(*curHeadingConfidence);

        attackBsm.setSenderWidth(*myWidth);
        attackBsm.setSenderLength(*myLength);
    }
        break;

    case attackTypes::DoSDisruptiveSybil: {
        if (!DoSInitiated) {
            beaconInterval->setRaw(
                    beaconInterval->raw() / DosMultipleFreqSybil);
            DoSInitiated = true;
        }

        if (detectedNodes->getNodesNum() > 0) {
            targetNode = attackBsm.getSenderPseudonym();
            attackBsm = *detectedNodes->getRandomBSM();
            attackBsm.setSenderPseudonym(pcPolicy->getNextPseudonym());
        }
    }
        break;

    case attackTypes::DataReplaySybil: {
        if (detectedNodes->getNodesNum() > 0) {
            if(ReplaySeq< ReplaySeqNum){
                attackBsm = *detectedNodes->getNextAttackedBsm(*curPosition,
                        saveAttackBsm.getSenderPseudonym(),
                        saveAttackBsm.getArrivalTime().dbl());
                ReplaySeq ++;
            }else{
                attackBsm = *detectedNodes->getRandomBSM();
                ReplaySeq = 0;
            }

            if (saveAttackBsm.getSenderPseudonym() != attackBsm.getSenderPseudonym()) {
                SybilPseudonyms[0] = pcPolicy->getNextPseudonym();
            }

            saveAttackBsm = attackBsm;
            targetNode = attackBsm.getSenderPseudonym();
            attackBsm.setSenderPseudonym(SybilPseudonyms[0]);
        }
    }
        break;

    }

    return attackBsm;
}

unsigned long MDAttack::getTargetNode() {
    return targetNode;
}


