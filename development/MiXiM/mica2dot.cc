#include "mica2dot.h"

Define_Module_Like(Mica2dot, NodeClass);

void Mica2dot::initialize() {
	Node::initialize();
	printfNoInfo(PRINT_INIT, "Initializing Mica2dot...");
	joulesPerCycle = 2.3; // TODO bogus value
}

void Mica2dot::finish() {
	printfNoInfo(PRINT_INIT, "Ending Mica2dot...");
	Node::finish();
}
