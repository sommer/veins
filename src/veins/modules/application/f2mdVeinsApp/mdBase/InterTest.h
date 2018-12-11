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

#ifndef __VEINS_InterTest_H_
#define __VEINS_InterTest_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/BaseWaveApplLayer.h>

using namespace omnetpp;

class InterTest {
    private:
        int interNum;
        int currentNum;
        unsigned long* interPseudonym;
        double* interValue;

    public:
        InterTest();
        InterTest(int interNum);
        int getInterNum();
        void addInterValue(unsigned long pseudo, double value);
        unsigned long getInterId(int index);
        double getInterValue(int index);

        int getIdIndex(unsigned long pseudo);

    };

#endif
