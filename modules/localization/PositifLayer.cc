/***************************************************************************
 * file:        PositifLayer.cc
 *
 * author:      Peterpaul Klein Haneveld
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     mixim framework
 * description: localization layer: general class for the network layer
 *              subclass to create your own localization layer
 ***************************************************************************/


#include "PositifLayer.h"
#include "BaseUtility.h"
#include "FindModule.h"
#include "ModuleAccess.h"
#include "ChannelControl.h"
#include "NetwControlInfo.h"
#include <SimpleAddress.h>

Define_Module(PositifLayer);

int PositifLayer::num_nodes;
int PositifLayer::num_anchors;
int PositifLayer::algorithm;
int PositifLayer::version;
unsigned int PositifLayer::nr_dims;
node_info *PositifLayer::node = NULL;
FLOAT PositifLayer::area;
double *PositifLayer::dim;
FLOAT PositifLayer::range;
double PositifLayer::range_variance;
double PositifLayer::pos_variance;
char PositifLayer::topology_type[TOPOLOGYTYPE_LENGTH];
int PositifLayer::flood_limit;
int PositifLayer::refine_limit;
int PositifLayer::tri_alg;
bool PositifLayer::do_2nd_phase = true;
int PositifLayer::phase1_min_anchors;
int PositifLayer::phase1_max_anchors;
FLOAT PositifLayer::var0;
FLOAT PositifLayer::var1;
FLOAT PositifLayer::var2;

struct myParams params;

timer_info _timers[MAX_TIMERS];

/*******************************************************************************
 * static functions
 ******************************************************************************/
const static char *status2string(int status)
{
	const static char *anchor = "ANCHOR";
	const static char *unknown = "UNKNOWN";
	const static char *positioned = "POSITIONED";
	const static char *bad = "BAD";

	if (status == STATUS_ANCHOR)
		return anchor;
	else if (status == STATUS_UNKNOWN)
		return unknown;
	else if (status == STATUS_POSITIONED)
		return positioned;
	else
		return bad;
}

static void delete_cMessage(void *_msg)
{
	cMessage *msg = (cMessage *) _msg;
	delete msg;
}

static double square(double x)
{
	return x * x;
}

/*******************************************************************************
 * Initialization methods
 ******************************************************************************/
void PositifLayer::initialize(int stage)
{
	BaseLayer::initialize(stage);

	switch (stage) {
	case 0:
		RepeatTimer::init(this);
//              me = par("me");
		start_timer = MAX_TIMERS;
		me = findHost()->index();
		/* clear arrays */
		for (int i = 0; i < MAX_MSG_TYPES; i++) {
			seqno[i] = last_sent_seqno[i] = 0;
		}

		/* only one node needs to initialize this */
		if (me == 0) {
			setup_global_vars();
			node = new node_info[num_nodes];
		}
		break;
	case 1:
		{
			headerLength = par("headerLength");

			node[me].ID = me;
			node[me].anchor = false;
			node[me].recv_cnt = 0;
			/* initialize the node specific stuff */
			node[me].neighbors = new cLinkedList();
			node[me].perf_data.bcast_total = new int[MAX_MSG_TYPES];
			node[me].perf_data.bcast_unique =
			    new int[MAX_MSG_TYPES];
			for (unsigned int m = 0; m < nr_dims; m++)
				node[me].perf_data.curr_pos[m] = 0.0;
			node[me].perf_data.flops = 0;
			node[me].perf_data.confidence = 0.0;
			node[me].perf_data.err = -1.0;
			node[me].perf_data.phase1_err = -1.0;
			node[me].perf_data.phase2_err = -1.0;
			node[me].perf_data.status = STATUS_UNKNOWN;
			node[me].perf_data.anchor_range_error = 0.0;
			node[me].perf_data.rel_anchor_range_error = 0.0;
			node[me].perf_data.abs_anchor_range_error = 0.0;
			node[me].perf_data.abs_rel_anchor_range_error = 0.0;
			node[me].perf_data.anchor_range = 0.0;
			node[me].perf_data.anchor_range_error_count = 0;
			for (int m = 0; m < MAX_MSG_TYPES; m++) {
				node[me].perf_data.bcast_total[m] = 0;
				node[me].perf_data.bcast_unique[m] = 0;
			}

			refine_count = 0;

			log = node[me].perf_data.log;
			log[0] = '\0';
			reps = par("repeats");
			use_log = par("use_log");
			msg_buffering = par("msg_buffering");

			// Range errors
			used_anchors = 0;
			range_list = NULL;
			real_range_list = NULL;

			Coord coord = getPosition();
			/* convert from Coord to Position */
			node[me].true_pos[0] = position[0] = coord.x;
			node[me].true_pos[1] = position[1] = coord.y;
			node[me].true_pos[2] = position[2] = coord.z;

#ifndef NDEBUG
			ev << "node " << node[me].
			    ID << "'s true location: " <<
			    pos2str(node[me].true_pos) << "\n";
#endif

			/* In positif this is done by an offset:
			 * genk_normal(1, node[me].true_pos[i], range * pos_variance)
			 */
			for (int i = 0; i < 3; i++)
				node[me].init_pos[i] =
				    genk_normal(1, node[me].true_pos[i],
						range * pos_variance);
		}
		break;
	case 2:
		{
			setup_grid();
			/* init must reserve the number of used repeat timers for the application */
			init();
			/* create the start timer, one repeat */
			setRepeatTimer(start_timer, 1, 1);
			cMessage *msg = new cMessage("START", MSG_START);
			msg->addPar("anchor") = node[me].anchor;
			if (node[me].anchor) {
				add_struct(msg, "position", node[me].true_pos);
			} else {
				add_struct(msg, "position", node[me].init_pos);
			}
			setContextPointer(start_timer, msg);
			setContextDestructor(start_timer, delete_cMessage);
		}
		break;
	default:
		break;
	}
}

