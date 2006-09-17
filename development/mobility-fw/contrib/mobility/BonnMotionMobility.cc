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

    EV << "initializing BonnMotionMobility stage " << stage << endl;

    if(stage == 0)
    {
        int nodeId = par("nodeId");
        if (nodeId == -1)
            nodeId = parentModule()->index();

        const char *fname = par("traceFile");
        const BonnMotionFile *bmFile = BonnMotionFileCache::instance()->getFile(fname);

        vecp = bmFile->getLine(nodeId);
        if (!vecp)
            error("invalid nodeId %d -- no such line in file '%s'", nodeId, fname);
        vecpos = 0;

        // obtain initial position
        const BonnMotionFile::Line& vec = *vecp;
        if (vec.size()>=3){
            move.startPos.x = vec[1];
            move.startPos.y = vec[2];

	    move.startTime = vec[0];

	    //vecpos += 3;
            targetPos = move.startPos;
	    targetTime = simTime();
	    //stepTarget = move.startPos;

	    //dummy value; speed not used in BonnMotion
	    move.speed = 1;
	    EV << "start pos: t=" << move.startTime << move.startPos.info() << endl;
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

    if (vecpos+2 >= vec.size())
    {
	move.speed = 0;
	EV << "host is stationary now!!!\n";
        return;
    }

    targetTime = vec[vecpos];
    targetPos.x = vec[vecpos+1];
    targetPos.y = vec[vecpos+2];
    vecpos += 3;

    EV << "TARGET: t=" << targetTime << " (" << targetPos.x << "," << targetPos.y << ")\n";
}

void BonnMotionMobility::fixIfHostGetsOutside()
{
    Coord dummy; 
    double dum;

    handleIfOutside( RAISEERROR, stepTarget, dummy, dummy, dum );
}

