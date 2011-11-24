//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "BonnMotionMobility.h"
#include "BonnMotionFileCache.h"
#include "FWMath.h"


Define_Module(BonnMotionMobility);


void BonnMotionMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    debugEV << "initializing BonnMotionMobility stage " << stage << endl;

    if(stage == 0)
    {
        int nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = getParentModule()->getIndex();

        const char*           fname  = par("traceFile");
        const BonnMotionFile *bmFile = BonnMotionFileCache::getInstance()->getFile(fname);

        vecp = bmFile->getLine(nodeId);
        if (!vecp)
            error("invalid nodeId %d -- no such line in file '%s'", nodeId, fname);
        vecpos = 0;
        bIs3D  = par("is3D").boolValue();

        if ((bIs3D && (vecp->size() % 4 != 0)) || (!bIs3D && (vecp->size() % 3 != 0)))
        	opp_warning("nodeId %d -- missing bon-motion elements in '%s'", nodeId, fname);

        // obtain initial position
        const BonnMotionFile::Line& vec = *vecp;
        if (vec.size() >= (bIs3D ? 4 : 3)) {
        	if (!bIs3D || world->use2D()) {
        		move.setStart(Coord(vec[1], vec[2]), vec[0]);
        	}
        	else {
        		move.setStart(Coord(vec[1], vec[2], vec[3]), vec[0]);
        	}
			//vecpos += (bIs3D ? 4 : 3);
			targetPos  = move.getStartPos();
			targetTime = simTime();
			//stepTarget = move.startPos;

			//dummy value; speed not used in BonnMotion
			move.setSpeed(1);
			debugEV << "start pos: t=" << move.getStartTime() << move.getStartPos().info() << endl;
        }
    }
}

BonnMotionMobility::~BonnMotionMobility()
{
    BonnMotionFileCache::deleteInstance();
}

void BonnMotionMobility::setTargetPosition()
{
    const BonnMotionFile::Line& vec = *vecp;

    if (vecpos+(bIs3D ? 3 : 2) >= vec.size())
    {
		move.setSpeed(0);
		debugEV << "host is stationary now!!!\n";
        return;
    }

    targetTime = vec[vecpos];
    targetPos.x = (vec[vecpos+1]);
    targetPos.y = (vec[vecpos+2]);
    if (bIs3D && !world->use2D())
        targetPos.z = (vec[vecpos+3]);
    vecpos    += (bIs3D ? 4 : 3);

    debugEV << "TARGET: t=" << targetTime << targetPos.info() << "\n";
}

void BonnMotionMobility::fixIfHostGetsOutside()
{
    Coord dummy = Coord::ZERO;
    double dum;

    handleIfOutside( RAISEERROR, stepTarget, dummy, dummy, dum );
}

