/* -*- mode:c++ -*- ********************************************************
 * Energy Framework for Omnet++, version 0.9
 *
 * Author:  Laura Marie Feeney
 *
 * Copyright 2009 Swedish Institute of Computer Science.
 *
 * This software is provided `as is' and without any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 ***************************************************************************/
/*
 *  a trivial test module that subscribes to the HostState
 *  notification published by the Battery and records when a Host
 *  Failure notification arrives.  All modules should subscribe to
 *  HostState and make sure that events are cancelled, statistics
 *  properly recorded, etc.
 *
 ***************************************************************************/

#include "Subscriber.h"

Define_Module(Subscriber);

void Subscriber::initialize(int stage)
{
  BaseModule::initialize(stage);

  if (stage == 0) {
  }
}

void Subscriber::handleMessage(cMessage *msg)
{
  error("does not handle any messages");
}

void Subscriber::handleHostState(const HostState& hostState)
{

	HostState::States newState;
	newState = hostState.get();

	switch (newState) {
	case HostState::ACTIVE:
		EV << simTime() << "host state ON" << endl;
		break;
	case HostState::FAILED:
		EV << simTime() << " host state FAILED" << endl;
		// battery is depleted, stop events, record statistics
		recordScalar("HostState::FAILED", simTime());
		break;
	case HostState::OFF:
		EV << (simTime()) << " host state OFF" << endl;
		// not used
		break;
	default:
		error ("unknown host state from BB");
	}

}


void Subscriber::finish()
{
  cComponent::finish();
}
