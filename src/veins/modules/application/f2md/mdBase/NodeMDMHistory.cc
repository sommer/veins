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

#include <veins/modules/application/f2md/mdBase/NodeMDMHistory.h>

MDMHistory::MDMHistory() {
    nodePseudonym = 0;
    BSMNumV1 = 0;
    BSMNumV2 = 0;
}

int MDMHistory::getMDMNumV1() {
    return BSMNumV1;
}

int MDMHistory::getMDMNumV2() {
    return BSMNumV2;
}

MDMHistory::MDMHistory(unsigned long pseudo) {
    nodePseudonym = pseudo;
    BSMNumV1 = 0;
    BSMNumV2 = 0;
}

BsmCheck MDMHistory::getBsmCheck(int index, int version) {
    if (version == 1) {
        return bsmCheckListV1[index];
    } else {
        return bsmCheckListV2[index];
    }
}

void MDMHistory::setBsmCheck(int index, BsmCheck bsmCheckV1,
        BsmCheck bsmCheckV2) {
    setBsmCheck(index, bsmCheckV1, 1);
    setBsmCheck(index, bsmCheckV2, 2);
}

void MDMHistory::setBsmCheck(int index, BsmCheck bsmCheck, int version) {
    switch (version) {
    case 1: {
        bsmCheckListV1[index] = bsmCheck;
        break;
    }
    case 2: {
        bsmCheckListV2[index] = bsmCheck;
        break;
    }
    }
}

void MDMHistory::addBsmCheck(BsmCheck bsmCheckV1, BsmCheck bsmCheckV2) {
    addBsmCheck(bsmCheckV1, 1);
    addBsmCheck(bsmCheckV2, 2);
}

void MDMHistory::initKalman(BasicSafetyMessage * bsm, int version) {
    switch (version) {
    case 1: {
        kalmanSVIV1.setInitial(bsm->getSenderPos().x, bsm->getSenderPos().y,
                bsm->getSenderSpeed().x, bsm->getSenderSpeed().y);

        double speed = sqrt(
                pow(bsm->getSenderSpeed().x, 2.0)
                        + pow(bsm->getSenderSpeed().y, 2.0)
                        + pow(bsm->getSenderSpeed().z, 2.0));

        kalmanSVSIV1.setInitial(0, speed);
        kalmanSIV1.setInitial(bsm->getSenderPos().x, bsm->getSenderPos().y);
        kalmanSAIV1.setInitial(bsm->getSenderPos().x, bsm->getSenderPos().y);
        kalmanVIV1.setInitial(bsm->getSenderSpeed().x, bsm->getSenderSpeed().y);
        break;
    }
    case 2: {
        kalmanSVIV2.setInitial(bsm->getSenderPos().x, bsm->getSenderPos().y,
                bsm->getSenderSpeed().x, bsm->getSenderSpeed().y);

        double speed = sqrt(
                pow(bsm->getSenderSpeed().x, 2.0)
                        + pow(bsm->getSenderSpeed().y, 2.0)
                        + pow(bsm->getSenderSpeed().z, 2.0));
        kalmanSVSIV2.setInitial(0, speed);
        kalmanSIV2.setInitial(bsm->getSenderPos().x, bsm->getSenderPos().y);
        kalmanSAIV2.setInitial(bsm->getSenderPos().x, bsm->getSenderPos().y);
        kalmanVIV2.setInitial(bsm->getSenderSpeed().x, bsm->getSenderSpeed().y);
        break;
    }
    }
}

void MDMHistory::addBsmCheck(BsmCheck bsmCheck, int version) {
    switch (version) {
    case 1: {
        if (BSMNumV1 < MAX_MDM_LENGTH) {
            BSMNumV1++;
        }
        for (int var = BSMNumV1 - 1; var > 0; --var) {
            bsmCheckListV1[var] = bsmCheckListV1[var - 1];
        }
        bsmCheckListV1[0] = bsmCheck;
        break;
    }

    case 2: {
        if (BSMNumV2 < MAX_MDM_LENGTH) {
            BSMNumV2++;
        }
        for (int var = BSMNumV2 - 1; var > 0; --var) {
            bsmCheckListV2[var] = bsmCheckListV2[var - 1];
        }
        bsmCheckListV2[0] = bsmCheck;
        break;
    }
    }

}

Kalman_SVI* MDMHistory::getKalmanSviv1() {
    return &kalmanSVIV1;
}

Kalman_SVI* MDMHistory::getKalmanSviv2() {
    return &kalmanSVIV2;
}

Kalman_SC* MDMHistory::getKalmanSvsiv1() {
    return &kalmanSVSIV1;
}

Kalman_SC* MDMHistory::getKalmanSvsiv2() {
    return &kalmanSVSIV2;
}

Kalman_SI* MDMHistory::getKalmanSaiv1() {
    return &kalmanSAIV1;
}

Kalman_SI* MDMHistory::getKalmanSaiv2() {
    return &kalmanSAIV2;
}

Kalman_SI* MDMHistory::getKalmanSiv1() {
    return &kalmanSIV1;
}

Kalman_SI* MDMHistory::getKalmanSiv2() {
    return &kalmanSIV2;
}

Kalman_SI* MDMHistory::getKalmanViv1() {
    return &kalmanVIV1;
}

Kalman_SI* MDMHistory::getKalmanViv2() {
    return &kalmanVIV2;
}
