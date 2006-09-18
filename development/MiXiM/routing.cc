#include "routing.h"

//Define_Module(Routing);

bool Routing::parametersInitialised = false;
int Routing::maxQueueLength, Routing::preferTXThreshold;

void Routing::initialize() {
	MiximSoftwareModule::initialize();
	printfNoInfo(PRINT_INIT, "\tInitializing abstract routing...");
	node = (Node*) parentModule()->parentModule()->submodule("node");
	macBusy = false;
	msgQueue = new cQueue();

	stat_tx = stat_rx = stat_tx_drop = stat_rx_dup = 0;
	
	if (!parametersInitialised) {
		maxQueueLength = getLongParameter("maxQueueLength", 10);
		preferTXThreshold = getLongParameter("preferTXThreshold", 8);
		parametersInitialised = true;
	}

	mySerial = 1;
	serials.clear();
}

void Routing::handleEvent(cMessage *msg) {
	Packet *packet = dynamic_cast<Packet *>(msg);

	switch (msg->kind()) {
		case RX:
			if (!registerSerial(packet->from, packet->serial)) {
				printf(PRINT_ROUTING, "Dropping duplicate message from %d serial %d", packet->from, packet->serial);
				stat_rx_dup++;
				delete packet;
				return;
			}
			printf(PRINT_ROUTING, "Received message from %d serial %d", packet->from, packet->serial);
			rx(packet);
			break;
		case TX:
			packet->serial = -1;
			tx(packet);
			break;
		case TX_DONE:
			macBusy = false;
			txDone(packet);
			break;
		case TX_FAILED:
			macBusy = false;
			txFailed(packet);
			break;
		case TX_NO_ACK:
			macBusy = false;
			txNoAck(packet);
			break;
		case TX_CONTEND_LOSE:
			macBusy = false;
			txContendLose(packet);
			break;
		case TX_RETRY_SKIPPED:
			macBusy = false;
			txRetrySkipped(packet);
			break;
		case FORCE_AWAKE_GRANT:
			forceGrant(packet->serial);
			delete packet;
			break;
		default:
			printf(PRINT_CRIT,"packet->kind() = %d",packet->kind());
			assert(0);
	}
}

void Routing::txNoAck(Packet *msg) { error("%s does not support the extended mac<->routing interface", className()); }
void Routing::txSkipped(Packet *msg) { error("%s does not support the extended mac<->routing interface", className()); }
void Routing::txContendLose(Packet *msg) { txSkipped(msg); }
void Routing::txRetrySkipped(Packet *msg) { txSkipped(msg); }
void Routing::txFailed(Packet *msg) { txDone(msg); }

void Routing::sendToMac(Packet *msg) {
	assert((msg->kind() == TX && !macBusy)|| msg->kind() == FORCE_AWAKE_REQ || msg->kind() == FORCE_END);
	if (msg->kind() == TX) {
		macBusy = true;

		if (msg->serial == -1) { // i.e not set
			msg->serial = mySerial++;
			msg->increaseLength(2);
			msg->from = node->getNodeId();
			registerSerial(msg->from, msg->serial);
		}
	}
	send(msg, findGate("toMac"));
}

void Routing::sendToApp(Packet *msg) {
	// Make sure packets don't keep on growing
	msg->decreaseLength(2);
	switch (msg->kind()) {
		case RX:
			stat_rx++;
			break;
		case TX_DONE:
			stat_tx++;
			break;
		case TX_FAILED:
			stat_tx_drop++;
			break;
		default:
			assert(0);
	}
	send(msg, findGate("toApp"));
}

void Routing::finish() {
	printf(PRINT_STATS, "stats: tx=%d rx=%d tx_drop=%d rx_dup=%d queuelen=%d", stat_tx, stat_rx, stat_tx_drop, stat_rx_dup, msgQueue->length());
	
	printfNoInfo(PRINT_INIT, "\tEnding abstract routing...");
	MiximSoftwareModule::finish();
}

Routing::~Routing() {
	parametersInitialised = false;
	delete msgQueue;
}

void Routing::addToQueue(cMessage *msg) {
	if (msgQueue->length() == maxQueueLength) {
		stat_tx_drop++;
		msg->setKind(TX_FAILED);
		send(msg, findGate("toApp"));
		return;
	}
	if (msgQueue->length() >= preferTXThreshold) {
		printf(PRINT_ROUTING, "transmit queue filling up, signal mac");
		cMessage * msg = new cMessage("prefer tx", PREFER_TX);
		send(msg, findGate("toMac"));
	}
	msgQueue->insert(msg);
}

#define HISTORY_BITS 8
#define HISTORY_MASK ((1<<HISTORY_BITS) - 1)

bool Routing::registerSerial(int nodeId, int serial) {
	map<int, SerialInfo>::iterator iter = serials.find(nodeId);
	
	if (iter == serials.end()) {
		SerialInfo info;
		info.lastSerial = serial;
		info.bitmap = 0;
		serials[nodeId] = info;
		return true;
	} else if (iter->second.lastSerial < serial) {
		int shift = serial - iter->second.lastSerial;
		if (shift > HISTORY_BITS + 1)
			iter->second.bitmap = 0;
		else
			iter->second.bitmap = (((1 << HISTORY_BITS) | iter->second.bitmap) >> shift) & HISTORY_MASK;
		iter->second.lastSerial = serial;
		return true;
	} else if (iter->second.lastSerial == serial) {
		return false;
	} else if (iter->second.lastSerial - serial <= HISTORY_BITS) {
		int shift = HISTORY_BITS - (iter->second.lastSerial - serial);
		assert(shift >= 0 && shift <= HISTORY_BITS - 1);
		if (iter->second.bitmap & (1 << shift))
			return false;
		else
			iter->second.bitmap |= (1 << shift);
		return true;
	} else {
		return false;
	}
}

void Routing::forceGrant(bool success) 
{
	assert(0);
}

