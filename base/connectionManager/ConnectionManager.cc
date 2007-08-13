#include <set>
#include <cmath>

#include "ConnectionManager.h"
#include "BaseWorldUtility.h"

#ifndef ccEV
#define ccEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << "ConnectionManager: "
#endif

Define_Module( ConnectionManager );

void ConnectionManager::initialize(int stage)
{
	BaseConnectionManager::initialize(stage);
	if (stage == 1)
	{
		unsigned numX, numY;		
		
		NicEntries entries;
		RowVector row;
		row.push_back(entries);

		findDistance = ceil(maxInterferenceDistance);
		if(ceil(maxInterferenceDistance) == maxInterferenceDistance) 
			findDistance += EPSILON;
		
		numX = static_cast<unsigned>(playgroundSize->getX()/maxInterferenceDistance)+1;
		numY = static_cast<unsigned>(playgroundSize->getY()/maxInterferenceDistance)+1;

		if((numX <= 3) && (numY <= 3))
		{
			if(playgroundSize->getX() < playgroundSize->getY()) {
				findDistance = ceil(playgroundSize->getY()) + 1.0;
			}
			else {
				findDistance = ceil(playgroundSize->getX()) + 1.0;
			}
			ccEV <<" using 1x1 grid"<<endl;
			nicMatrix.push_back(row);
		} else {
			for(unsigned i = 1; i < numY; ++i) {
				row.push_back(entries);
			}
			for(unsigned i = 0; i < numX; ++i) {
				nicMatrix.push_back(row);
			}
			ccEV <<" using "<<numX<<"x"<<numY<<" grid"<<endl;
		}
		ccEV << "findDistance is "<<findDistance <<endl;
		maxX = nicMatrix.size()-1;
		maxY = nicMatrix[0].size()-1;
	}
}

/**
 * Calculation of the interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power
 *
 * You may want to overwrite this function in order to do your own
 * interference calculation
 **/
double ConnectionManager::calcInterfDist()
{
  double interfDistance;

  //the minimum carrier frequency for this cell
  double carrierFrequency = par("carrierFrequency");
  //maximum transmission power possible
  double pMax             = par("pMax");
  if (pMax <=0)
  	error("Max transmission power is <=0!");
  //minimum signal attenuation threshold
  double sat              = par("sat");
  //minimum path loss coefficient
  double alpha            = par("alpha");

  double waveLength     = (BaseWorldUtility::speedOfLight/carrierFrequency);
  //minimum power level to be able to physically receive a signal
  double minReceivePower = pow(10.0, sat/10.0);
  
  interfDistance = pow(waveLength * waveLength * pMax / 
		       (16.0*M_PI*M_PI*minReceivePower), 1.0/alpha);
  
  ccEV <<"max interference distance:"<<interfDistance<<endl;
  
  return interfDistance;
}

void ConnectionManager::updateConnections(int nicID, const Coord* oldPos, const Coord* newPos)
{
	unsigned oldX = static_cast<unsigned>(oldPos->getX()/findDistance);
	unsigned oldY = static_cast<unsigned>(oldPos->getY()/findDistance);
	unsigned newX = static_cast<unsigned>(newPos->getX()/findDistance);
	unsigned newY = static_cast<unsigned>(newPos->getY()/findDistance);
    
	checkGrid(oldX, oldY, newX, newY, nicID );
}

void ConnectionManager::registerNicExt(int nicID)
{
	NicEntry* nicEntry = nics[nicID];
	unsigned x,y;
	x = static_cast<unsigned>(nicEntry->pos.getX()/findDistance);
	y = static_cast<unsigned>(nicEntry->pos.getY()/findDistance);

	ccEV <<" registering (ext) nic at loc "<<x<<","<<y<<endl;
	
	// add to matrix
	nicMatrix[x][y][nicID] = nicEntry;
}

/**
 * Called by ConnectionManager::updateConnections(...) when a nic has
 * moved. 
 **/
void ConnectionManager::checkGrid(unsigned oldX, unsigned oldY,
                               unsigned newX, unsigned newY,
                               int id)
    
