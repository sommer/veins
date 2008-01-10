/* -*- mode:c++ -*- */
/**
 * This is an example utility module that keeps end of simulation statistics. 
 */

#ifndef STATISTICSUTILITY_H
#define STATISTICSUTILITY_H

#include <map>
#include <string>
#include <ostream>

#include <BaseUtility.h>
#include <PassedMessage.h>

class PacketStatistics {
 protected:
    class Integer {
    public:
        int value;
        Integer() : value(0){
        }
    };

    typedef std::map<std::string, Integer> NC_t;  // name - count pair
    typedef std::map<int, NC_t>  KNC_t;           // kind - namecount pair
    typedef std::map<PassedMessage::gates_t, KNC_t> GKNC_t;          // gate - kindnamecount pair
    GKNC_t stats;
    
 public:
    void addMsg(const PassedMessage *pm);
    void printStats(std::string& lineStart) const;
};

class StatisticsUtility: public BaseUtility {
 protected:
    Module_Class_Members(StatisticsUtility, BaseUtility, 0);
 protected:
    // track messages per layer
    int catPassedMsg;
    typedef std::map<int, PacketStatistics> Stats_t;
    Stats_t in;
    Stats_t out;
    
 public:
    virtual void initialize(int);
    virtual void finish(void);
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
};

#endif

