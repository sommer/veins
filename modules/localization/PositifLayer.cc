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

Define_Module(PositifLayer);

int PositifLayer::num_nodes;
int PositifLayer::num_anchors;
int PositifLayer::algorithm;
int PositifLayer::version;
unsigned int PositifLayer::nr_dims;
node_info *PositifLayer::node = NULL;
FLOAT PositifLayer::range;
FLOAT PositifLayer::area;
double *PositifLayer::dim;
double PositifLayer::range_variance;
int PositifLayer::flood_limit;
int PositifLayer::refine_limit;
int PositifLayer::tri_alg;
bool PositifLayer::do_2nd_phase;
int PositifLayer::phase1_min_anchors;
int PositifLayer::phase1_max_anchors;

struct myParams params;

void PositifLayer::initialize(int stage)
{
	BaseLayer::initialize(stage);

	switch (stage) {
	case 0:
		RepeatTimer::init(this);
		me = findHost()->index();
		/* clear arrays */
		for (int i = 0; i < MAX_MSG_TYPES; i++) {
			seqno[i] = last_sent_seqno[i] = 0;
		}
		/* initialize pointers */
		range_list = NULL;
		real_range_list = NULL;
		log = NULL;

		reps = 1;
		/* only one node needs to initialize this */
		if (me == 0) {
			setGlobalVars();
			node = new node_info[num_nodes];
		}
		break;
	case 1:
		headerLength = par("headerLength");

		/* initialize the node specific stuff */
		node[me].neighbors = new cLinkedList();
		node[me].perf_data.bcast_total = new int[MAX_MSG_TYPES];
		node[me].perf_data.bcast_unique = new int[MAX_MSG_TYPES];
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

		getPosition();
		break;
	case 2:
		init();
		handleStartMessage(NULL);
		break;
	default:
		break;
	}
}

void PositifLayer::finish()
{
	handleStopMessage(NULL);
	delete node[me].neighbors;
}

Coord PositifLayer::getPosition()
{
	BaseUtility *util =
	    FindModule <BaseUtility *>::findSubModule(findHost());
	const Coord *coord = util->getPos();
	/* convert from Coord to Position */
	node[me].true_pos[0] = position[0] = coord->x;
	node[me].true_pos[1] = position[1] = coord->y;
	node[me].true_pos[2] = position[2] = coord->z;

	return *coord;
}

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

static double square(double x) { return x * x; }

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
	double distance = sqrt(square(node[me].true_pos[0] - node[src].true_pos[0]) +
			       square(node[me].true_pos[1] - node[src].true_pos[1]) +
			       square(node[me].true_pos[2] - node[src].true_pos[2]));
	msg->addPar("distance") = distance;
	/* If we received a LocPkt, this was received from
	 * upper layers. */
	if (m) {
		sendUp(decapsMsg(m));
	} else {
		handleMessage(msg, true);
	}
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

unsigned int PositifLayer::timer(int reps, int handler, void *arg)
{
	setRepeatTimer(handler, 1, reps);
	return handler;
}

// timer_info *PositifLayer::timer(int reps, int handler, void *arg)
// {
//      timer_info *e = new timer_info;

//      e->repeats = e->cnt = reps;
//      e->handler = handler;
//      e->arg = arg;

//      return e;
// }

int PositifLayer::logprintf(__const char *__restrict __format, ...)
{
	if (!use_log)
		return 0;

	va_list pars;
	va_start(pars, __format);
	int res =
	    vsnprintf(log + strlen(log), LOGLENGTH - strlen(log), __format,
		      pars);
	va_end(pars);

	return res;
}

void PositifLayer::resetTimer(unsigned int index)
{
	resetRepeatTimer(index);
}

// void PositifLayer::resetTimer(timer_info * e)
// {
//      if (e)
//              e->cnt = e->repeats;
// }

// void PositifLayer::invokeTimer(timer_info * e)
// {
//      if (e) {
//              if (e->cnt > 0) {
//                      e->cnt--;
//                      handleTimer(e);
//              }
//      }
// }

