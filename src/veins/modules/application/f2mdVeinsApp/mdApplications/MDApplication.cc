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

#include <stdio.h>
#include <stdlib.h>     /* atof */
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <veins/modules/application/f2mdVeinsApp/mdApplications/MDApplication.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;
using namespace boost;

MDApplication::MDApplication(int version) {

    this->version = version;

    if(version == 1){
        this->prntApp = &prntAppV1;
        this->prntAppInst = &prntAppInstV1;

        this->prntApp->setName(AppV1Name);
        this->prntAppInst->setName(AppV1Name);
    }else{

        this->prntApp = &prntAppV2;
        this->prntAppInst = &prntAppInstV2;

        this->prntApp->setName(AppV2Name);
        this->prntAppInst->setName(AppV2Name);
    }

}

void MDApplication::saveLine(std::string path, std::string serial, double density,
        double deltaT, bool printOut){

    char fileNameApp[64];
    char fileNameAppInst[64];

    if(version == 1){
        strcpy(fileNameApp,AppV1Name);
        strcat(fileNameApp, ".dat");
        strcpy(fileNameAppInst,AppV1Name);
        strcat(fileNameAppInst, "Inst.dat");
    }else{
        strcpy(fileNameApp,AppV2Name);
        strcat(fileNameApp, ".dat");
        strcpy(fileNameAppInst,AppV2Name);
        strcat(fileNameAppInst, "Inst.dat");
    }

    char outChar[1024];
    char directoryPathGen[1024] = "";
    char filePathGen[1024] = "";
    const char * pathChar = path.c_str();
    const char * serialChar = serial.c_str();
    strcat(directoryPathGen, pathChar);
    strcat(directoryPathGen, serialChar);

    struct stat info;

    if (stat(directoryPathGen, &info) != 0) {
        mkdir(directoryPathGen, 0777);
    } else if (info.st_mode & S_IFDIR) {
    } else {
        mkdir(directoryPathGen, 0777);
    }

    strcpy(filePathGen, directoryPathGen);
    strcat(filePathGen, "/");
    strcat(filePathGen, fileNameApp);

    prntApp->getPrintable(outChar, density, deltaT, printOut);
    prntApp->writeFile(filePathGen, outChar);

    strcpy(filePathGen, directoryPathGen);
    strcat(filePathGen, "/");
    strcat(filePathGen, fileNameAppInst);

    prntAppInst->getPrintable(outChar, density, deltaT, printOut);
    prntAppInst->writeFile(filePathGen, outChar);

}



void MDApplication::resetInstFlags(){
    prntAppInst->resetAll();
}

void MDApplication::resetAllFlags(){
    prntApp->resetAll();
    prntAppInst->resetAll();
}


