#ifndef BASECONNECTIONMANAGER_H_
#define BASECONNECTIONMANAGER_H_

#include "BaseModule.h"
#include "NicEntry.h"

/**
 * @brief Module to control the channel and handle all connection
 * related stuff
 *
 * The central module that coordinates the connections between all
 * nodes, and handles dynamic gate creation. BasicConnectionManager therefore
 * periodically communicates with the ChannelAccess modules
 * 
 * You may not instantiate BasicConnectionManager! 
 * Use ConnectionManager instead.
 *       
 * @ingroup ConnectionManager
 * @author Steffen Sroka, Daniel Willkomm, Karl Wessel
 * @sa ChannelAccess
 */
class BaseConnectionManager : public BaseModule
{
protected:
	typedef std::map<int, NicEntry*> NicEntries;
	
	NicEntries nics;
	
	/** @brief Set debugging for the basic module*/
	bool coreDebug;
	
	/** @brief Does the ConnectionManager use sendDirect or not?*/
	bool sendDirect;
	
	/** @brief Stores the size of the playground.*/
	const Coord* playgroundSize;
	
	/** @brief the biggest interference distance in the network.*/
	double maxInterferenceDistance;

	/** @brief Square of maxInterferenceDistance
	 * cache a value that is often used
	 */
	double maxDistSquared;
	
	/** @brief Stores the useTorus flag of the WorldUtility */
	bool useTorus;
	
protected:
	
	/**
	 * @brief This method is called by "registerNic()" after the nic has been
	 * registered. That means that the NicEntry for the nic has already been 
	 * created and added to nics map.
	 * 
	 * Derived classes of BaseConnectionManager may override this function
	 * to do their own stuff after the registration of a nic. 
	 * They don't need to update the connections for the new nic
	 * because "updateConnections()" is already called by "registerNic()"
	 * after registerNicExt().
	 * 
	 * The order of method calls looks like this:
	 * 
	 * registerNic(...){
	 * -> <NicENtry for nic is created and stored in nics-map>
	 * -> registerNicExt(...)
	 * -> updateConnections(...)
	 * }
	 * 
	 * See ConnectionManager::registerNicExt() for an example.
	 * 
	 * @param nicID - the id of the NicEntry
	 */
	virtual void registerNicExt(int nicID) {};
	
	
	/** 
	 * @brief Calculate interference distance
	 * This method has to be overridden by any derived class.
	 */
	virtual double calcInterfDist() = 0;
	
	/**
	 * @brief Updates the connections of the nic with "nicID".
	 * 
	 * This method is called by "updateNicPos()" after the 
	 * new Position is stored in the corresponding nic.
	 * 
	 * This method has to be overridden by any derived class.
	 * 
	 * @param nicID the id of the NicEntry
	 * @param oldPos the old position of the nic
	 * @param newPos the new position of the nic
	 */
	virtual void updateConnections(int nicID, const Coord* oldPos, const Coord* newPos) = 0;

public:
	
	/**
	 * @brief Constructor
	 **/
	Module_Class_Members(BaseConnectionManager, BaseModule, 0);
	
	
	virtual ~BaseConnectionManager();
	    
	/**
	 * @brief Reads init parameters and calculates a maximal interfence
	 * distance
	 **/
	virtual void initialize(int stage);
	
	/** 
	 * @brief Registers a nic to have its connections managed by ConnectionManager.
	 * 
	 * If you want to do your own stuff at the registration of a nic see
	 * "registerNicExt()".
	 */
	bool registerNic(cModule* nic, const Coord* nicPos);
	
	/** @brief Updates the position information of a registered nic.*/
	void updateNicPos(int nicID, const Coord* newPos);
	    
	/** @brief Returns the ingates of all nics in range*/
	const NicEntry::GateList& getGateList( int nicID);

	/** @brief Returns the ingate of the with id==targetID, or 0 if not in range*/
	const cGate* getOutGateTo(int nicID, int targetID);
};

#endif /*BASECONNECTIONMANAGER_H_*/
