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

#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeMDMHistory.h>


MDMHistory::MDMHistory() {
    nodePseudonym = 0;
    BSMNumV1 = 0;
    BSMNumV2 = 0;
}

MDMHistory::MDMHistory(unsigned long pseudo) {
    nodePseudonym = pseudo;
    BSMNumV1 = 0;
    BSMNumV2 = 0;
}

BsmCheck MDMHistory::getBsmCheck(int index, int version) {
    if(version == 1){
        return bsmCheckListV1[index];
    }else{
        return bsmCheckListV2[index];
    }
}


void MDMHistory::setBsmCheck(int index, BsmCheck bsmCheckV1, BsmCheck bsmCheckV2) {
    setBsmCheck(index,bsmCheckV1,1);
    setBsmCheck(index,bsmCheckV2,2);
}

void MDMHistory::setBsmCheck(int index, BsmCheck bsmCheck, int version) {
    switch (version) {
        case 1:{
            bsmCheckListV1[index] = bsmCheck;
            break;
        }
        case 2:{
            bsmCheckListV2[index] = bsmCheck;
            break;
        }
    }
}

void MDMHistory::addBsmCheck(BsmCheck bsmCheckV1, BsmCheck bsmCheckV2) {
    addBsmCheck(bsmCheckV1,1);
    addBsmCheck(bsmCheckV2,2);
}

void MDMHistory::addBsmCheck(BsmCheck bsmCheck, int version) {
    switch (version) {
        case 1:{
            if (BSMNumV1 < MAX_MDM_LENGTH) {
                BSMNumV1++;
            }
            for (int var = BSMNumV1 - 1; var > 0; --var) {
                bsmCheckListV1[var] = bsmCheckListV1[var - 1];
            }
            bsmCheckListV1[0] = bsmCheck;
            break;
        }

        case 2:{
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
