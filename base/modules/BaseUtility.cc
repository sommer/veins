
#include "BaseUtility.h"

Define_Module(BaseUtility);

void BaseUtility::initialize(int stage) {
	BaseModule::initialize(stage);
}

void BaseUtility::handleMessage(cMessage *msg) {
	error("This module does not handle any messages yet");
}

