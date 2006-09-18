#include "mica2.h"

Define_Module_Like(Mica2, NodeClass);

void Mica2::initialize() {
	Node::initialize();
	printfNoInfo(PRINT_INIT, "Initializing Mica2...");
	joulesPerCycle = 2.3; // TODO bogus value
}

void Mica2::finish() {
	printfNoInfo(PRINT_INIT, "Ending Mica2...");
	Node::finish();
}
