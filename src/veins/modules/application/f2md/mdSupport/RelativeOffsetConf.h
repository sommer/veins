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

#ifndef __VEINS_RelativeOffsetConf_H_
#define __VEINS_RelativeOffsetConf_H_

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
#include <veins/modules/application/f2md/mdSupport/GeneralLib.h>
#include <veins/modules/application/f2md/mdSupport/MDMLib.h>
using namespace std;
using namespace boost;

using namespace omnetpp;
using namespace veins;

class RelativeOffsetConf {
private:
    Coord* ConfPosMax = 0;
    Coord* ConfSpeedMax = 0;
    Coord* ConfHeadingMax = 0;
    Coord* ConfAccelMax = 0;

    double* deltaConfPos = 0;
    double* deltaConfSpeed = 0;
    double* deltaConfHeading = 0;
    double* deltaConfAccel = 0;

    double getGaussianRand(double mean, double stddev);

public:

    RelativeOffsetConf(Coord* ConfPosMax, Coord* ConfSpeedMax,
            Coord *ConfHeadingMax,Coord*ConfAccelMax, double* deltaConfPos,
            double* deltaConfSpeed, double *deltaConfHeading,
            double *deltaConfAccel);

    Coord OffsetPosConf(Coord curPosConf);
    Coord OffsetSpeedConf(Coord curSpeedConf);
    Coord OffsetAccelConf(Coord curAccelConf);
    Coord OffsetHeadingConf(Coord curHeadingConf);

};

#endif
