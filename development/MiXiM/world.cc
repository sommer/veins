#include "world.h"

Define_Module(World);

void World::initialize() {
	MiximBaseModule::initialize();
	printfNoInfo(PRINT_INIT, "World  initializing...");
}

void World::finish() {
	printfNoInfo(PRINT_INIT, "World ending...");
	MiximBaseModule::finish();
}

