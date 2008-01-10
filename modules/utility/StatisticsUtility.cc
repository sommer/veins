/* -*- mode:c++ -*- */
/**
 * This is an example utility module that keeps end of simulation statistics. 
 */

#include "StatisticsUtility.h"

using namespace std;

Define_Module(StatisticsUtility);

void StatisticsUtility::initialize(int stage) {
    BaseUtility::initialize(stage);
    if (stage == 0) {
        PassedMessage pm;
        catPassedMsg = subscribe(this, &pm, -1);
        in.clear();
        out.clear();
    }
}

void StatisticsUtility::receiveBBItem(int category, const BBItem *details, int scopeModuleId) {
    BaseUtility::receiveBBItem(category, details, scopeModuleId);
    if(category == catPassedMsg) {
        const PassedMessage *pm = static_cast<const PassedMessage *>(details);
        if(pm->direction == PassedMessage::INCOMING) {
            in[pm->fromModule].addMsg(pm);
        }
        else {
            out[pm->fromModule].addMsg(pm);
        }
    }
}

void StatisticsUtility::finish() {
    Stats_t::const_iterator it;
    for(it = in.begin(); it != in.end(); ++it) {
        string lineStart("STATS incomingPackets : ");
        BaseModule *bm = check_and_cast<BaseModule*>(simulation.module( it->first ));
        lineStart.append(bm->logName()).append("::").append(bm->className()).append(": ");
        it->second.printStats(lineStart);
    }
    for(it = out.begin(); it != out.end(); ++it) {
        string lineStart("STATS outgoingPackets : ");
        BaseModule *bm = check_and_cast<BaseModule*>(simulation.module( it->first ));
        lineStart.append(bm->logName()).append("::").append(bm->className()).append(": ");
        it->second.printStats(lineStart);
    }
    in.clear();
    out.clear();
}

void  PacketStatistics::addMsg(const PassedMessage *pm) {
    stats[pm->gateType][pm->kind][pm->name].value++;
}

void  PacketStatistics::printStats(std::string& lineStart) const {
    GKNC_t::const_iterator gkit;
    KNC_t::const_iterator kit;
    NC_t::const_iterator it;
    for(gkit = stats.begin(); gkit != stats.end(); ++gkit) { // for each gate 
        string line(lineStart);
        line.append("\tonGate ").append(PassedMessage::gateToString(gkit->first));
        for(kit = gkit->second.begin(); kit != gkit->second.end(); ++kit) { // for each kind
            for(it = kit->second.begin(); it != kit->second.end(); ++it) { // for each name
                ev << line << "\t" << it->second.value << " x " << it->first << endl;
            }
        }
    }
}
