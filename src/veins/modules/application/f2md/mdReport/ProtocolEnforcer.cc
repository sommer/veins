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

#include <veins/modules/application/f2md/mdReport/ProtocolEnforcer.h>

ProtocolEnforcer::ProtocolEnforcer() {
    reportedPseudosNum = 0;
}


bool ProtocolEnforcer::addMisbehavingPseudo(unsigned long pseudo, double curTime){
    int pseudoIndex = -1;

    for (int var = 0; var < reportedPseudosNum; ++var) {
        if(reportedPseudos[var] == pseudo){
            pseudoIndex = var;
            break;
        }
    }

    if(pseudoIndex !=-1){
        lastMisbehavingTime[pseudoIndex] = curTime;
        return false;
    }else{
        if(reportedPseudosNum<MAX_REP_PSEUDOS){
            reportedPseudos[reportedPseudosNum] = pseudo;
            reportTime[reportedPseudosNum] = curTime;
            lastMisbehavingTime[reportedPseudosNum] = curTime;
            reportedPseudosNum ++;
        }
        return true;
    }
}

int ProtocolEnforcer::getReportPseudoes(double curTime, unsigned long * pseudosList){
    int reportCount = 0;
    for (int var = 0; var < reportedPseudosNum; ++var) {
        if((curTime - reportTime[var]) > CollectionPeriod){
            pseudosList[reportCount] = reportedPseudos[var];
            reportCount ++;
            reportTime[var] = curTime;
        }
    }
    for (int var = reportedPseudosNum - 1; var >= 0; --var) {
        if((curTime - lastMisbehavingTime[var]) > (CollectionPeriod + UntolerancePeriod)){
            removeMisbehavingPseudo(var);
        }
    }
    return reportCount;
}

void ProtocolEnforcer::removeMisbehavingPseudo(int index){
    for (int var = index; var < reportedPseudosNum -1 ; ++var) {
        reportedPseudos[var] = reportedPseudos[var + 1];
        reportTime[var] = reportTime[var+1];
        lastMisbehavingTime[var] = lastMisbehavingTime[var+1];
    }
    reportedPseudosNum -- ;
}

