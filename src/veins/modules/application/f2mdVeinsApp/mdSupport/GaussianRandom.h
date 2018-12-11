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

#ifndef __VEINS_GaussianRandom_H_
#define __VEINS_GaussianRandom_H_

#include <omnetpp.h>
#include "../BaseWaveApplLayer.h"
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
#include <veins/modules/application/f2mdVeinsApp/mdSupport/GeneralLib.h>
using namespace std;
using namespace boost;

using namespace omnetpp;
using namespace Veins;

class GaussianRandom {
    private:

    Coord *curPosConfidence;
    Coord *curSpeedConfidence;
    Coord *curHeadingConfidence;

    double getGaussianRand(double mean, double stddev);

    public:
        GaussianRandom(Coord *curPosConfidence, Coord *curSpeedConfidence, Coord* curHeadingConfidence);
        Coord OffsetPosition(Coord curPosition);
        Coord OffsetSpeed(Coord curSpeed);
        Coord OffsetHeading(Coord curHeading);

    };

#endif