void PositifLayer::setup_global_vars(void)
{
	/*random_seed = (long *) malloc(NUM_RAND_GEN * sizeof(long));

	   for (int i = 0; i < NUM_RAND_GEN; i++)
	   random_seed[i] = genk_randseed(i);   // Save for output purposes. */

	node = NULL;
	nr_dims = par("nr_dims");

	dim = new double[nr_dims];

	switch (nr_dims) {
	case 3:
		dim[2] = par("z_dim");
	case 2:
		dim[1] = par("y_dim");
	case 1:
		dim[0] = par("x_dim");
		break;

	default:
		error("initialize() can't handle %d-dimensional space",
		      nr_dims);
		abort();
	}

//      num_nodes = par("num_nodes");
	num_nodes = simulation.systemModule()->par("numHosts");
	fprintf(stdout, "numHosts = %d\n", num_nodes);
	num_anchors =
	    (int) Max(num_nodes * (double) par("anchor_frac"), nr_dims + 1);
	range = (double) par("range");
	do_2nd_phase = par("do_2nd_phase");	// Each node writes to these global variables, but this shouldn't cause any problems.

	phase1_min_anchors = par("phase1_min_anchors");
	phase1_max_anchors = par("phase1_max_anchors");
	flood_limit = par("flood_limit");
	refine_limit = par("refine_limit");
	tri_alg = par("tri_alg");
	var0 = (double) par("var0");
	var1 = (double) par("var1");
	var2 = (double) par("var2");
}

void PositifLayer::setup_grid()
{
	if (me == 0) {
		//      double grid_d = par("grid_d");

		range_variance = par("range_variance");
		snprintf(topology_type, TOPOLOGYTYPE_LENGTH, "Grid");
		pos_variance = par("pos_variance");

		// Select random anchors
		for (int i = 0; i < num_anchors; i++) {
			int n;

			do {
				n = (int) genk_uniform(1, 0, num_nodes - 1);
			} while (node[n].anchor);

			node[n].anchor = true;
			memcpy(node[n].init_pos, node[n].true_pos,
			       sizeof(node[n].true_pos));
		}
	}
}

/*******************************************************************************
 * Finalization methods
 ******************************************************************************/
void PositifLayer::finish()
{
	if (me == 0) {
		write_statistics();
	}
	handleStopMessage(NULL);
	delete node[me].neighbors;
}

/*******************************************************************************
 * Localization methods
 ******************************************************************************/
Coord PositifLayer::getPosition()
{
	BaseUtility *util =
	    FindModule < BaseUtility * >::findSubModule(findHost());
	const Coord *coord = util->getPos();
	return *coord;
}

/* Check if this node exists in neighbor list. */
bool PositifLayer::isNewNeighbor(int src)
{
	for (cLinkedListIterator iter =
	     cLinkedListIterator(*node[me].neighbors); !iter.end(); iter++) {
		neighbor_info *n = (neighbor_info *) iter();
		if (n->idx == src)
			return false;
	}
	return true;
}

void PositifLayer::addNewNeighbor(int src, double distance,
				   double estimated_distance)
{
	neighbor_info *n = new neighbor_info;
	n->idx = src;
	n->true_dist = distance;
	// Use estimated distance here
	if (estimated_distance > range)
		estimated_distance = range;
	n->est_dist = estimated_distance;
	node[me].neighbors->insert(n);
}

