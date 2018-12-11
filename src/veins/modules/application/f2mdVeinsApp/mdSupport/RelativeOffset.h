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

#ifndef __VEINS_RelativeOffset_H_
#define __VEINS_RelativeOffset_H_

#include <omnetpp.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>

#include <ctime>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include "GeneralLib.h"
#include "MDMLib.h"
using namespace std;
using namespace boost;

using namespace omnetpp;
using namespace Veins;

class RelativeOffset {
private:
    Coord* curPosConfidence;
    Coord* curSpeedConfidence;
    Coord* curHeadingConfidence;

    double* deltaRPosition = 0;
    double* deltaThetaPosition = 0;
    double* deltaSpeed = 0;
    double* deltaHeading = 0;

    double getGaussianRand(double mean, double stddev);

public:

    RelativeOffset(Coord* curPosConfidence, Coord *curSpeedConfidence,
            Coord *curHeadingConfidence, double* deltaRPosition,
            double* deltaThetaPosition, double* deltaSpeed,
            double* deltaHeadin);
    Coord OffsetPosition(Coord curPosition);
    Coord OffsetSpeed(Coord curSpeed);
    Coord OffsetHeading(Coord curHeading);

};

#endif
