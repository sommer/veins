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
#include <veins/modules/application/f2md/mdBase/BsmCheck.h>
#include "../BaseWaveApplLayer.h"

#include "../mdSupport/kalmanLib/Kalman_SVI.h"
#include "../mdSupport/kalmanLib/Kalman_SI.h"
#include "../mdSupport/kalmanLib/Kalman_SC.h"

using namespace omnetpp;
#include <veins/modules/application/f2md/F2MDParameters.h>

class MDMHistory {
private:
    unsigned long nodePseudonym;
    int BSMNumV1;
    int BSMNumV2;
    BsmCheck bsmCheckListV1[MAX_MDM_LENGTH];
    BsmCheck bsmCheckListV2[MAX_MDM_LENGTH];

    Kalman_SVI kalmanSVIV1;
    Kalman_SVI kalmanSVIV2;

    Kalman_SC kalmanSVSIV1;
    Kalman_SC kalmanSVSIV2;

    Kalman_SI kalmanSIV1;
    Kalman_SI kalmanSIV2;

    Kalman_SI kalmanVIV1;
    Kalman_SI kalmanVIV2;

    Kalman_SI kalmanSAIV1;
    Kalman_SI kalmanSAIV2;

    void addBsmCheck(BsmCheck bsmCheckV1, BsmCheck bsmCheckV2);
    void setBsmCheck(int index, BsmCheck bsmCheckV1, BsmCheck bsmCheckV2);

public:
    MDMHistory();
    MDMHistory(unsigned long);
    int getMDMNumV1();
    int getMDMNumV2();

    BsmCheck getBsmCheck(int index, int version);

    void addBsmCheck(BsmCheck bsmCheck, int version);

    void initKalman(BasicSafetyMessage * bsm, int version);

    void setBsmCheck(int index, BsmCheck bsmCheck, int version);


    Kalman_SVI* getKalmanSviv1();

    Kalman_SVI* getKalmanSviv2();

    Kalman_SC* getKalmanSvsiv1();

    Kalman_SC* getKalmanSvsiv2();

    Kalman_SI* getKalmanSaiv1();

    Kalman_SI* getKalmanSaiv2();

    Kalman_SI* getKalmanSiv1();

    Kalman_SI* getKalmanSiv2();

    Kalman_SI* getKalmanViv1();

    Kalman_SI* getKalmanViv2();
};

#endif
