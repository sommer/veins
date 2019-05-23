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

#include <veins/modules/application/f2md/mdSupport/RelativeOffsetConf.h>
#define EPSILON_G 0.05
#define STEP_G 0.05


RelativeOffsetConf::RelativeOffsetConf(Coord* ConfPosMax, Coord* ConfSpeedMax,
        Coord *ConfHeadingMax,Coord*ConfAccelMax, double* deltaConfPos,
        double* deltaConfSpeed, double *deltaConfHeading,
        double *deltaConfAccel) {
    this->ConfPosMax = ConfPosMax;
    this->ConfSpeedMax = ConfSpeedMax;
    this->ConfHeadingMax = ConfHeadingMax;
    this->ConfAccelMax = ConfAccelMax;
    this->deltaConfPos = deltaConfPos;
    this->deltaConfSpeed = deltaConfSpeed;
    this->deltaConfHeading = deltaConfHeading;
    this->deltaConfAccel = deltaConfAccel;
}

double RelativeOffsetConf::getGaussianRand(double mean, double stddev) {

    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    random::mt19937 rng(tm.tv_nsec);

    std::normal_distribution<> d { mean, stddev };
    return d(rng);
}

Coord RelativeOffsetConf::OffsetPosConf(Coord curPosConf) {
    MDMLib mdmLib = MDMLib();
    GeneralLib genLib = GeneralLib();
    double stepS = fabs(getGaussianRand(0, STEP_G * ConfPosMax->x / 3));
    double gsim = 0.5;
    if (*deltaConfPos > ConfPosMax->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaConfPos, ConfPosMax->x);
    }

    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        if (*deltaConfPos > stepS) {
            *deltaConfPos = *deltaConfPos - stepS;
        } else {
            *deltaConfPos = *deltaConfPos + stepS;
        }
    } else {
        *deltaConfPos = *deltaConfPos + stepS;
        if (*deltaConfPos > ConfPosMax->x) {
            *deltaConfPos = *deltaConfPos - stepS;
        }
    }
    double deltaConfPosR = 1.0;
    if(*deltaConfPos<0){
        deltaConfPosR = -1.0;
    }
    deltaConfPosR = deltaConfPosR*((int)(*deltaConfPos * 100 + .5) / 100.0);
    return Coord(curPosConf.x + deltaConfPosR, curPosConf.y + deltaConfPosR, 0);
}

Coord RelativeOffsetConf::OffsetSpeedConf(Coord curSpeedConf) {
    MDMLib mdmLib = MDMLib();
    GeneralLib genLib = GeneralLib();
    double stepS = fabs(getGaussianRand(0, STEP_G * ConfSpeedMax->x / 3));
    double gsim = 0.5;
    if (*deltaConfSpeed > ConfSpeedMax->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaConfSpeed, ConfSpeedMax->x);
    }

    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        if (*deltaConfSpeed > stepS) {
            *deltaConfSpeed = *deltaConfSpeed - stepS;
        } else {
            *deltaConfSpeed = *deltaConfSpeed + stepS;
        }
    } else {
        *deltaConfSpeed = *deltaConfSpeed + stepS;
        if (*deltaConfSpeed > ConfSpeedMax->x) {
            *deltaConfSpeed = *deltaConfSpeed - stepS;
        }
    }
    double deltaConfSpeedR = 1.0;
    if(*deltaConfSpeed<0){
        deltaConfSpeedR = -1.0;
    }
    deltaConfSpeedR = deltaConfSpeedR * ((int)(*deltaConfSpeed * 100 + .5) / 100.0);
    return Coord(curSpeedConf.x + deltaConfSpeedR, curSpeedConf.y + deltaConfSpeedR, 0);
}

Coord RelativeOffsetConf::OffsetAccelConf(Coord curAccelConf) {
    MDMLib mdmLib = MDMLib();
    GeneralLib genLib = GeneralLib();
    double stepS = fabs(getGaussianRand(0, STEP_G * ConfAccelMax->x / 3));
    double gsim = 0.5;
    if (*deltaConfAccel > ConfAccelMax->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaConfAccel, ConfAccelMax->x);
    }

    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        if (*deltaConfAccel > stepS) {
            *deltaConfAccel = *deltaConfAccel - stepS;
        } else {
            *deltaConfAccel = *deltaConfAccel + stepS;
        }
    } else {
        *deltaConfAccel = *deltaConfAccel + stepS;
        if (*deltaConfAccel > ConfAccelMax->x) {
            *deltaConfAccel = *deltaConfAccel - stepS;
        }
    }

    double deltaConfAccelR = 1.0;
    if(*deltaConfAccel<0){
        deltaConfAccelR = -1.0;
    }

    deltaConfAccelR = deltaConfAccelR* ((int)(*deltaConfAccel * 100 + .5) / 100.0);
    return Coord(curAccelConf.x + deltaConfAccelR, curAccelConf.y + deltaConfAccelR, 0);
}

Coord RelativeOffsetConf::OffsetHeadingConf(Coord curHeadingConf) {
    MDMLib mdmLib = MDMLib();
    GeneralLib genLib = GeneralLib();
    double stepS = fabs(getGaussianRand(0, STEP_G * ConfHeadingMax->x / 3));
    double gsim = 0.5;
    if (*deltaConfHeading > ConfHeadingMax->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaConfHeading, ConfHeadingMax->x);
    }

    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        if (*deltaConfHeading > stepS) {
            *deltaConfHeading = *deltaConfHeading - stepS;
        } else {
            *deltaConfHeading = *deltaConfHeading + stepS;
        }
    } else {
        *deltaConfHeading = *deltaConfHeading + stepS;
        if (*deltaConfHeading > ConfHeadingMax->x) {
            *deltaConfHeading = *deltaConfHeading - stepS;
        }
    }
    double deltaConfHeadingR = 1.0;
    if(*deltaConfHeading<0){
        deltaConfHeadingR = -1.0;
    }

    deltaConfHeadingR = deltaConfHeadingR * ((int)(*deltaConfHeading * 100 + .5) / 100.0);
    return Coord(curHeadingConf.x + deltaConfHeadingR, curHeadingConf.y + deltaConfHeadingR, 0);
}