int PositifLayer::logprintf(__const char *__restrict __format, ...)
{
	if (!use_log)
		return 0;

	assert(log != NULL);

	va_list pars;
	va_start(pars, __format);
	int log_len = strlen(log);
	int res = vsnprintf(log + log_len, LOGLENGTH - log_len, __format, pars);
	va_end(pars);

	assert(res > 0);
	return res;
}

FLOAT PositifLayer::distance(Position a, Position b)
{
	FLOAT sumsqr = 0;
	for (unsigned int i = 0; i < nr_dims; i++) {
		sumsqr += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return sqrt(sumsqr);
}

char *PositifLayer::pos2str(Position a)
{
	static char str[100];

	switch (nr_dims) {
	case 2:
		sprintf(str, "<%.2f,%.2f>", a[0], a[1]);
		break;
	case 3:
		sprintf(str, "<%.2f,%.2f,%.2f>", a[0], a[1], a[2]);
		break;
	default:
		abort();
	}
	return str;
}

void PositifLayer::update_perf_data()
{
	memcpy(node[me].perf_data.curr_pos, position, sizeof(position));
	node[me].perf_data.flops = flops;
	node[me].perf_data.confidence = confidence;
	node[me].perf_data.status = status;
	node[me].perf_data.err = distance(position, node[me].true_pos) / range;
	node[me].perf_data.used_anchors = used_anchors;
	node[me].perf_data.range_list = range_list;
	node[me].perf_data.real_range_list = real_range_list;
}

/*******************************************************************************
 * MiXiM layering methods
 ******************************************************************************/
/**
 * Decapsulates the packet from the received Network packet 
 **/
cMessage *PositifLayer::decapsMsg(LocPkt * msg)
{
	cMessage *m = msg->decapsulate();

	cPolymorphic *cInfo = msg->removeControlInfo();
	if (cInfo != NULL) {
		m->setControlInfo(cInfo);
	}

	EV << " pkt decapsulated\n";

	return m;
}

/**
 * Encapsulates the received ApplPkt into a LocPkt and set all needed
 * header fields.
 **/
LocPkt *PositifLayer::encapsMsg(cMessage * msg)
{
	LocPkt *pkt = new LocPkt(msg->name(), msg->kind());
	pkt->setLength(headerLength);

	cPolymorphic *cInfo = msg->removeControlInfo();
	if (cInfo != NULL) {
		pkt->setControlInfo(cInfo);
	}
	//encapsulate the application packet
	pkt->encapsulate(msg);

	EV << " pkt encapsulated\n";
	return pkt;
}

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
void PositifLayer::handleLowerMsg(cMessage * msg)
{
	LocPkt *m = dynamic_cast < LocPkt * >(msg);

	/* Get the distance between the source and this node.
	 * This is cheated, and should be calculated based on ultrasone
	 * or RSSI. Currently the actual distance between the nodes is
	 * used. */
	int src = msg->par("src");
	double distance =
	    sqrt(square(node[me].true_pos[0] - node[src].true_pos[0]) +
		 square(node[me].true_pos[1] - node[src].true_pos[1]) +
		 square(node[me].true_pos[2] - node[src].true_pos[2]));
	msg->addPar("distance") = distance;

//      fprintf (stderr, "[%d] PositifLayer::handleLowerMsg() from %d\n",
//               me, src);

	node[me].recv_cnt++;
	/* If we received a LocPkt, this was received from
	 * upper layers. */
	if (m) {
		if (isNewNeighbor(src))
			addNewNeighbor(src, distance, distance);
		sendUp(decapsMsg(m));
	} else {
		switch (msg->kind()) {
		case MSG_NEIGHBOR:
//                      fprintf(stderr, "[%d] Received MSG_NEIGHBOR from [%d]\n", me, src);
			assert(isNewNeighbor(src));
			addNewNeighbor(src, distance, distance);
			break;
		default:
			{
				bool behind = !msg_buffering
				    && !putAside.empty();
				if (!behind) {
					bool new_neighbor =
					    isNewNeighbor(src);
					if (new_neighbor)
						addNewNeighbor(src, distance,
								distance);

					handleMessage(msg, new_neighbor);
//                                      // filter out redundant msg
//                                      int s = msg->par("seqno");
//                                      int kind = msg->kind();
//                                      assert(s >= neighbor->seqno[kind]);
//                                      // A message is new if it has a higher sequence number
//                                      if (s > neighbor->seqno[kind]) {
//                                              neighbor->seqno[kind] = s;
//                                              handleMessage(msg, new_neighbor);
//                                      }
				}
			}
		}
	}
	update_perf_data();
	delete msg;
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the PositifLayer we just use the destAddr of the network
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void PositifLayer::handleUpperMsg(cMessage * msg)
{
	sendDown(encapsMsg(msg));
}

/**
 * Redefine this function if you want to process control messages
 * from lower layers. 
 *
 * This function currently handles one messagetype: TRANSMISSION_OVER.
 * If such a message is received in the network layer it is deleted.
 * This is done as this type of messages is passed on by the BaseMacLayer.
 *
 * It may be used by network protocols to determine when the lower layers
 * are finished sending a message.
 **/
void PositifLayer::handleLowerControl(cMessage * msg)
{
	switch (msg->kind()) {
	default:
		opp_warning
		    ("PositifLayer does not handle control messages called %s",
		     msg->name());
		delete msg;
	}
}

// Functions that subclasses should implement.
// (They can't be abstract because of OMNET++
void PositifLayer::handleStopMessage(cMessage * msg)
{
	if (status == STATUS_POSITIONED) {
		fprintf(stderr, "Used anchors for node %d\n", me);
		for (int i = 0; i < used_anchors; i++)
			fprintf(stderr,
				"used_anchor: range %4.2f real range %4.2f error %4.2f rel error %4.2f\n",
				range_list[i], real_range_list[i],
				range_list[i] - real_range_list[i],
				(range_list[i] -
				 real_range_list[i]) / real_range_list[i]);
	}
}

/*******************************************************************************
 * Timer methods
 ******************************************************************************/
// timer_info *PositifLayer::timer(int reps, int handler, void *arg)
// {
//      timer_info *e = new timer_info;

//      e->repeats = e->cnt = reps;
//      e->handler = handler;
//      e->arg = arg;

//      return e;
// }

timer_info *PositifLayer::timer(int reps, int handler, void *arg)
{
	assert(handler < MAX_TIMERS);
	setRepeatTimer(handler, 1, reps);
	if (arg)
		setContextPointer(handler, arg);
	_timers[handler] = handler;
	return &_timers[handler];
}

// void PositifLayer::resetTimer(timer_info * e)
// {
//      if (e)
//              e->cnt = e->repeats;
// }

void PositifLayer::resetTimer(timer_info * e)
{
	if (e)
		resetRepeatTimer(*e);
}

// void PositifLayer::invokeTimer(timer_info * e)
// {
//      if (e) {
//              if (e->cnt > 0) {
//                      e->cnt--;
//                      handleTimer(e);
//              }
//      }
// }

void PositifLayer::cancelTimer(timer_info * e)
{
	cancelRepeatTimer(*e);
}

// addTimer isn't needed anymore, since timer() already adds it.
// void PositifLayer::addTimer(timer_info * e)
// {
//      if (e)
//              timeouts.insert(e);
// }

void PositifLayer::addTimer(timer_info * e)
{
	assert(e);
	assert(*e >= 0 && *e < MAX_TIMERS);
}

// iterator has to be done differently
// cLinkedListIterator PositifLayer::getTimers(void)
// {
//      return cLinkedListIterator(timeouts);
// }

/* Normally all repeatTimers are handled by the algorithm, but there's
 * one special message -start_timer- that is needed to initialize the
 * network. Therefor all localization modules must call:
 * - PositifLayer::handleRepeatTimer on the default case. */
void PositifLayer::handleRepeatTimer(unsigned int index)
{
	Enter_Method_Silent();

	if (index < 0) {
		error("Uninitialized timer called? aborting now...");
		abort();
	} else if (index == start_timer) {
//              fprintf(stderr, "[%d] received: %s\n", me, "START_TIMER");
		cMessage *msg = (cMessage *) contextPointer(index);
		assert(msg->kind() == MSG_START);
		status = msg->par("anchor") ? STATUS_ANCHOR : STATUS_UNKNOWN;
		// Anchors start with their position set
		if (status == STATUS_ANCHOR) {
			get_struct(msg, "position", position);
			confidence = 1.0;
		} else {
			for (unsigned int i = 0; i < nr_dims; i++)
				position[i] = 0.0;
			confidence = 0.0;
		}

		flops = 0;

#ifndef NDEBUG
		ev << node[me].
		    ID << ": starts at pos " << pos2str(position) << '\n';
#endif

		handleStartMessage(msg);
		deleteRepeatTimer(index);
		start_timer = -1;

		cMessage *neighborMsg =
		    new cMessage("NEIGHBOR", MSG_NEIGHBOR);
		send(neighborMsg);
	} else {
		assert(index < MAX_TIMERS);
		handleTimer(&_timers[index]);
	}
}

/*******************************************************************************
 * Communication methods
 ******************************************************************************/
void PositifLayer::send(cMessage * msg)	// Synchronous send
{
	int kind = msg->kind();

	// Add header
	msg->addPar("src") = me;
	msg->addPar("seqno") = seqno[kind];
	msg->setControlInfo(new NetwControlInfo(L3BROADCAST));

#ifndef NDEBUG
	ev << node[me].ID << ": broadcast " << msg->name();
//      if (kind == MSG_POSITION) {
//              ev << " confidence = " << msg->par("confidence");
//      }
	ev << " seqno = " << seqno[kind] << "\n";
#endif

	if (kind >= MSG_TYPE_BASE) {
		node[me].perf_data.bcast_total[kind]++;
		if (last_sent_seqno[kind] < seqno[kind]) {
			last_sent_seqno[kind] = seqno[kind];
			node[me].perf_data.bcast_unique[kind]++;
		}
	}
	sendDown(msg);

	return;
}

/*******************************************************************************
 * Triangulate methods
 ******************************************************************************/
FLOAT PositifLayer::savvides_minmax(unsigned int n_pts, FLOAT ** positions,
				    FLOAT * ranges, FLOAT * confs, int target)
{
	//
	//  Savvides' algorithm
	//
	Position min, max;

	// Find the max-min and min-max in each dimension.
	for (unsigned int i = 0; i < n_pts; i++)
		for (unsigned int j = 0; j < nr_dims; j++) {
			if (positions[i][j] - ranges[i] > min[j] || i == 0)
				min[j] = positions[i][j] - ranges[i];
			if (positions[i][j] + ranges[i] < max[j] || i == 0)
				max[j] = positions[i][j] + ranges[i];
		}

	// Store the result (avg of min and max)
	for (unsigned int i = 0; i < nr_dims; i++) {
		positions[n_pts][i] = (min[i] + max[i]) / 2;
	}

	FLOAT residu = 0;
	FLOAT sum_c = 0;
	for (unsigned int j = 0; j < n_pts; j++) {
		FLOAT c = (confs == NULL ? 1 : confs[j]);
		residu +=
		    c * fabs(ranges[j] -
			     distance(positions[n_pts], positions[j]));
		sum_c += c;
	}
	residu /= sum_c;

	return residu;
}

FLOAT PositifLayer::triangulate(unsigned int n_pts, FLOAT ** positions,
				FLOAT * ranges, FLOAT * confs, int target)
{
	FLOAT dop;

	if (n_pts <= nr_dims)
		return -1;

#ifndef NDEBUG
	if (confs != NULL) {
		ev << "confs:";
		for (unsigned int j = 0; j < n_pts; j++) {
			ev << " " << confs[j];
		}
		ev << "\n";
	}
#endif

	::params.dim = nr_dims;
	::params.alg_sel = 0;
	::params.conf_mets = par("use_confs") && (confs != NULL);
	if (!::triangulate(&::params, n_pts, positions, ranges, confs,
			   positions[n_pts], &dop)) {
#ifndef NDEBUG
		ev << ": FAILED TRIANGULATION\n";
#endif
		return -1;
	}
	// triangulate() used the last point to linearize the equations.
	// Use the estimated position for an extra equation so all inputs are
	// treated equally.
	ranges[n_pts] = 0;
	::triangulate(&::params, n_pts + 1, positions, ranges, confs,
		      positions[n_pts], &dop);
#ifndef NDEBUG
	ev << ": " << pos2str(positions[n_pts]);
#endif

	FLOAT residu = 0;
	FLOAT sum_c = 0;
	for (unsigned int j = 0; j < n_pts; j++) {
		FLOAT c = (confs == NULL ? 1 : confs[j]);
		residu +=
		    c * fabs(ranges[j] -
			     distance(positions[n_pts], positions[j]));
		sum_c += c;
	}
	residu /= sum_c;

#ifndef NDEBUG
	ev << " RESIDU = " << residu << " ERR = " <<
	    100 * distance(positions[n_pts],
			   node[target].true_pos) / range << "%\n";
#endif
	return residu;
}


FLOAT PositifLayer::hoptriangulate(unsigned int n_pts, FLOAT ** positions,
				   FLOAT * ranges, int target)
{
	FLOAT est_R;

	if (n_pts <= nr_dims + 1)
		return false;

	::params.dim = nr_dims;
	::params.alg_sel = 0;
	::params.conf_mets = false;
	if (!::hoptriangulate(&::params, n_pts, positions, ranges, NULL,
			      positions[n_pts], &est_R)) {
#ifndef NDEBUG
		ev << ": FAILED TRIANGULATION\n";
#endif
		return -1;
	}
#ifndef NDEBUG
	ev << ":A " << pos2str(positions[n_pts]);
#endif

	// Run sanity check
	for (unsigned int i = 0; i < n_pts; i++)
		ranges[i] *= est_R;
	FLOAT correction;
	::hoptriangulate(&::params, n_pts + 1, positions, ranges, NULL,
			 positions[n_pts], &correction);
#ifndef NDEBUG
	ev << ":B " << pos2str(positions[n_pts]);
#endif

	return (correction < .9 || correction > 1.1 ? -1 : correction * est_R);
}

/*******************************************************************************
 * Output statistics methods
 ******************************************************************************/
void PositifLayer::write_statistics()
{
	//////// START OF OUTPUT CODE /////////

	// Print this twice for convience. (Once more below)
	statistics(false);

	// Collect final positions
	int stopped = 0;
	cStdDev errs_phase1;
	cStdDev errs_phase2;
	cStdDev errs;
	cStdDev bad_node_pos_errs;
	cStdDev avg_flops;
	cStdDev avg_conf;
	cStdDev bcast_unique[MAX_MSG_TYPES];
	cStdDev bcast_total[MAX_MSG_TYPES];
	cStdDev bcast_sum_unique;
	cStdDev bcast_sum_total;
	cStdDev real_range_error;
	cStdDev real_abs_range_error;
	float anchor_range_error = 0;
	float rel_anchor_range_error = 0;
	float abs_anchor_range_error = 0;
	float abs_rel_anchor_range_error = 0;
	float anchor_range = 0;
	int anchor_range_error_count = 0;
	int nr_retries = 0;

	for (int src = 0; src < num_nodes; src++) {

		avg_flops += node[src].perf_data.flops;
		if (node[src].perf_data.phase1_err >= 0)
			errs_phase1 += node[src].perf_data.phase1_err;
		if (node[src].perf_data.phase2_err >= 0)
			errs_phase2 += node[src].perf_data.phase2_err;
		// Only measure the error and confidence for nodes that have a position
		if (node[src].perf_data.status == STATUS_POSITIONED) {
			avg_conf += node[src].perf_data.confidence;
			if (node[src].perf_data.err >= 0)
				errs += node[src].perf_data.err;
		} else if (node[src].perf_data.status == STATUS_BAD)
			bad_node_pos_errs += node[src].perf_data.err;


		// The number of retries is counted in the position of the START message, which is useless to count.
		nr_retries += node[src].perf_data.bcast_total[0];
		int bcast_total_tmp;
		int bcast_unique_tmp;
		bcast_total_tmp = 0;
		bcast_unique_tmp = 0;
		for (int i = MSG_TYPE_BASE; i < MAX_MSG_TYPES; i++) {
			bcast_unique[i] += node[src].perf_data.bcast_unique[i];
			bcast_total[i] += node[src].perf_data.bcast_total[i];
			bcast_unique_tmp += node[src].perf_data.bcast_unique[i];
			bcast_total_tmp += node[src].perf_data.bcast_total[i];
		}
		bcast_sum_unique += bcast_unique_tmp;
		bcast_sum_total += bcast_total_tmp;

		if (node[src].perf_data.status == STATUS_POSITIONED) {
			anchor_range_error +=
			    node[src].perf_data.anchor_range_error *
			    node[src].perf_data.anchor_range_error_count;
			rel_anchor_range_error +=
			    node[src].perf_data.rel_anchor_range_error *
			    node[src].perf_data.anchor_range_error_count;
			abs_anchor_range_error +=
			    node[src].perf_data.abs_anchor_range_error *
			    node[src].perf_data.anchor_range_error_count;
			abs_rel_anchor_range_error +=
			    node[src].perf_data.
			    abs_rel_anchor_range_error *
			    node[src].perf_data.anchor_range_error_count;
			for (int i = 0;
			     i < node[src].perf_data.used_anchors; i++) {
				real_range_error +=
				    (node[src].perf_data.range_list[i] -
				     node[src].perf_data.
				     real_range_list[i]) /
				    node[src].perf_data.real_range_list[i];
				real_abs_range_error +=
				    fabs(node[src].perf_data.
					 range_list[i] -
					 node[src].perf_data.
					 real_range_list[i]) /
				    node[src].perf_data.real_range_list[i];
			}
			anchor_range +=
			    node[src].perf_data.anchor_range *
			    node[src].perf_data.anchor_range_error_count;
			anchor_range_error_count +=
			    node[src].perf_data.anchor_range_error_count;
		}

		char finalposstr[20];	// Should be enough
		char trueposstr[20];	// Should be enough

		strcpy(finalposstr, pos2str(node[src].perf_data.curr_pos));
		strcpy(trueposstr, pos2str(node[src].true_pos));

		fprintf(stderr,
			"%3d: pos = %16s truepos = %16s %10s err = %6.2f%% (p1 %6.2f%% p2 %6.2f%%) conf = %.3f re= %3.1f %3.1f %3.1f\n",
			node[src].ID, finalposstr, trueposstr,
			status2string(node[src].perf_data.status),
			100 * distance(node[src].perf_data.curr_pos,
				       node[src].true_pos) / range,
			node[src].perf_data.phase1_err * 100,
			node[src].perf_data.phase2_err * 100,
			node[src].perf_data.confidence,
			node[src].perf_data.anchor_range_error,
			node[src].perf_data.abs_anchor_range_error,
			node[src].perf_data.anchor_range);

		stopped++;
	}

	if (errs_phase1.samples() > 0)
		fprintf(stderr,
			"avg 1st phase error (%d nodes): %4.1f%% (+/- %3.1f%%)\n",
			(int) errs_phase1.samples(), 100 * errs_phase1.mean(),
			100 * errs_phase1.stddev());
	if (errs_phase2.samples() > 0)
		fprintf(stderr,
			"avg 2nd phase error (%d nodes): %4.1f%% (+/- %3.1f%%)\n",
			(int) errs_phase2.samples(), 100 * errs_phase2.mean(),
			100 * errs_phase2.stddev());
	if (errs.samples() > 0)
		fprintf(stderr,
			"avg final error (%d nodes): %4.1f%% (+/- %3.1f%%)\n",
			(int) errs.samples(), 100 * errs.mean(),
			100 * errs.stddev());

	if (bad_node_pos_errs.samples() > 0)
		fprintf(stderr,
			"avg error of bad nodes (%d nodes): %4.1f%% (+/- %3.1f%%)\n",
			(int) bad_node_pos_errs.samples(),
			100 * bad_node_pos_errs.mean(),
			100 * bad_node_pos_errs.stddev());

	fprintf(stderr, "avg confidence: %.3f +/- %.3f\n",
		avg_conf.mean(), avg_conf.stddev());
	fprintf(stderr, "avg flops: %f +/- %.3f\n",
		avg_flops.mean(), avg_flops.stddev());

	statistics(false);

	fprintf(stderr, "\nNumber of RETRY messages: %d\n", nr_retries);
	fprintf(stderr,
		"Message type, bcasts/node, unique bcasts/node, total, total unique\n");
	for (int i = MSG_TYPE_BASE; i < MAX_MSG_TYPES; i++)
		if (bcast_total[i].sum() > 0)
			fprintf(stderr,
				"%8d, %7.1f (+/- %4.1f), %7.1f (+/- %4.1f), %8d, %8d\n",
				i, bcast_total[i].mean(),
				bcast_total[i].stddev(), bcast_unique[i].mean(),
				bcast_unique[i].stddev(),
				(int) bcast_sum_total.sum(),
				(int) bcast_sum_unique.sum());

	for (int n = 0; n < num_nodes; n++)
		if (strlen(node[n].perf_data.log) > 0)
			fprintf(stderr, "---%d\n%s\n", n,
				node[n].perf_data.log);

	//
	// Print raw data stuff
	//

	int count_anchor = 0;
	int count_unknown = 0;
	int count_positioned = 0;
	int count_bad = 0;
	//int flop_count=0;
	//int bcast_count=0;
	cStdDev connectivity;
	for (int i = 0; i < num_nodes; i++) {
		switch (node[i].perf_data.status) {
		case STATUS_ANCHOR:
			count_anchor++;
			break;
		case STATUS_UNKNOWN:
			count_unknown++;
			break;
		case STATUS_POSITIONED:
			count_positioned++;
			break;
		case STATUS_BAD:
			count_bad++;
			break;
		}
		connectivity += node[i].recv_cnt;
	}

	/*
	   %d %f %f
	   %d %f %f 
	   %d %f %f 
	   %d %f %f 
	   %d %f %f 
	   %f %f 
	   %f %f 
	   %f %f 
	   %f %f 
	   %d 
	   %d 
	   %d 
	   %d 
	   %d 
	 */
	fprintf(stderr,
		"random seed, algorithm, range, range variance, anch_frac, num_nodes, density, connectivity, final samples/mean err/stddev, 1st phase s/m/s, 2nd phase s/m/s, bad nodes s/m/s, conf m/s, flops m/s, bcast tot m/s, bcast uniq m/s, bcast t5 m/s, t6 m/s, t7 m/s, t8 m/s, t9 m/s, retries, count ANC/UNK/POS/BAD, algorithm version, do_2nd_phase, phase1_anchors, topology\n");
	fprintf(stderr,
		"RAWDATA %d %f %f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %d %d %d %d %d %d %d %d %d %s %f %f %f %f %f %f %f %f %f %f %d %f %f %f %f %d\n",
		// Parameters and derived statistics
		algorithm,
		range,
		range_variance,
		(double) num_anchors / (double) num_nodes,
		num_nodes, num_nodes / area, connectivity.mean(),
		// Sim results
		(int) errs.samples(), 100 * errs.mean(), 100 * errs.stddev(),
		(int) errs_phase1.samples(), 100 * errs_phase1.mean(),
		100 * errs_phase1.stddev(), (int) errs_phase2.samples(),
		100 * errs_phase2.mean(), 100 * errs_phase2.stddev(),
		(int) bad_node_pos_errs.samples(),
		100 * bad_node_pos_errs.mean(),
		100 * bad_node_pos_errs.stddev(), avg_conf.mean(),
		avg_conf.stddev(), avg_flops.mean(), avg_flops.stddev(),
		bcast_sum_total.mean(), bcast_sum_total.stddev(),
		bcast_sum_unique.mean(), bcast_sum_unique.stddev(),
		bcast_total[5].mean(), bcast_total[5].stddev(),
		bcast_total[6].mean(), bcast_total[6].stddev(),
		bcast_total[7].mean(), bcast_total[7].stddev(),
		bcast_total[8].mean(), bcast_total[8].stddev(),
		bcast_total[9].mean(), bcast_total[9].stddev(), nr_retries,
		count_anchor, count_unknown, count_positioned, count_bad,
		version, do_2nd_phase ? 1 : 0, phase1_min_anchors,
		phase1_max_anchors, flood_limit, topology_type, pos_variance,
		var0, var1, var2,
		(double) count_positioned / (double) (count_unknown +
						      count_positioned +
						      count_bad),
		anchor_range_error / anchor_range_error_count,
		rel_anchor_range_error / anchor_range_error_count,
		abs_anchor_range_error / anchor_range_error_count,
		abs_rel_anchor_range_error / anchor_range_error_count,
		anchor_range / anchor_range_error_count, tri_alg,
		real_range_error.mean(), real_range_error.stddev(),
		real_abs_range_error.mean(), real_abs_range_error.stddev(),
		refine_limit);
}

void PositifLayer::statistics(bool heading)
{
	cStdDev connectivity;
	cStdDev pos_errs;
	cStdDev avg_conf;
	int count_anchor = 0;
	int count_unknown = 0;
	int count_positioned = 0;
	int count_bad = 0;
	int flop_count = 0;
	int bcast_count = 0;

	// collect statistics
	for (int i = 0; i < num_nodes; i++) {
		// Only measure confidence and error for nodes with a position
		if (node[i].perf_data.status == STATUS_POSITIONED) {
			pos_errs +=
			    distance(node[i].true_pos,
				     node[i].perf_data.curr_pos);
			avg_conf += node[i].perf_data.confidence;
		}

		connectivity += node[i].recv_cnt;
		flop_count += node[i].perf_data.flops;
		switch (node[i].perf_data.status) {
		case STATUS_ANCHOR:
			count_anchor++;
			break;
		case STATUS_UNKNOWN:
			count_unknown++;
			break;
		case STATUS_POSITIONED:
			count_positioned++;
			break;
		case STATUS_BAD:
			count_bad++;
			break;
		}
		for (int j = MSG_TYPE_BASE; j < MAX_MSG_TYPES; j++)
			bcast_count += node[i].perf_data.bcast_total[j];
	}

	if (heading) {
		fprintf(stderr, "\nSTATISTICS\n");
		fprintf(stderr, "  #nodes      : %d\n", num_nodes);
		fprintf(stderr, "  density     : %.2f /m^%d\n",
			num_nodes / area, nr_dims);
		fprintf(stderr, "  #anchors    : %d (%d%%)\n", num_anchors,
			(100 * num_anchors) / num_nodes);
		fprintf(stderr, "  radio range : %g m\n", range);
		fprintf(stderr, "  connectivity: %.2f +/- %.2f\n",
			connectivity.mean(), connectivity.stddev());
	}

	if (pos_errs.samples() > 0) {
		fprintf(stderr, "t = %4d #bcast %6d, ", (int) simTime(),
			bcast_count);
		fprintf(stderr, " #flops %6d, ", flop_count);
		fprintf(stderr, " ANC%4d, UNK%4d, POS%4d, BAD%4d, TOT%4d, ",
			count_anchor, count_unknown, count_positioned,
			count_bad, num_nodes);
		fprintf(stderr, "err (/R): %6.2f%% +/- %5.2f, ",
			100 * pos_errs.mean() / range,
			100 * pos_errs.stddev() / range);
		fprintf(stderr, "conf: %1.3f +/- %1.3f\n", avg_conf.mean(),
			avg_conf.stddev());
	}
}
