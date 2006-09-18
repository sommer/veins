#include "ultrasound.h"

Define_Module(Ultrasound);

void Ultrasound::initialize() {
	MiximBaseModule::initialize();
	printfNoInfo(PRINT_INIT, "\tUltrasound super layer initializing...");
}

void Ultrasound::finish() {
	printfNoInfo(PRINT_INIT, "\tUltrasound super layer finishing...");
	MiximBaseModule::finish();
}

void Ultrasound::handleMessage(cMessage* msg) {
	// TODO
}

