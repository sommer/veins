#include "BaseUtility.h"
#include "BaseWorldUtility.h"
#include <assert.h>

Define_Module(BaseUtility);

void BaseUtility::initialize(int stage) {
	BaseModule::initialize(stage);
	if (stage == 0) {
		hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
		// reading the out from omnetpp.ini makes predefined scenarios a lot easier
		if (hasPar("x") && hasPar("y") && hasPar("z")){
			// pos.x = par("x");
			// pos.y = par("y");
			// pos.z = par("z");
			pos.setX(par("x"));
			pos.setY(par("y"));
			pos.setZ(par("z"));
		} else{
			// Start at a random position
			// pos.x = pos.y = pos.z = -1;
			pos.setX(-1);
			pos.setY(-1);
			pos.setZ(-1);
		}
		coreEV << "pos: " << pos.info() << endl;
	} else if (stage == 1) {
		BaseWorldUtility *world = dynamic_cast<BaseWorldUtility*>(getGlobalModule("BaseWorldUtility"));
		assert(world!=NULL);
		Coord pgs =  world->getPgs();

		coreEV << "pos: " << pos.info() << endl;

		// -1 indicates start at random position
		if (pos.getX() == -1 || pos.getY() == -1) // consideration of 3D?
		{
			pos = world->getRandomPosition();
			coreEV << "pos: " << pos.info() << endl;
		}
		//we do not have negative positions
		//also checks whether position is within the playground
		else if (	pos.getX() < 0 || pos.getY() < 0 || pos.getZ() < 0 ||
				pos.getX() > pgs.getX() || pos.getY() > pgs.getY() || pos.getZ() > pgs.getZ())
			error("node position specified in omnetpp.ini exceeds playgroundsize");

	}
}

void BaseUtility::handleMessage(cMessage *msg) {
	error("This module does not handle any messages yet");
}

void BaseUtility::setPos(Coord* newCoord) {
  coreEV << "Setting position to pos: " << pos.info() << endl;
	pos.setX() = newCoord->getX();
	pos.setY() = newCoord->getY();
	pos.setZ() = newCoord->getZ();
}

