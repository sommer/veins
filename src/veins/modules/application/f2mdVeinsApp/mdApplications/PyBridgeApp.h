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

#ifndef __VEINS_PyBridgeApp_H_
#define __VEINS_PyBridgeApp_H_

#include <tuple>
#include <omnetpp.h>
#include <stdio.h>
#include <stdlib.h>     /* atof */
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <veins/modules/application/f2mdVeinsApp/mdApplications/MDApplication.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/BsmPrintable.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/HTTPRequest.h>
#include <iostream>
#include <string>
#include <vector>


using namespace Veins;
using namespace omnetpp;

class PyBridgeApp: public MDApplication {
public:

    int port = 8888;
    std::string host = "localhost";
    double minFactor = 1;

    LAddress::L2Type myId;

    HTTPRequest httpr = HTTPRequest(8888, "localhost");

    BsmPrintable bsmPrint;

    PyBridgeApp(int version ,int port, std::string host);

    bool CheckNodeForReport(unsigned long myPseudonym,
            BasicSafetyMessage * bsm, BsmCheck * bsmCheck, NodeTable * detectedNodes);

    void setMyId(LAddress::L2Type myId);

    double getMinFactor();
};

#endif
