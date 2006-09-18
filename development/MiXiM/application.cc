#include "application.h"

Define_Module(Application);

void Application::initialize() {
	MiximBaseModule::initialize();
	printfNoInfo(PRINT_INIT, "\tInitializing abstract application...");
	node = (Node*)((parentModule()->parentModule())->submodule("node"));
}

void Application::finish() {
	printfNoInfo(PRINT_INIT, "\tEnding abstract application...");
	MiximBaseModule::finish();
}

void Application::eatCycles(unsigned cycles) {
	node->eatCycles(cycles);
}

