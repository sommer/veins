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

#include <veins/modules/application/f2mdVeinsApp/mdBase/NodeTable.h>

NodeTable::NodeTable() {
    nodesNum = 0;
}

int NodeTable::getNodesNum() {
    return nodesNum;
}

NodeHistory* NodeTable::getNodeHistoryList() {
    return nodeHistoryList;
}

unsigned long NodeTable::getNodePseudo(int index) {
    return nodePseudos[index];
}

void NodeTable::put(unsigned long pseudo, NodeHistory nodeHistory,
        MDMHistory mdmHistory) {

    bool included = false;
    int nodeKey;

    for (int var = 0; var < nodesNum; ++var) {
        if (pseudo == nodePseudos[var]) {
            included = true;
            nodeKey = var;
            break;
        }
    }

    if (included) {
        nodeHistoryList[nodeKey] = nodeHistory;
        mdmHistoryList[nodeKey] = mdmHistory;
    } else {
        if (nodesNum < MAX_NODES_LENGTH) {
            nodePseudos[nodesNum] = pseudo;
            nodeHistoryList[nodesNum] = nodeHistory;
            mdmHistoryList[nodesNum] = mdmHistory;
            nodesNum++;
        } else {
            nodeKey = getOldestNode();
            nodePseudos[nodeKey] = pseudo;
            nodeHistoryList[nodeKey] = nodeHistory;
            mdmHistoryList[nodeKey] = mdmHistory;
        }
    }
}

int NodeTable::getOldestNode() {
    int oldestNodeIndex = 0;
    double oldestNodeTime =
            nodeHistoryList[0].getLatestBSMAddr()->getSendingTime().dbl();
    double currentNodeTime = 0;

    for (int var = 0; var < nodesNum; ++var) {
        currentNodeTime =
                nodeHistoryList[var].getLatestBSMAddr()->getSendingTime().dbl();
        if (currentNodeTime < oldestNodeTime) {
            oldestNodeTime = currentNodeTime;
            oldestNodeIndex = var;
        }
    }
    return oldestNodeIndex;
}

NodeHistory* NodeTable::getNodeHistoryAddr(unsigned long nodePseudo) {
    int totalNodes = sizeof(nodePseudos) / sizeof(nodePseudos[0]);
    for (int var = 0; var < totalNodes; ++var) {
        if (nodePseudo == nodePseudos[var]) {
            return &nodeHistoryList[var];
        }
    }
    return &nullNode;
}

MDMHistory* NodeTable::getMDMHistoryAddr(unsigned long nodePseudonym) {
    int totalNodes = sizeof(nodePseudos) / sizeof(nodePseudos[0]);
    for (int var = 0; var < totalNodes; ++var) {
        if (nodePseudonym == nodePseudos[var]) {
            return &mdmHistoryList[var];
        }
    }
    return &nullMDMNode;
}

bool NodeTable::includes(unsigned long nodePseudonym) {
    int totalNodes = sizeof(nodePseudos) / sizeof(nodePseudos[0]);
    for (int var = 0; var < totalNodes; ++var) {
        if (nodePseudonym == nodePseudos[var]) {
            return true;
        }
    }
    return false;
}

double NodeTable::getDeltaTime(unsigned long nodePseudo1, unsigned long nodePseudo2) {
    return fabs(
            getNodeHistoryAddr(nodePseudo1)->getArrivalTime(0)
                    - getNodeHistoryAddr(nodePseudo2)->getArrivalTime(0));
}

BasicSafetyMessage* NodeTable::getRandomBSM() {
    GeneralLib genLib = GeneralLib();
    int randNode = genLib.RandomInt(0, nodesNum-1);
    int randBSM = genLib.RandomInt(0, nodeHistoryList[randNode].getBSMNum()-1);
    return nodeHistoryList[randNode].getBSMAddr(randBSM);
}

BasicSafetyMessage* NodeTable::getNextAttackedBsm(Coord myPosition, int bsmNode, double bsmTime) {
    if(bsmNode==0 || (simTime().dbl() - bsmTime) > 1.1){
        double minDistance = 10000000;
        int index = -1;
        MDMLib mdmLib = MDMLib();
        for (int var = 0; var < nodesNum; ++var) {
            double distance = mdmLib.calculateDistancePtr(&myPosition,&nodeHistoryList[var].getLatestBSMAddr()->getSenderPos());
            if(minDistance > distance){
                minDistance = distance;
                index = var;
            }
        }
        return nodeHistoryList[index].getLatestBSMAddr();
    }else{
        return getNodeHistoryAddr(bsmNode)->getLatestBSMAddr();
    }
}


