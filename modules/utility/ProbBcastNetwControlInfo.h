/*
 * ProbBcastNetwControlInfo.h
 *
 *  Created on: Nov 4, 2008
 *      Author: Damien Piguet
 */

#ifndef IPACNETWCONTROLINFO_H_
#define IPACNETWCONTROLINFO_H_

#include "NetwControlInfo.h"

/**
 * @brief Control info IPAC networking messages
 *
 * Control Info to pass interface information from the network to the
 * application layer and vice versa. The application layer passes the
 * TTL and criticality values for the message.
 *
 * @ingroup utils
 * @author Damien Piguet
 **/
class ProbBcastNetwControlInfo : public NetwControlInfo
{
protected:
    /** @brief netw address of the sending or receiving node*/
    simtime_t ttl;
    double criticality;
    unsigned int id;

public:
    /** @brief Default constructor*/
	ProbBcastNetwControlInfo(simtime_t vttl = 0, double vcriticality = 0.0, unsigned int vid = 0) :
		ttl(vttl), criticality(vcriticality), id(vid) {};
    /** @brief Destructor*/
    virtual ~ProbBcastNetwControlInfo(){};

    /** @brief Getter method*/
    virtual const simtime_t getTtl(){
    	return ttl;
    };

    /** @brief Setter method*/
    virtual void setTtl(const simtime_t vttl){
    	ttl = vttl;
    };

    /** @brief Getter method*/
	virtual const double getCriticality(){
		return criticality;
	};

	/** @brief Setter method*/
	virtual void setCriticality(const double vcriticality){
		criticality = vcriticality;
	};

    /** @brief Getter method*/
	virtual const unsigned int getId(){
		return id;
	};

	/** @brief Setter method*/
	virtual void setId(const unsigned int vid){
		id = vid;
	};
};


#endif /* IPACNETWCONTROLINFO_H_ */
