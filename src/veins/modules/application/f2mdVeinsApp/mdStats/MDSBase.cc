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

#include <veins/modules/application/f2mdVeinsApp/mdStats/MDSBase.h>

MDSBase::MDSBase() {
    init = true;
    totalGenuineNum = 0;
    totalAttackerNum = 0;
    reportedGenuineNum = 0;
    reportedAttackerNum = 0;
    genuineAverageReportDelay = 0;
    attackerAverageReportDelay = 0;

    CountTotalGenuine = 0;
    CountTotalAttacker = 0;
    CountReportedGenuine = 0;
    CountReportedAttacker = 0;

}

MDSBase::MDSBase(const char * name) {
    strcpy(this->name, name);
    init = true;
    totalGenuineNum = 0;
    totalAttackerNum = 0;
    reportedGenuineNum = 0;
    reportedAttackerNum = 0;
    genuineAverageReportDelay = 0;
    attackerAverageReportDelay = 0;

    CountTotalGenuine = 0;
    CountTotalAttacker = 0;
    CountReportedGenuine = 0;
    CountReportedAttacker = 0;
}

char* MDSBase::getName() {
    return this->name;
}

void MDSBase::setName(const char * name) {
    strcpy(this->name, name);
}

void MDSBase::resetAll() {
    totalGenuineNum = 0;
    totalAttackerNum = 0;
    reportedGenuineNum = 0;
    reportedAttackerNum = 0;
    genuineAverageReportDelay = 0;
    attackerAverageReportDelay = 0;

    CountTotalGenuine = 0;
    CountTotalAttacker = 0;
    CountReportedGenuine = 0;
    CountReportedAttacker = 0;
}

void MDSBase::addTotalGenuine(unsigned long pseudo, double time) {
    if (totalGenuineNum >= TOTCARNUM) {
        removeOldestGenuine();
    }
    totalGenuineIdList[totalGenuineNum] = pseudo;
    totalGenuineTimeList[totalGenuineNum] = time;
    totalGenuineNum++;

    CountTotalGenuine++;
}

void MDSBase::addTotalAttacker(unsigned long pseudo, double time) {
    if (totalAttackerNum >= TOTCARNUM) {
        removeOldestAttacker();
    }
    totalAttackerIdList[totalAttackerNum] = pseudo;
    totalAttackerTimeList[totalAttackerNum] = time;
    totalAttackerNum++;

    CountTotalAttacker++;
}

void MDSBase::addReportedGenuine(unsigned long pseudo, double time) {
   int index = totalGenuineIndex(pseudo);
    if(index!=-1){
        reportedGenuineIdList[reportedGenuineNum] = pseudo;
        reportedGenuineTimeList[reportedGenuineNum] = time;

        int totalIndex = totalGenuineIndex(pseudo);
        double deltaTime = time - totalGenuineTimeList[totalIndex];

        genuineAverageReportDelay = (genuineAverageReportDelay * (double) reportedGenuineNum + deltaTime)
                / ((double) reportedGenuineNum + 1);

        reportedGenuineNum++;

        CountReportedGenuine++;
    }

}

void MDSBase::addReportedAttacker(unsigned long pseudo, double time) {
    int index = totalAttackerIndex(pseudo);

    if(index!=-1){
        reportedAttackerIdList[reportedAttackerNum] = pseudo;
        reportedAttackerTimeList[reportedAttackerNum] = time;

        int totalIndex = totalAttackerIndex(pseudo);
        double deltaTime = time - totalAttackerTimeList[totalIndex];

        attackerAverageReportDelay = (attackerAverageReportDelay
                * (double) reportedAttackerNum + deltaTime)
                / ((double) reportedAttackerNum + 1);

        reportedAttackerNum++;

        CountReportedAttacker++;
    }


}

void MDSBase::writeFile(std::string path, char* printStr) {
    ofstream outFile;
    if (init) {
        outFile.open(path, std::ofstream::out);
        init = false;
    } else {
        outFile.open(path,
                std::ofstream::out | std::ofstream::app | std::ofstream::ate);
    }

    outFile.seekp(0, std::ios::end);
    outFile << printStr << "\n";
    outFile.close();
}

bool MDSBase::alreadyReportedGenuine(unsigned long pseudo) {
    for (int var = 0; var < reportedGenuineNum; ++var) {
        if (pseudo == reportedGenuineIdList[var]) {
            return true;
        }
    }
    return false;
}

bool MDSBase::alreadyReportedAttacker(unsigned long pseudo) {
    for (int var = 0; var < reportedAttackerNum; ++var) {
        if (pseudo == reportedAttackerIdList[var]) {
            return true;
        }
    }
    return false;
}

int MDSBase::totalGenuineIndex(unsigned long pseudo) {
    for (int var = 0; var < totalGenuineNum; ++var) {
        if (pseudo == totalGenuineIdList[var]) {
            return var;
        }
    }
    return -1;
}

int MDSBase::totalAttackerIndex(unsigned long pseudo) {
    for (int var = 0; var < totalAttackerNum; ++var) {
        if (pseudo == totalAttackerIdList[var]) {
            return var;
        }
    }
    return -1;
}

