#include "cc1000.h"

Define_Module_Like(CC1000, RadioClass);

void CC1000::initialize() {
	Radio::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing CC1000 radio...");
}

void CC1000::finish() {
	Radio::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding CC1000 radio...");
}

