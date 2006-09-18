#include "message.h"

#include "idle.h"

Define_Module( Idle );

void Idle::init() {
}


void Idle::generate() {
}

void Idle::rx(Packet * msg) {
	delete msg;
}

void Idle::activated() {
}

void Idle::deActivated() {}

void Idle::timeout() {
}

