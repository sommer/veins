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

#ifndef __VEINS_JOSEPHVEINSAPP_H_
#define __VEINS_JOSEPHVEINSAPP_H_

#include <math.h>
#include <omnetpp.h>
#include <veins/modules/application/f2md/BaseWaveApplLayer.h>
#include <veins/modules/application/f2md/mdChecks/CaTChChecks.h>
#include <veins/modules/application/f2md/mdChecks/LegacyChecks.h>

using namespace omnetpp;
using namespace veins;

#include <veins/modules/application/f2md/mdMessages/BasicSafetyMessage_m.h>

#include <veins/modules/application/f2md/mdSupport/GeneralLib.h>
#include <veins/modules/application/f2md/mdStats/MDStatistics.h>

#include <veins/modules/application/f2md/mdApplications/MDApplication.h>
#include <veins/modules/application/f2md/mdApplications/ThresholdApp.h>
#include <veins/modules/application/f2md/mdApplications/AggregationApp.h>
#include <veins/modules/application/f2md/mdApplications/BehavioralApp.h>
#include <veins/modules/application/f2md/mdApplications/PyBridgeApp.h>
#include <veins/modules/application/f2md/mdApplications/ExperiApp.h>

#include <veins/modules/application/f2md/mdSupport/VarThrePrintable.h>
#include <veins/modules/application/f2md/mdSupport/XmlWriter.h>

#include <veins/modules/application/f2md/mdReport/OneMessageReport.h>
#include <veins/modules/application/f2md/mdReport/EvidenceReport.h>
#include <veins/modules/application/f2md/mdReport/ProtocolReport.h>
#include <veins/modules/application/f2md/mdReport/BasicCheckReport.h>
#include <veins/modules/application/f2md/mdReport/ProtocolEnforcer.h>

#include <ctime>

#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>

#include <veins/modules/application/f2md/mdSupport/BsmPrintable.h>

#include <veins/modules/application/f2md/mdPCPolicies/PCPolicy.h>
#include <veins/modules/application/f2md/mdAttacks/MDAttack.h>
#include <veins/modules/application/f2md/mdAttacks/MDGlobalAttack.h>

#include <veins/modules/application/f2md/mdSupport/HTTPRequest.h>


#define mlHostV1 "localhost"
#define mlHostV2 "localhost"
#define mlPortV1 9997
#define mlPortV2 9998

static unsigned long targetNodes[MAX_TARGET_LENGTH];
static int targetNodesLength = 0;
static double targetClearTime = 0;
static unsigned long accusedNodes[MAX_ACCUSED_LENGTH];
static int accusedNodesLength = 0;
static double accusedClearTime = 0;

static bool linkInit = false;
static LinkControl linkControl = LinkControl();

static bool setDate = false;
static std::string curDate;

BsmCheck bsmCheckV1, bsmCheckV2;

static double meanTimeV1, meanTimeV2 = 0;
static unsigned long numTimeV1, numTimeV2 = 0;
clock_t beginV1, endV1, beginV2, endV2;

static double deltaTV1, deltaTV2, deltaTVS1, deltaTVS2 = 0;
static bool initV1, initV2 = false;

static MDStatistics mdStats = MDStatistics();
static VarThrePrintable varThrePrintableV1 = VarThrePrintable("AppV1");
static VarThrePrintable varThrePrintableV2 = VarThrePrintable("AppV2");

class JosephVeinsApp: public BaseWaveApplLayer {
private:
    GeneralLib genLib = GeneralLib();
public:
    NodeTable detectedNodes;

public:
    virtual void initialize(int stage);
    virtual void finish();
protected:

    MDApplication *AppV1;
    MDApplication *AppV2;

    virtual void onBSM(BasicSafetyMessage* bsm);
    virtual void onWSM(BaseFrame1609_4* wsm);
    virtual void onWSA(DemoServiceAdvertisment* wsa);

    virtual void handleSelfMsg(cMessage* msg);
    virtual void handlePositionUpdate(cObject* obj);

    mbTypes::Mbs induceMisbehavior(double localAttacker, double globalAttacker);
    void LocalMisbehaviorDetection(BasicSafetyMessage* bsm, int version);

    void writeReport(MDReport reportBase,int version,std::string maversion,
            BsmCheck bsmCheck, BasicSafetyMessage *bsm);

    void writeListReport(MDReport reportBase,int version, std::string maversion,
            BsmCheck bsmCheck, BasicSafetyMessage *bsm);

    void sendReport(MDReport reportBase,int version, std::string maversion, BsmCheck bsmCheck,
            BasicSafetyMessage *bsm);

    void writeMdBsm(std::string version, BsmCheck bsmCheck,
            BasicSafetyMessage *bsm);

    void writeMdListBsm(std::string version, BsmCheck bsmCheck,
            BasicSafetyMessage *bsm);

    void writeSelfBsm(BasicSafetyMessage bsm);
    void writeSelfListBsm(BasicSafetyMessage bsm);

    void treatAttackFlags();
    MDAttack mdAttack;
    MDGlobalAttack mdGlobalAttack;

    pseudoChangeTypes::PseudoChange myPcType;
    PCPolicy pcPolicy;

    typedef std::list<Obstacle*> ObstacleGridCell;
    typedef std::vector<ObstacleGridCell> ObstacleGridRow;
    typedef std::vector<ObstacleGridRow> Obstacles;

    ThresholdApp ThreV1 = ThresholdApp(1, 0.5);
    ThresholdApp ThreV2 = ThresholdApp(2, 0.5);

    AggrigationApp AggrV1 = AggrigationApp(1, 0.5, 10.0, 3);
    AggrigationApp AggrV2 = AggrigationApp(2, 0.5, 10.0, 3);

    BehavioralApp BehaV1 = BehavioralApp(1, 0.5);
    BehavioralApp BehaV2 = BehavioralApp(2, 0.5);

    ExperiApp ExperV1 = ExperiApp(1, 10.0, 10, 3);
    ExperiApp ExperV2 = ExperiApp(2, 10.0, 10, 3);

    PyBridgeApp PybgV1 = PyBridgeApp(1, mlPortV1, mlHostV1);
    PyBridgeApp PybgV2 = PyBridgeApp(2, mlPortV2, mlHostV2);

    ProtocolEnforcer reportProtocolEnforcerV1 = ProtocolEnforcer();
    ProtocolEnforcer reportProtocolEnforcerV2 = ProtocolEnforcer();

    double MAX_PLAUSIBLE_ACCEL = 0;
    double MAX_PLAUSIBLE_DECEL = 0;
    double MAX_PLAUSIBLE_SPEED = 0;

    void handleReportProtocol();

public:

    void setMDApp(mdAppTypes::App, mdAppTypes::App);

    void addTargetNode(unsigned long id);
    void removeTargetNode(unsigned long id);
    void clearTargetNodes();
    bool isTargetNode(unsigned long id);

    void addAccusedNode(unsigned long id);
    void removeAccusedNode(unsigned long id);
    void clearAccusedNodes();
    bool isAccusedNode(unsigned long id);

};

#endif
