#include "simpleUS.h"

Define_Module_Like(SimpleUS, UltrasoundClass);

void SimpleUS::initialize() {
	Ultrasound::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing simple ultrasound...");
}

void SimpleUS::finish() {
	Ultrasound::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding simple ultrasound...");
}

