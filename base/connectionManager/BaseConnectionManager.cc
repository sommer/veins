#include "BaseConnectionManager.h"

#include "NicEntryDebug.h"
#include "NicEntryDirect.h"

#include "BaseWorldUtility.h"
#include "BaseUtility.h"

#include <cassert>

#ifndef ccEV
#define ccEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << "ConnectionManager: "
#endif

void BaseConnectionManager::initialize(int stage)
{
	BaseModule::initialize(stage);
	if (stage == 0)
	{
		if(hasPar("coreDebug"))
			coreDebug = par("coreDebug").boolValue();
		else
			coreDebug = false;
	}
	else if (stage == 1)
	{		
		ccEV <<"initializing BaseConnectionManager\n";
		
		BaseWorldUtility* world = (BaseWorldUtility*)getGlobalModule("BaseWorldUtility");
		
		assert(world != 0);
		
		playgroundSize = world->getPgs();
		useTorus = world->useTorus();		
		
		if(hasPar("sendDirect"))
			sendDirect = par("sendDirect").boolValue();
		else
			sendDirect = false;
	  
		maxInterferenceDistance = calcInterfDist();
		maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;
	}
}

/**
 * Called by ChannelAccess for the nic module upon
 * initialization. The nics are written into a list. *
 *
 * @param nic the cModule pointer of the registered nic 
 **/
bool BaseConnectionManager::registerNic(cModule* nic, const Coord* nicPos)
{
	assert(nic != 0);
	
	int nicID = nic->id();
	ccEV << " registering nic #" << nicID << endl;		
	
	// create new NicEntry	
	NicEntry *nicEntry;
	
	if(sendDirect)
		nicEntry = new NicEntryDirect(coreDebug);
	else
		nicEntry = new NicEntryDebug(coreDebug);
	
	// fill nicEntry  
	nicEntry->nicPtr = nic;
	nicEntry->nicId = nicID;
	nicEntry->hostId = nic->parentModule()->id();
	nicEntry->pos = nicPos;

	// add to map
	nics[nicID] = nicEntry;
	
	registerNicExt(nicID);
	
	updateConnections(nicID, nicPos, nicPos);

	return sendDirect;	
}

/**
 * Called by ChannelAccess to indicate that the nic
 * with "nicID" moved.
 *
 * Updates the nics' position and all its connections
 *
 * Actually this class stores the new position of the nic.
 * After that it updates the connections by calling the
 * method "updateConnections()".
 * 
 * @param nicID the module id of the registered nic
 * @param newPos the new coordinates of the registered nic
 **/
void BaseConnectionManager::updateNicPos(int nicID, const Coord* newPos)
{
	NicEntry* nicEntry = nics[nicID];
	if(nicEntry == 0)
		error("No nic with this ID is registered with this ConnectionManager.");
		
    Coord oldPos = nicEntry->pos;    
    nicEntry->pos = newPos;
	
	updateConnections(nicID, &oldPos, newPos);
}

/**
 * Returns the gates of all nics in range of the nic with "nicID".
 */
const NicEntry::GateList& BaseConnectionManager::getGateList(int nicID)
{
	return nics[nicID]->getGateList();
}

/**
 * Called by P2PPhyLayer. Needed to send a packet directly to a
 * certain nic without other nodes 'hearing' it. This is only useful
 * for physical layers that work with bit error probability like
 * P2PPhyLayer.
 *
 * @param nicID id of the nic from which the a packet is about to be sent
 * @param targetID id of the nic to which the packet is about to be sent
 */
const cGate* BaseConnectionManager::getOutGateTo(int nicID, int targetID)
{
    return nics[nicID]->getOutGateTo(targetID);
}

BaseConnectionManager::~BaseConnectionManager()
{
	for (NicEntries::iterator ne = nics.begin(); ne != nics.end(); ne++)
	{	
		delete ne->second;
	}
}
