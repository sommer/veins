#include "node.h"

void Node::initialize() {
	MiximBaseModule::initialize();
	node_id = par("node_id");
	printfNoInfo(PRINT_INIT, "Node %d initializing...", getNodeId());
	// make sure node clocks are not synched
	clock_skew = uniform(0.0, 1.0, RNG_NODE);
	currentPosition.x = par("pos_x");
	currentPosition.y = par("pos_y");
	currentPosition.z = par("pos_z");
	radioRange = par("radioRange");
	assert(radioRange > 0.0);
	cpu_cycles = 0;
}

void Node::finish() {
	printfNoInfo(PRINT_INIT, "Node %d ending...", getNodeId());
	printf(PRINT_STATS, "CPU cycles consumed: %u", cpu_cycles);
	MiximBaseModule::finish();
}

/**
 * the value of a 16 bit timer, 32768 Hz crystal
 */
unsigned short Node::getCurrentTime() {
	return (unsigned short)( simTime() * 32768.0 + clock_skew );
}

double Node::getThenTime(unsigned short ticks) {
	double then = floor(simTime() * 32768.0 + clock_skew);
	then += ticks;
	return (then - clock_skew + 0.0001) / 32768.0;
}

int Node::getNodeId() {
	return node_id;
}

Position Node::getPosition() {
	return currentPosition;
}

void Node::eatCycles(unsigned cycles) {
	cpu_cycles += cycles;
	// TODO
	powerConsumed(joulesPerCycle * cycles);
}