{
    unsigned cX, cY;
    
    // structure to find union of grid squares
    std::map<unsigned, std::set<unsigned> > gridUnion;
    std::map<unsigned, std::set<unsigned> >::const_iterator gUmIt;
    std::set<unsigned>::const_iterator gUsIt;
    
    // find nic at old position
    NicEntries::iterator it = nicMatrix[oldX][oldY].find(id);
    NicEntry *nic = it->second;
    
    // move nic to a new position in matrix
    if((newX != oldX) || (newY != oldY)) {
    	nicMatrix[oldX][oldY].erase(it);
    	nicMatrix[newX][newY][id] = nic;
    }

    if((maxX == 0) && (maxY == 0)) {
        gridUnion[oldX].insert(oldY);
    } else {
        if(decrement(maxX, oldX, &cX)) {
            // square top left of old square
            if(decrement(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
            // square top of old square
            gridUnion[cX].insert(oldY);
            // square top right of old square
            if(increment(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
        }
        
        // square left of old square
        if(decrement(maxY, oldY, &cY)) gridUnion[oldX].insert(cY);
        // old square
        gridUnion[oldX].insert(oldY);
        // square right of old square
        if(increment(maxY, oldY, &cY)) gridUnion[oldX].insert(cY);

        if(increment(maxX, oldX, &cX)) {
            // square bottom left of old square
            if(decrement(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
            // square bottom of old square
            gridUnion[cX].insert(oldY);
            // square bottom right of old square
            if(increment(maxY, oldY, &cY)) gridUnion[cX].insert(cY);
        }

        if((newX != oldX) || (newY != oldY)) {
            if(decrement(maxX, newX, &cX)) {
                // square top left of new square
                if(decrement(maxY, newY, &cY)) gridUnion[cX].insert(cY);
                // square top of new square
                gridUnion[cX].insert(newY);
                // square top right of new square
                if(increment(maxY, newY, &cY)) gridUnion[cX].insert(cY);
            }
            
            // square left of new square
            if(decrement(maxY, newY, &cY)) gridUnion[newX].insert(cY);
            // new square
            gridUnion[newX].insert(newY);
            // square right of new square
            if(increment(maxY, newY, &cY)) gridUnion[newX].insert(cY);

            if(increment(maxX, newX, &cX)) {
                // square bottom left of new square
                if(decrement(maxY, newY, &cY)) gridUnion[cX].insert(cY);
                // square bottom of new square
                gridUnion[cX].insert(newY);
                // square bottom right of new square
                if(increment(maxY, newY, &cY)) gridUnion[cX].insert(cY);
            }
        }
    }
    for(gUmIt = gridUnion.begin(); gUmIt != gridUnion.end(); ++gUmIt)
    {
        cX = gUmIt->first;
        for(gUsIt = gUmIt->second.begin(); gUsIt != gUmIt->second.end(); ++gUsIt)
        {
            cY = *gUsIt;
            ccEV << "Update cons in ["<<cX<<", "<<cY<<"]"<<endl;
            updateNicConnections(nicMatrix[cX][cY], nic);
        }
    }
}

/**
 * Called by ConnectionManager::checkGrid(...) when a nic has
 * moved. Sets up a new connection between two nics, if they are
 * within interference range. Accordingly tears down a connection,
 * if the nics move out of range.
 *
 * @param id Id of the nic that will have its' connections
 * updated
 **/
void ConnectionManager::updateNicConnections(NicEntries& nmap, NicEntry* nic)
{
    int id = nic->nicId;
    bool inRange;

    for(NicEntries::iterator i = nmap.begin(); i != nmap.end(); ++i)
    {
        // no recursive connections
        if ( i->second->nicId == id ) continue;

        NicEntry* nic_i = i->second;

		double distance;
		
        if(useTorus)
        {
        	distance = nic->pos.sqrTorusDist(nic_i->pos, playgroundSize);
        } else {
        	distance = nic->pos.sqrdist(nic_i->pos);
        }
        
        inRange = (distance <= maxDistSquared);
        
        
        if ( inRange && !nic->isConnected(nic_i) ){
            // nodes within communication range: connect
            // nodes within communication range && not yet connected
            ccEV <<"nic #"<<nic->nicId<<" and #"<<nic_i->nicId << " are in range"<<endl;
            nic->connectTo( nic_i );
            nic_i->connectTo( nic );
        }
        else if ( !inRange && nic->isConnected(nic_i) ){
            // out of range: disconnect
            // out of range, and still connected
            ccEV <<"nic #"<<nic->nicId<<" and #"<<nic_i->nicId << " are NOT in range"<<endl;
            nic->disconnectFrom( nic_i );
            nic_i->disconnectFrom( nic );
        }
    }
}

/**
 * Helper functions dealing with the connection handling in the grid
 *@{
 */
/**
 * find the next larger coordinate in grid, return true if the
 * connections in this position should be updated.
 */
bool ConnectionManager::increment(unsigned max, unsigned src, unsigned* target) {
    bool res = true;
    unsigned v = src + 1;
    if(src == max) {
        v = 0;
        if(!useTorus) res = false;
    }
    *target = v;
    return res;
}

/**
 * find the next smaller coordinate in grid, return true if the
 * connections in this position should be updated.
 */
bool ConnectionManager::decrement(unsigned max, unsigned src, unsigned* target) {
    bool res = true;
    unsigned v = src - 1;
    if(src == 0) {
        v = max;
        if(!useTorus) res = false;
    }
    *target = v;
    return res;
}
