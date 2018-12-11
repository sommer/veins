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

#ifndef __VEINS_MDMLib_H_
#define __VEINS_MDMLib_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdMessages/BasicSafetyMessage_m.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/ellipseIntLib/EllipseIntLib.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/rectIntLib/RectIntLib.h>
#include "../BaseWaveApplLayer.h"

using namespace Veins;

class MDMLib {

private:
    void countCircles(double rc, double rl, double rs);
    double calculateCircles(double dl, double ds);

    double importanceFactor(double r1, double r2, double d);

public:
    double gaussianSum(double x, double sig);
    double boundedGaussianSum(double x1, double x2, double sig);

    double calculateDistancePtr(Coord * , Coord * );
    double calculateSpeedPtr(Coord * Speed);
    double calculateHeadingAnglePtr(Coord * heading);

    double calculateDistance(Coord  , Coord );
    double calculateSpeed(Coord  Speed);
    double calculateHeadingAngle(Coord heading);

    double calculatePolynom(long double coof[],const int coofNum, double x);

    double calculateDeltaTime(BasicSafetyMessage * bsm1, BasicSafetyMessage * bsm2);
    double calculateCircleSegment(double radius, double intDistance);
    double calculateCircleCircleIntersection(double r1, double r2, double d);
    double SegmentSegmentFactor(double d, double r1, double r2, double range);

    double CircleSegmentFactor(double d, double r1, double r2, double range);
    double CircleCircleFactor(double d, double r1, double r2, double range);
    double OneSidedCircleSegmentFactor(double d, double r1, double r2,
            double range);
    double CircleIntersectionFactor(double conf1, double conf2, double d,
            double initRadius);

    double RectRectFactor(Coord c1, Coord c2, double heading1, double heading2,
            Coord size1, Coord size2);

    double EllipseEllipseIntersectionFactor(Coord pos1, Coord posConf1, Coord pos2, Coord posConf2, double heading1,
            double heading2, Coord size1, Coord size2);

};

#endif
