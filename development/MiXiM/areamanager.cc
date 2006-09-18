#include "message.h"
#include "node.h"
#include "areamanager.h"

int centerNode;

Define_Module( AreaManager );

void AreaManager::initialize() {
	MiximBaseModule::initialize();

	// get params
	area_size = getDoubleParameter("area_size", 55);
	assert(area_size > 0.0);
	double s_frac = getDoubleParameter("area_size_var", 0.05);
	assert(s_frac >= 0.0);
	area_size_v = area_size * s_frac;
	area_lifetime = getDoubleParameter("area_lifetime", 3.0);
	assert(area_lifetime > 0.0);
	double area_freq = getDoubleParameter("area_freq", 1.0);
	assert(area_freq > 0.0);
	area_ival = 1.0/area_freq;
	nnodes = strtol(ev.getParameter(simulation.runNumber(), "net.nodeCount").c_str(), NULL, 0);
	assert(nnodes > 0);

	// create timeout msg
	timeout = new cMessage("timeout");

	// schedule the first active area
	scheduleAt(simTime() + exponential(area_ival, RNG_APP), timeout);
}

void AreaManager::finish() {
	MiximBaseModule::finish();
}

AreaManager::~AreaManager() {
	cancelAndDelete(timeout);
}

void AreaManager::handleMessage(cMessage * msg) {
	// can only be timeout msg
	assert(msg == timeout);
	// schedule new timeout
	scheduleAt(simTime() + exponential(area_ival, RNG_APP), timeout);
	// handle msg
	newArea();
}

void AreaManager::newArea() {
	double lifetime = exponential(area_lifetime, RNG_APP);
	double size;
	do {
		size = normal(area_size, area_size_v / 2.0, RNG_APP);
		// 95% of size should be in the interval
		// area_size +/- area_size_v
	} while(size < area_size - area_size_v 
			|| size > area_size + area_size_v); // truncate

	size = .25 * size * size; // convert diameter to square of radius
	
	// pick a node as the center
	centerNode = (int)intuniform(0, nnodes-1, RNG_APP);
	Position myPos = ((Node *) parentModule()->submodule("nodes", centerNode)->submodule("node"))->getPosition();

	printf(PRINT_APP, "new active area around %d, lifetime=%.2lf",
			centerNode, lifetime);
	
	// find all nodes in area
	cModule *node = parentModule()->submodule("nodes", 0);
	for (int i = 0; i < node->size(); i++) {
		Position toPos = ((Node *) node->submodule("node"))->getPosition();
		if (absDistance(myPos, toPos) < size) {
			cModule * app = node // node
				-> submodule("software") // software
				-> submodule("application"); // app
			// make active
			sendDirect(new cMessage("become active",BECOME_ACTIVE),0.0,app,"from_area");
			// make idle
			sendDirect(new cMessage("become idle", BECOME_IDLE), lifetime, app, "from_area");
		}
	}
}
