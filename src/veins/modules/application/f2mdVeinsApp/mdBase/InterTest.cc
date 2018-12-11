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

#include <veins/modules/application/f2mdVeinsApp/mdBase/InterTest.h>

InterTest::InterTest() {
    interNum = 0;
    currentNum = 0;
    interPseudonym = new unsigned long[interNum];
    interValue = new double[interNum];
}

InterTest::InterTest(int interNum) {
    this->interNum = interNum;
    currentNum = 0;
    interPseudonym = new unsigned long[interNum];
    interValue = new double[interNum];
}

void InterTest::addInterValue(unsigned long pseudo, double value) {
    interPseudonym[currentNum] = pseudo;
    interValue[currentNum] = value;
    currentNum++;
}

int InterTest::getInterNum() {
    return interNum;
}

unsigned long InterTest::getInterId(int index) {
    return interPseudonym[index];
}

double InterTest::getInterValue(int index) {
    return interValue[index];
}

int InterTest::getIdIndex(unsigned long id) {
    for (int var = 0; var < currentNum; ++var) {
        if(getInterId(var) == id){
            return var;
        }
    }
    return -1;
}

