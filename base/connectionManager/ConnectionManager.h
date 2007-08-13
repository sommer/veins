#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include <vector>
#include <list>

#include "BaseConnectionManager.h"

class ConnectionManager : public BaseConnectionManager
{
protected:
	typedef std::vector<NicEntries> RowVector;
	typedef std::vector<RowVector> NicMatrix;

   /** @brief Registry of all Nics
    *
    * This matrix keeps all Nics according to their position.  It
    * allows to restrict the position update to a subset of all nics.
    */
    NicMatrix nicMatrix;
    
    /**
     * Distance that helps to find a node under a certain
     * position. Can be larger then @see maxInterferenceDistance to
     * allow nodes to be placed into the same square if the playground
     * is too small for the grid speedup to work. */
    double findDistance;

    /**
     * further cached values
     */
    unsigned maxX, maxY;

protected:
	/**
	 * @brief This method is called by "registerNic()" after the nic has been
	 * registered. That means that the NicEntry for the nic has already been 
	 * created and added to nics map.
	 * 
	 * @param nicID - the id of the NicEntry
	 */
	virtual void registerNicExt(int nicID);
	
	
	/** 
	 * @brief Calculate interference distance
	 */
	virtual double calcInterfDist();
	
	/**
	 * @brief Updates the connections of the nic with "nicID".
	 * 
	 * This method is called by "updateNicPos()" after the 
	 * new Position is stored in the corresponding nic.
	 * 
	 * @param nicID the id of the NicEntry
	 * @param oldPos the old position of the nic
	 * @param newPos the new position of the nic
	 */
	virtual void updateConnections(int nicID, const Coord* oldPos, const Coord* newPos);
	
	/**
     * find the next larger coordinate in grid, return true if the
     * connections in this position should be updated.
     */
	bool increment(unsigned max, unsigned src, unsigned* target);
	
	/**
     * find the next smaller coordinate in grid, return true if the
     * connections in this position should be updated.
     */
    bool decrement(unsigned max, unsigned src, unsigned* target);
    
    /** @brief Manages the connections of a registered nic. */ 
    void updateNicConnections(NicEntries& nmap, NicEntry* nic);

    /**
     * check connections of a nic in the grid
     */
    void checkGrid(unsigned oldX, unsigned oldY,
                   unsigned newX, unsigned newY,
                   int id);
public:
	/**
	 * @brief Constructor
	 **/
	Module_Class_Members(ConnectionManager, BaseConnectionManager, 0);
	
	/**
	 * @brief Reads init parameters and calculates a maximal interfence
	 * distance
	 **/
	virtual void initialize(int stage);
};

#endif /*CONNECTIONMANAGER_H_*/
