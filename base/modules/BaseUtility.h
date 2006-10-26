#ifndef BASE_UTILITY_H
#define BASE_UTILITY_H

#include "BaseModule.h"

class BaseUtility : public BaseModule {
	Module_Class_Members(BaseUtility, BaseModule, 0);

    public:
	/** @brief This modules should only receive self-messages*/
	void handleMessage(cMessage *msg);

	/** @brief Initializes mobility model parameters.*/
	virtual void initialize(int);

	/** @brief Delete dynamically allocated objects*/
//	virtual void finish(){};
};

#endif

