#include "tnode.h"

Define_Module_Like(TNOde, NodeClass);

void TNOde::initialize() {
	Node::initialize();
	printfNoInfo(PRINT_INIT, "\tInitializing TNOde...");
	joulesPerCycle = 0.023; // TODO bogus value
}

void TNOde::finish() {
	printfNoInfo(PRINT_INIT, "\tEnding TNOde...");
	Node::finish();
}
