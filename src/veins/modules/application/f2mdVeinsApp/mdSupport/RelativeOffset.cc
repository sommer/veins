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

#include "RelativeOffset.h"
#define EPSILON_G 0.05
#define STEP_G 0.1
MDMLib mdmLib = MDMLib();
GeneralLib genLib = GeneralLib();

RelativeOffset::RelativeOffset(Coord *curPosConfidence,
        Coord *curSpeedConfidence, Coord* curHeadingConfidence,
        double* deltaRPosition, double* deltaThetaPosition, double* deltaSpeed,
        double* deltaHeading) {
    this->curPosConfidence = curPosConfidence;
    this->curSpeedConfidence = curSpeedConfidence;
    this->curHeadingConfidence = curHeadingConfidence;
    this->deltaRPosition = deltaRPosition;
    this->deltaThetaPosition = deltaThetaPosition;
    this->deltaSpeed = deltaSpeed;
    this->deltaHeading = deltaHeading;
}

double RelativeOffset::getGaussianRand(double mean, double stddev) {

    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    random::mt19937 rng(tm.tv_nsec);

    std::normal_distribution<> d { mean, stddev };
    return d(rng);
}

Coord RelativeOffset::OffsetPosition(Coord curPosition) {

    double stepr = fabs(getGaussianRand(0, STEP_G * curPosConfidence->x / 3));

    double gsim = 0.5;
    if (*deltaRPosition > curPosConfidence->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaRPosition, curPosConfidence->x);
    }

    // std::cout<<"gsim:"<<gsim<<"\n";
    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        if (*deltaRPosition > stepr) {
            *deltaRPosition = *deltaRPosition - stepr;
        } else {
            *deltaRPosition = *deltaRPosition + stepr;
        }
    } else {
        *deltaRPosition = *deltaRPosition + stepr;
        if (*deltaRPosition > curPosConfidence->x) {
            *deltaRPosition = *deltaRPosition - stepr;
        }
    }

    double stepTheta = genLib.RandomDouble(-(STEP_G / 2) * PI,
            (STEP_G / 2) * PI);
    *deltaThetaPosition = *deltaThetaPosition + stepTheta;

    double stepX = *deltaRPosition * cos(*deltaThetaPosition);
    double stepY = *deltaRPosition * sin(*deltaThetaPosition);

    //non gaussian
    //r = genLib.RandomDouble(-curPosConfidence, curPosConfidence);

    return Coord(curPosition.x + stepX, curPosition.y + stepY, 0);
}

Coord RelativeOffset::OffsetSpeed(Coord curSpeed) {
    double stepS = fabs(getGaussianRand(0, STEP_G * curSpeedConfidence->x / 3));
    double gsim = 0.5;
    if (*deltaSpeed > curSpeedConfidence->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaSpeed, curSpeedConfidence->x);
    }

    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        if (*deltaSpeed > stepS) {
            *deltaSpeed = *deltaSpeed - stepS;
        } else {
            *deltaSpeed = *deltaSpeed + stepS;
        }
    } else {
        *deltaSpeed = *deltaSpeed + stepS;
        if (*deltaSpeed > curSpeedConfidence->x) {
            *deltaSpeed = *deltaSpeed - stepS;
        }
    }
    return Coord(curSpeed.x + *deltaSpeed, curSpeed.y + *deltaSpeed, 0);

}

Coord RelativeOffset::OffsetHeading(Coord curHeading) {
    double headingAngle = mdmLib.calculateHeadingAngle(curHeading);

    double stepH = fabs(
            getGaussianRand(0, STEP_G * curHeadingConfidence->x / 3));

    double gsim = 0.5;
    if (*deltaHeading > curHeadingConfidence->x / 2) {
        gsim = mdmLib.gaussianSum(*deltaHeading, curHeadingConfidence->x);
    }

    double upDownProb = genLib.RandomDouble(0, 1);
    if (upDownProb < gsim) {
        *deltaHeading = *deltaHeading - stepH;
        if (*deltaHeading < curHeadingConfidence->x / 2) {
            *deltaHeading = *deltaHeading + stepH;
        }
    } else {
        *deltaHeading = *deltaHeading + stepH;
        if (*deltaHeading > curHeadingConfidence->x / 2) {
            *deltaHeading = *deltaHeading - stepH;
        }
    }

    headingAngle = headingAngle + *deltaHeading;
    double x = cos(headingAngle * PI / 180);
    double y = sin(headingAngle * PI / 180);

    return Coord(x, -y, curHeading.z);

}

