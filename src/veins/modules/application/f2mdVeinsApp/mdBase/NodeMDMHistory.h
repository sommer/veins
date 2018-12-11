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

#ifndef __VEINS_MDMHistory_H_
#define __VEINS_MDMHistory_H_

#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/mdBase/BsmCheck.h>
#include "../BaseWaveApplLayer.h"

using namespace omnetpp;
#include <veins/modules/application/f2mdVeinsApp/F2MDParameters.h>

class MDMHistory {
    private:
    unsigned long nodePseudonym;
        int BSMNumV1;
        int BSMNumV2;
        BsmCheck bsmCheckListV1[MAX_MDM_LENGTH];
        BsmCheck bsmCheckListV2[MAX_MDM_LENGTH];

        void addBsmCheck(BsmCheck bsmCheckV1, BsmCheck bsmCheckV2);
        void setBsmCheck(int index, BsmCheck bsmCheckV1,BsmCheck bsmCheckV2);

    public:
        MDMHistory();
        MDMHistory(unsigned long);
        int getMDMNum();

        BsmCheck getBsmCheck(int index, int version);

        void addBsmCheck(BsmCheck bsmCheck, int version);

        void setBsmCheck(int index, BsmCheck bsmCheck, int version);
    };

#endif