// addTimer isn't needed anymore, since timer() already adds it.
// void PositifLayer::addTimer(timer_info * e)
// {
//      if (e)
//              timeouts.insert(e);
// }

// iterator has to be done differently
// cLinkedListIterator PositifLayer::getTimers(void)
// {
//      return cLinkedListIterator(timeouts);
// }

void PositifLayer::send(cMessage * msg)	// Synchronous send
{
	int kind = msg->kind();

	// Add header
	msg->addPar("src") = me;
	msg->addPar("seqno") = seqno[kind];

#ifndef NDEBUG
	ev << node[me].ID << ": broadcast " << msg->name();
// 	if (kind == MSG_POSITION) {
// 		ev << " confidence = " << msg->par("confidence");
// 	}
	ev << " seqno = " << seqno[kind] << "\n";
#endif

	if (kind >= MSG_TYPE_BASE) {
		node[me].perf_data.bcast_total[kind]++;
		if (last_sent_seqno[kind] < seqno[kind]) {
			last_sent_seqno[kind] = seqno[kind];
			node[me].perf_data.bcast_unique[kind]++;
		}
	}
//      Triangulate::send(msg, "out");
	sendDown(msg);

	return;

// The code below isn't needed in MiXiM, as this is basically what the network and
// MAC layer do.

// 	if (kind != MSG_DONE) {	// Retry stuff isn't necessary for the last message.
// //              msg = receiveOnGate("status");

// 		if (msg->kind() == MSG_RETRY) {
// 			// retry once
// 			node[me].perf_data.bcast_total[0]++;	// Track the number of retries in the position of the START message, which is useless to count anyway.
// 			msg->setKind(kind);
// 			if (kind >= MSG_TYPE_BASE)
// 				node[me].perf_data.bcast_total[kind]++;
// //                      Triangulate::send(msg, "out");
// //                      msg = receiveOnGate("status");
// 		}

// 		delete msg;
// 	}
}

void PositifLayer::setGlobalVars(void)
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

	num_nodes = par("num_nodes");
	num_anchors =
	    (int) Max(num_nodes * (double) par("anchor_frac"), nr_dims + 1);
	range = (double) par("range");
}

void PositifLayer::DoNeighbours()
{
	if (par("gen_points_only")) {
		for (unsigned int d = 0; d < nr_dims; d++)
			ev << "Range " << d << " , " << dim[d] << "\n";
		ev << "Anchors " << num_anchors << "\n";
		//Global::Activity();

		exit(0);
	}
	// Determine neighbours based on radio range
	for (int i = 0; i < num_nodes; i++) {
		neighbor_info *tail = NULL;

		for (int j = 0; j < num_nodes; j++) {
			FLOAT true_d =
			    distance(node[i].true_pos, node[j].true_pos);
			FLOAT est_d =
			    genk_truncnormal(1, true_d, range * range_variance);
			//FLOAT est_d = truncnormal( true_d, range*range_variance);

			if (i != j && true_d <= range) {
				neighbor_info *n = new neighbor_info;

				n->idx = j;
				n->true_dist = true_d;
				if (est_d > range)
					est_d = range;
				n->est_dist = est_d;

				if (tail != NULL) {
					node[i].neighbors->insertAfter(tail, n);
				} else {
					node[i].neighbors->insert(n);
				}
				tail = n;

				node[j].recv_cnt++;
			}
		}
	}
}

FLOAT PositifLayer::distance(Position a, Position b)
{
	FLOAT sumsqr = 0;

	for (unsigned int c = 0; c < nr_dims; c++) {
		sumsqr += (a[c] - b[c]) * (a[c] - b[c]);
	}
	return sqrt(sumsqr);
}

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

	//FLOAT area_size = 0;
	// Store the result (avg of min and max)
	for (unsigned int i = 0; i < nr_dims; i++) {
		positions[n_pts][i] = (min[i] + max[i]) / 2;

		/*  if( i==0 )
		   area_size=max[i]-min[i];
		   else
		   area_size*=max[i]-min[i]; */
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