void MDSBase::removeOldestAttacker() {
    int oldestTotal = oldestTotalAttacker();
    int reportedIndex = reportedAttackerIndex(
            totalAttackerIdList[oldestTotal]);
    removeTotalAttacker(oldestTotal);
    if (reportedIndex != -1) {
        removeReportedAttacker(reportedIndex);
    }
}

void MDSBase::removeOldestGenuine() {
    int oldestTotal = oldestTotalGenuine();
    int reportedIndex = reportedGenuineIndex(totalGenuineIdList[oldestTotal]);
    removeTotalGenuine(oldestTotal);
    if (reportedIndex != -1) {
        removeReportedGenuine(reportedIndex);
    }
}

//int MDABase::oldestReportedGenuine() {
//    int oldestIndex = 0;
//    double oldestTime = reportedGenuineTimeList[oldestIndex];
//    for (int var = 0; var < reportedGenuineNum; ++var) {
//        if (oldestTime > reportedGenuineTimeList[var]) {
//            oldestTime = reportedGenuineTimeList[var];
//            oldestIndex = var;
//        }
//    }
//    return oldestIndex;
//}



int MDSBase::oldestTotalGenuine() {
    int oldestIndex = 0;
    double oldestTime = totalGenuineTimeList[oldestIndex];
    for (int var = 0; var < totalGenuineNum; ++var) {
        if (oldestTime >= totalGenuineTimeList[var]) {
            oldestTime = totalGenuineTimeList[var];
            oldestIndex = var;
        }
    }
    return oldestIndex;
}
int MDSBase::oldestTotalAttacker() {
    int oldestIndex = 0;
    double oldestTime = totalAttackerTimeList[oldestIndex];
    for (int var = 0; var < totalAttackerNum; ++var) {
        if (oldestTime > totalAttackerTimeList[var]) {
            oldestTime = totalAttackerTimeList[var];
            oldestIndex = var;
        }
    }
    return oldestIndex;
}

//int MDABase::oldestReportedAttacker() {
//    int oldestIndex = 0;
//    double oldestTime = reportedAttackerTimeList[oldestIndex];
//    for (int var = 0; var < reportedAttackerNum; ++var) {
//        if (oldestTime >= reportedAttackerTimeList[var]) {
//            oldestTime = reportedAttackerTimeList[var];
//            oldestIndex = var;
//        }
//    }
//    return oldestIndex;
//}

int MDSBase::reportedGenuineIndex(unsigned long pseudo) {
    for (int var = 0; var < reportedGenuineNum; ++var) {
        if (pseudo == reportedGenuineIdList[var]) {
            return var;
        }
    }
    return -1;
}

int MDSBase::reportedAttackerIndex(unsigned long pseudo) {
    for (int var = 0; var < reportedAttackerNum; ++var) {
        if (pseudo == reportedAttackerIdList[var]) {
            return var;
        }
    }
    return -1;
}



void MDSBase::removeTotalGenuine(int index) {
    for (int var = index; var < totalGenuineNum; ++var) {
        totalGenuineIdList[var] = totalGenuineIdList[var + 1];
        totalGenuineTimeList[var] = totalGenuineTimeList[var + 1];
    }
    totalGenuineNum--;
}

void MDSBase::removeTotalAttacker(int index) {
    for (int var = index; var < totalAttackerNum; ++var) {
        totalAttackerIdList[var] = totalAttackerIdList[var + 1];
        totalAttackerTimeList[var] = totalAttackerTimeList[var + 1];
    }
    totalAttackerNum--;
}

void MDSBase::removeReportedGenuine(int index) {
    for (int var = index; var < reportedGenuineNum; ++var) {
        reportedGenuineIdList[var] = reportedGenuineIdList[var + 1];
        reportedGenuineTimeList[var] = reportedGenuineTimeList[var + 1];
    }
    reportedGenuineNum--;
}

void MDSBase::removeReportedAttacker(int index) {
    for (int var = index; var < reportedAttackerNum; ++var) {
        reportedAttackerIdList[var] = reportedAttackerIdList[var + 1];
        reportedAttackerTimeList[var] = reportedAttackerTimeList[var + 1];
    }
    reportedAttackerNum--;
}

void MDSBase::getPrintable(char* outStr, double time, bool printOut) {

    char line[1024] = "";
    char data[64] = "";
    strcat(line, name);
    strcat(line, " ");
    sprintf(data, "%f", time);
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%lu", CountReportedGenuine);
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%lu", CountTotalGenuine);
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", genuineAverageReportDelay);
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%lu", CountReportedAttacker);
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%lu", CountTotalAttacker);
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", attackerAverageReportDelay);
    strcat(line, data);

    for (int i = 0; i < 1024; ++i) {
        outStr[i] = line[i];
    }


    if(printOut){
        std::cout << "=*-=*-=*-=*- " << name << " =*-=*-=*-=*- Genuine:"
                << CountReportedGenuine << "/" << CountTotalGenuine << " "
                << (double) CountReportedGenuine / (double) CountTotalGenuine * 100
                << "% " << genuineAverageReportDelay << "s |Attacker:"
                << CountReportedAttacker << "/" << CountTotalAttacker << " "
                << (double) CountReportedAttacker / (double) CountTotalAttacker
                        * 100 << "% " << attackerAverageReportDelay << "s" << '\n';
    }

}
