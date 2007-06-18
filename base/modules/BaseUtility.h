#ifndef BASE_UTILITY_H
#define BASE_UTILITY_H

#include "BaseModule.h"
#include "Coord.h"

class BaseUtility : public BaseModule {
private:
	Coord pos;
    bool coreDebug;
protected:
	Module_Class_Members(BaseUtility, BaseModule, 0);

public:
	/** @brief This modules should only receive self-messages*/
	void handleMessage(cMessage *msg);

	/** @brief Initializes mobility model parameters.*/
	virtual void initialize(int);

	/** @brief Delete dynamically allocated objects*/
//	virtual void finish(){};

	/** @brief Get current position */
	const Coord* getPos() {return &pos;}

	void setPos(Coord* newCoord);
};

#endif

