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

#ifndef __VEINS_VarThrePrintable_H_
#define __VEINS_VarThrePrintable_H_

#include <stdio.h>
#include <stdlib.h>     /* atof */
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <veins/modules/application/f2md/mdEnumTypes/MbTypes.h>

using namespace std;
using namespace boost;

class VarThrePrintable {


#define THRE_MIN 0
#define THRE_STEP 0.1
#define THRE_MAX 1

#define THRE_SIZE 11

private:
    char name[32];

    double DeltaTPFP = 0;
    double TP[THRE_SIZE]= {0,0,0,0,0,0,0,0,0,0,0};
    double TN[THRE_SIZE]= {0,0,0,0,0,0,0,0,0,0,0};
    double FP[THRE_SIZE]= {0,0,0,0,0,0,0,0,0,0,0};
    double FN[THRE_SIZE]= {0,0,0,0,0,0,0,0,0,0,0};

public:
    VarThrePrintable();
    VarThrePrintable(const char *);

    void setName(const char *);

    void registerMessage(mbTypes::Mbs mbType, double minFactor);

    void resetAll();
    void getVarThrePrintable(char* outStr, bool printOut);
    void writeFile(std::string path, char* printStr);
    void saveFile(std::string path, std::string serial, bool printOut);
};

#endif
