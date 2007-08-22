#include <set>
#include <cmath>

#include "TestConnectionManager.h"
#include "BaseWorldUtility.h"


#ifndef ccEV
#define ccEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << "ConnectionManager: "
#endif

Define_Module( TestConnectionManager );

void TestConnectionManager::initialize(int stage)
{
	BaseConnectionManager::initialize(stage);
	if (stage == 1)
	{
		//initialize node grid
		//step 1 - calculate dimension of grid
		//one cell should have the size of maxInterferenceDistance
		Coord dim((*playgroundSize) / maxInterferenceDistance + Coord(1.0, 1.0, 1.0));		
		
		gridDim = GridCoord(dim);

		//A grid smaller or equal to 3x3 whould mean that every cell has every other cell as direct
		//neighbor (if our playground is a torus, even if not the most of the cells are direct
		//neighbors of each other. So we reduce the grid size to 1x1.
		if((gridDim.x <= 3) && (gridDim.y <= 3) && (gridDim.z <= 3))
		{
			gridDim.x = 1;
			gridDim.y = 1;
			gridDim.z = 1;
		}

		//step 2 - initialize the matrix which represents our grid
		NicEntries entries;
		RowVector row;
		NicMatrix matrix;

		for (unsigned i = 0; i < gridDim.z; ++i) {
			row.push_back(entries);					//copy empty NicEntries to RowVector
		}
		for (unsigned i = 0; i < gridDim.y; ++i) {	//fill the ColVector with copies of 
			matrix.push_back(row);					//the RowVector.
		}
		for (unsigned i = 0; i < gridDim.x; ++i) {	//fill the grid with copies of
			nicGrid.push_back(matrix);				//the matrix.
		}											
		ccEV << " using " << gridDim.x << "x" <<
							 gridDim.y << "x" <<
							 gridDim.z << " grid" << endl;
		
		//step 3 -	calculate the factor which maps the coordinate of a node
		//			to the grid cell

		if (gridDim.x == 1 &&												//if we use a 1x1 grid 
			gridDim.y == 1 &&												//every coordinate is
			gridDim.z == 1) {									 			//mapped to (1,1, 1)
			findDistance = FWMath::max(playgroundSize->getX(),				//so the factor is the 
									   FWMath::max(playgroundSize->getY(),	//largest number a 
												   playgroundSize->getZ()))	//coordinate can get (+1)
						   + 1.0;								
		} else {
			findDistance = ceil(maxInterferenceDistance);		//otherwise the factor is our
			if(findDistance == maxInterferenceDistance)			//maxInterferenceDistance
				findDistance += EPSILON;						//TODO: find out why +EPSILON, why ceil()
		}
		ccEV << "findDistance is " << findDistance << endl;


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
double TestConnectionManager::calcInterfDist()
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

/**
* Calculates the corresponding cell of a coordinate.
*/
TestConnectionManager::GridCoord TestConnectionManager::getCellForCoordinate(const Coord& c) {
    return GridCoord(c, findDistance);
}

void TestConnectionManager::updateConnections(int nicID, const Coord* oldPos, const Coord* newPos)
{
	GridCoord oldCell = getCellForCoordinate(*oldPos);
    GridCoord newCell = getCellForCoordinate(*newPos);
	
	checkGrid(oldCell, newCell, nicID );
}

/**
* Returns the NicEntries of the cell with specified
* coordinate.
*/
BaseConnectionManager::NicEntries& TestConnectionManager::getCellEntries(TestConnectionManager::GridCoord& cell) {
    return nicGrid[cell.x][cell.y][cell.z];
}

void TestConnectionManager::registerNicExt(int nicID)
{
	NicEntry* nicEntry = nics[nicID];
    
	GridCoord cell = getCellForCoordinate(nicEntry->pos);
	
	ccEV <<" registering (ext) nic at loc " << cell.info() << std::endl;
	
	// add to matrix
	NicEntries& cellEntries = getCellEntries(cell);
    cellEntries[nicID] = nicEntry;
}

/**
 * Called by ConnectionManager::updateConnections(...) when a nic has
 * moved. 
 **/
void TestConnectionManager::checkGrid(TestConnectionManager::GridCoord& oldCell,
                                      TestConnectionManager::GridCoord& newCell,
                                      int id)
    
{
    
    // structure to find union of grid squares
    CoordSet gridUnion(73);
    
    // find nic at old position
    NicEntries oldCellEntries = getCellEntries(oldCell);
    NicEntries::iterator it = oldCellEntries.find(id);
    NicEntry *nic = it->second;

    
    // move nic to a new position in matrix
    if(oldCell != newCell) {
    	oldCellEntries.erase(it);
    	getCellEntries(newCell)[id] = nic;
    }
    
	if((gridDim.x == 1) && (gridDim.y == 1) && (gridDim.z == 1)) {
		gridUnion.add(oldCell);
    } else {
		//add grid around oldPos
		fillUnionWithNeighbors(gridUnion, oldCell);


        if(oldCell != newCell) {
            //add grid around newPos
            fillUnionWithNeighbors(gridUnion, newCell);
        }
    }

    GridCoord* c = gridUnion.next();
    while(c != 0) {
		updateNicConnections(getCellEntries(*c), nic);
		c = gridUnion.next();
    }
}

/**
 * If the value is outside of its bounds (zero and max) this function
 * returns -1 if useTorus is false and the wrapped value if useTorus is true.
 * Otherwise its just returns the value unchanged.
 */
int TestConnectionManager::wrapIfTorus(int value, unsigned max) {
	if(value < 0) {
		if(useTorus) {
			return max + value;
		} else {
			return -1;
		}
	} else if(value >= max) {
		if(useTorus) {
			return value - max;
		} else {
			return -1;
		}
	} else {
		return value;
	}
}

/**
 * Adds every direct Neighbor of a GridCoord to a union of coords.
 */
void TestConnectionManager::fillUnionWithNeighbors(CoordSet& gridUnion, GridCoord cell) {
	for(int iz = (int)cell.z - 1; iz <= (int)cell.z + 1; iz++) {
		if(iz != cell.z && cell.use2D) {
			continue;
		}
		int cz = wrapIfTorus(iz, gridDim.z);
		if(cz == -1) {
			continue;
		}
		for(int ix = (int)cell.x - 1; ix <= (int)cell.x + 1; ix++) {
			unsigned cx = wrapIfTorus(ix, gridDim.x);
			if(cx == -1) {
				continue;
			}
			for(int iy = (int)cell.y - 1; iy <= (int)cell.y + 1; iy++) {
				int cy = wrapIfTorus(iy, gridDim.y);
				if(cy != -1) {
					if(cell.use2D) {
						gridUnion.add(GridCoord(cx, cy));
					} else {
						gridUnion.add(GridCoord(cx, cy, cz));
					}
				}
			}
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
void TestConnectionManager::updateNicConnections(NicEntries& nmap, NicEntry* nic)
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
