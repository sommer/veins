#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include <vector>
#include <list>

#include "BaseConnectionManager.h"

class ConnectionManager : public BaseConnectionManager
{
protected:
	
	/** 
	 * @brief Calculate interference distance
	 */
	virtual double calcInterfDist();
	
public:
	/**
	 * @brief Constructor
	 **/
	//Module_Class_Members(ConnectionManager, BaseConnectionManager, 0);
};

#endif /*CONNECTIONMANAGER_H_*/
