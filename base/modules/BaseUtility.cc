#include "BaseUtility.h"
#include "BaseWorldUtility.h"

Define_Module(BaseUtility);

void BaseUtility::initialize(int stage) {
	BaseModule::initialize(stage);
	if (stage == 0) {
		hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
		// reading the out from omnetpp.ini makes predefined scenarios a lot easier
		if (hasPar("x") && hasPar("y") && hasPar("z")){
			pos.x = par("x");
			pos.y = par("y");
			pos.z = par("z");
		} else{
			// Start at a random position
			pos.x = pos.y = pos.z = -1;
		}
	} else if (stage == 1) {
		BaseWorldUtility *world = (BaseWorldUtility*)getGlobalModule("BaseWorldUtility");
		Coord pgs =  world->getPgs();

		coreEV << "pos: " << pos.info() << endl;

		// -1 indicates start at random position
		if (pos.x == -1 || pos.y == -1)
			pos = world->getRandomPosition();
		//we do not have negative positions
		//also checks whether position is within the playground
		else if (	pos.x < 0 || pos.y < 0 || pos.z < 0 ||
				pos.x > pgs.x || pos.y > pgs.y || pos.z > pgs.z)
			error("node position specified in omnetpp.ini exceeds playgroundsize");

		coreEV << "pos: " << pos.info() << endl;
	}
}

void BaseUtility::handleMessage(cMessage *msg) {
	error("This module does not handle any messages yet");
}

void BaseUtility::setPos(Coord* newCoord) {
	pos.x = newCoord->x;
	pos.y = newCoord->y;
	pos.z = newCoord->z;
}

