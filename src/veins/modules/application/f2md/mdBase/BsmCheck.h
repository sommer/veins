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

#ifndef __VEINS_BsmCheck_H_
#define __VEINS_BsmCheck_H_

#include <omnetpp.h>
#include <veins/modules/application/f2md/mdBase/InterTest.h>
#include "../BaseWaveApplLayer.h"

using namespace omnetpp;

class BsmCheck {

    private:
        double rangePlausibility;
        double speedConsistancy;
        double positionConsistancy;
        double positionSpeedConsistancy;
        double positionSpeedMaxConsistancy;
        double speedPlausibility;
        InterTest intersection;
        double suddenAppearence;
        double beaconFrequency;
        double positionPlausibility;
        double positionHeadingConsistancy;

        bool reported;

    public:
        BsmCheck();

        double getRangePlausibility();
        double getPositionPlausibility();
        double getSpeedPlausibility();
        double getPositionConsistancy();
        double getSpeedConsistancy();
        double getPositionSpeedConsistancy();
        double getPositionSpeedMaxConsistancy();
        InterTest getIntersection();
        double getSuddenAppearence();
        double getBeaconFrequency();
        double getPositionHeadingConsistancy();

        bool getReported();

        void setRangePlausibility(double);
        void setPositionPlausibility(double);
        void setSpeedPlausibility(double);
        void setSpeedConsistancy(double);
        void setPositionConsistancy(double);
        void setPositionSpeedConsistancy(double);
        void setPositionSpeedMaxConsistancy(double);
        void setIntersection(InterTest);
        void setSuddenAppearence(double);
        void setBeaconFrequency(double);
        void setPositionHeadingConsistancy(double);

        void setReported(bool);

    };

#endif
