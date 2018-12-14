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

#include <veins/modules/application/f2md/mdSupport/VarThrePrintable.h>

VarThrePrintable::VarThrePrintable(const char* name) {
    strcpy(this->name, name);
}

VarThrePrintable::VarThrePrintable() {
}

void VarThrePrintable::setName(const char* name) {
    strcpy(this->name, name);
}

void VarThrePrintable::registerMessage(mbTypes::Mbs mbType, double minFactor) {
    int index = 0;
    for (double var = THRE_MIN; var <= THRE_MAX; var = var + THRE_STEP) {
        if (minFactor <= var) {

            switch (mbType) {
            case mbTypes::LocalAttacker:
                TP[index]++;
                break;
            case mbTypes::Genuine:
                FP[index]++;
                break;
            case mbTypes::GlobalAttacker:
                FP[index]++;
                break;
            }
        } else {
            switch (mbType) {
            case mbTypes::LocalAttacker:
                FN[index]++;
                break;
            case mbTypes::Genuine:
                TN[index]++;
                break;
            case mbTypes::GlobalAttacker:
                TN[index]++;
                break;
            }
        }
        index++;
    }
}

void VarThrePrintable::resetAll() {
    int index = 0;
    for (double var = THRE_MIN; var <= THRE_MAX; var = var + THRE_STEP) {
        TP[index] = 0;
        FP[index] = 0;
        FN[index] = 0;
        TN[index] = 0;
        index++;
    }
}

void VarThrePrintable::writeFile(std::string path, char* printStr) {
    ofstream outFile;
    outFile.open(path, std::ofstream::out);
    outFile.seekp(0, std::ios::end);
    outFile << printStr << "\n";
    outFile.close();
}

void VarThrePrintable::getVarThrePrintable(char* outStr, bool printOut) {
    char line[1024] = "";
    char data[64] = "";

    strcat(line, "# ");
    strcat(line, name);
    strcat(line, "        TP        FP      TN      FN\n");
    int index = 0;
    for (double var = THRE_MIN; var <= THRE_MAX; var = var + THRE_STEP) {
        sprintf(data, "%f", var);
        strcat(line, data); //1
        strcat(line, " ");
        sprintf(data, "%f", TP[index]);
        strcat(line, data); //2
        strcat(line, " ");
        sprintf(data, "%f", FP[index]);
        strcat(line, data); //3
        strcat(line, " ");
        sprintf(data, "%f", TN[index]);
        strcat(line, data); //4
        strcat(line, " ");
        sprintf(data, "%f", FN[index]);
        strcat(line, data); //5
        strcat(line, "\n");
        index++;
    }



    for (int i = 0; i < 1024; ++i) {
        outStr[i] = line[i];
    }

    if(printOut){
        index = 0;
        std::cout << name << "        TP        FP      TN      FN" << '\n';
        for (double var = THRE_MIN; var <= THRE_MAX; var = var + THRE_STEP) {
            std::cout << var << "        " << TP[index] << "       " << FP[index]
                    << "       " << TN[index] << "     " << FN[index] << '\n';
            index++;
        }
    }


}

void VarThrePrintable::saveFile(std::string path, std::string serial, bool printOut) {

    char fileNameApp[64];

    strcpy(fileNameApp, "VarThre");
    strcat(fileNameApp, name);
    strcat(fileNameApp, ".dat");

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

    getVarThrePrintable(outChar, printOut);
    writeFile(filePathGen, outChar);

}

