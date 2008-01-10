/* -*- mode:c++ -*- ********************************************************
 * file:        Savvides.h
 *
 * author:      Peterpaul Klein Haneveld <pp_kl_h@hotmail.com>
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
 * description: savvides class for the localization module
 **************************************************************************/

/*
 * ANCHOR_MSG and ANCHOR_TIMER reflect the first phase of the algorithm:
 * collecting anchor information.
 *
 * POSITION_MSG and POSITION_TIMER reflect the second phase of the algorithm:
 * 
 */
#include "Savvides.h"
#include "cores.h"

#include <NetwControlInfo.h>
#include <SimpleAddress.h>
#include <ConnectionManager.h>

int Savvides::refine_limit = 0;
int Savvides::flood_limit = 1;
int Savvides::repeats = 3;
int Savvides::tri_alg = 0;
int Savvides::phase1_min_anchors = 0;
int Savvides::phase1_max_anchors = -1;
double Savvides::range;
double Savvides::accuracy = 0.1;
double Savvides::anchor_timer_interval;
double Savvides::position_timer_interval;
double Savvides::triangulation_timer_interval;

//Define_Module_Like(Savvides, BaseLocalization);
Define_Module(Savvides)

static bool AnchorSortPredicate(const AnchorInfo* a, const AnchorInfo* b)
{
	return a->path_dst < b->path_dst;
}

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

#define BRANCH_INIT(p) BRANCH_INIT2(p,p)
#define BRANCH_INIT2(p,d) (hasPar(#p)?(EV << "using par: " << #p << " = " << par(#p) << endl, par(#p)):d)

/* Initialization and Finalization of the layer */
void Savvides::initialize(int stage)
{
	BaseLocalization::initialize(stage);

	switch (stage) {
	case 0:
		Timer::init(this);

		valid_rectangle = false;

		if (id == 0) {
			refine_limit = BRANCH_INIT(refine_limit);
			flood_limit = BRANCH_INIT(flood_limit);
			repeats = BRANCH_INIT(repeats);
			tri_alg = BRANCH_INIT(tri_alg);
			accuracy = BRANCH_INIT(accuracy);
// 			phase1_min_anchors = BRANCH_INIT2(phase1_min_anchors,getPosition().is2D() ? 3 : 4);
			phase1_min_anchors = BRANCH_INIT(phase1_min_anchors);
			phase1_max_anchors = BRANCH_INIT(phase1_max_anchors);

			/** anchor_timer_interval must be smaller than
			 * the update interval for moves. Otherwise incorrect
			 * distance information is stored.
			 */
			anchor_timer_interval = 0.5;
			position_timer_interval = 2;
			triangulation_timer_interval = 4;
		}

		used_anchors = 0;

		range_list = NULL;
		real_range_list = NULL;
		break;
	case 1:
		nr_dims = (worldUtility->use2D()?2:3);
		range = (double)(FindModule<BaseConnectionManager*>::findGlobalModule())->par("radioRange");

		if (isAnchor) {
			status = STATUS_ANCHOR;
			AnchorInfo *anchor = new AnchorInfo(id, true, getLocation(), 0.0);
			anchor->path_dst = 0.0;
			anchor->cnt = repeats;
			anchor->flood = true;
			anchor->last_hop_idx = 0;
			PUSH_BACK(anc,anchor);
			setTimer(ANCHOR_TIMER, anchor_timer_interval);
		} else {
			status = STATUS_UNKNOWN;
		}

		// subscribe to move
		moveCategory = utility->subscribe(this, &move);
		break;
	}
}

void Savvides::handleTimer(unsigned int index)
{
	switch (index) {
	case ANCHOR_TIMER:
		/* The anchor timer fired, it's time to send our list of
		 * anchors. */
		sendAnchors();
		break;
	case POSITION_TIMER:
		/* The position timer fired, it's time to perform a
		 * refinement */
		sendPosition();
		break;
	case TRIANGULATION_TIMER:
		doTriangulation();
		break;
	default:
		delayedDoTriangulation(index, contextPointer(index));
	}
}

void Savvides::sendAnchors()
{
	cMessage *msg = new cMessage("ANCHOR_MSG", ANCHOR_MSG);

	int num_anchors = 0;
	char parname[16] = "";

	for (list<AnchorInfo *>::iterator iter = anc.begin();
	     iter != anc.end();
	     iter++) {
		AnchorInfo *anchor = (*iter);
		if (anchor->cnt != 0 && anchor->flood) {
			sprintf(parname, "anchor_%d", num_anchors);
			add_struct(msg, parname, anchor);
			num_anchors++;
			anchor->cnt--;
		}
	}
	msg->addPar("num_anchors") = num_anchors;

	if (num_anchors > 0)
		sendMsg(msg);
	else
		delete msg;
}

void Savvides::sendPosition()
{
	if (!isAnchor && neighbors.size() <= (unsigned)nr_dims) {
		/* check if i'm a sound node */
		if (anc.size() + neighbors.size() <= (unsigned)nr_dims)
			return;
	}

	if (refine_limit > -1 && refine_count >= refine_limit)
		return;
	refine_count ++;

	cMessage *msg = new cMessage("POSITION_MSG", POSITION_MSG);
	msg->addPar("confidence") = pos.getConfidence();
	add_struct(msg, "pos", &pos);

	if (summary_update) {
		msg->addPar("#neighbors") = summary.nr_nghbrs;
		add_array(msg, "id", summary.nghbr_idx, summary.nr_nghbrs);
		summary_update = false;
	} else {
		msg->addPar("#neighbors") = 0;
	}

	sendMsg(msg);
}

void Savvides::finish()
{
	EV << "Savvides::finish()" 
	   << status2string(status)
	   << " pos "
	   << getLocationEstimation().info()
	   << " realpos "
	   << getPosition().info()
	   << endl;

	EV << "Savvides ERROR: " << getLocationEstimation().getError(getPosition(), range) << " %" << endl;

	while (anc.begin() != anc.end()) {
		AnchorInfo *anchor = *anc.begin();
// 		EV_clear << "\t" 
// 			 << anchor->info()
// 			 << " at "
// 			 << anchor->pos.getTimestamp()
// 			 << " s"
// 			 << endl;
		anc.erase(anc.begin());
		delete anchor;
	}

// 	while (nb.begin() != nb.end()) {
// 		NeighborInfo *neighbor = *nb.begin();
// 		EV_clear << "\t" << neighbor->id <<
// 			neighbor->pos.info() <<
// 			neighbor->pos.getTimestamp() << endl;
// 		nb.erase(nb.begin());
// 		delete neighbor;
// 	}

	if (range_list)
		delete range_list;
	if (real_range_list)
		delete real_range_list;

	debug = false;
	BaseLocalization::finish();
	debug = true;
}

NodeInfo * Savvides::handleNewAnchor(NodeInfo * _anchor)
{
	AnchorInfo *anchor = new AnchorInfo(_anchor);
	anchor->last_hop_idx = anchor->id;
	anchor->path_dst = anchor->distance;
	anchor->cnt = repeats;
	anchor->flood = true;
	PUSH_BACK(anc,anchor);
	/* A new anchor is received, broadcast list of anchors. */
	if ((int)anc.size() < flood_limit || flood_limit == -1)
		anchor->flood = true;
	else
		anchor->flood = false;
	setTimer(ANCHOR_TIMER, anchor_timer_interval);
	if (!isAnchor) {
		update_rectangle(anchor);
		savvides();
	}
	setTimer(POSITION_TIMER, position_timer_interval);
	return _anchor;
}

/** An ANCHOR_MSG is received, check the contents of this message. */
bool Savvides::checkAnchors(cMessage * msg, NodeInfo * node)
{
	int num_anchors = msg->par("num_anchors");
	double distance = node->distance;
	char parname[16] = "";
	bool changed = false;

	EV << "checkAnchors():distance = " << distance << endl;

	EV << id << " checking for new anchors in list of " << num_anchors << " anchors" << endl;

	for (int i = 0; i < num_anchors; i++) {
		AnchorInfo *anchor = new AnchorInfo;
		sprintf(parname, "anchor_%d", i);
		get_struct(msg, parname, anchor);
		anchor->path_dst += distance;
		EV << anchor->info() << " -- anchor to check - ";
		if (anchor->id == id) {
			EV_clear << "no new anchor(me)" << endl;
			delete anchor;
			continue;
		}
		// check if we received this one before
		bool found = false;
		AnchorInfo * old_anchor = NULL;
		for (list<AnchorInfo *>::iterator iter = anc.begin();
		     iter != anc.end();
		     iter++) {
			old_anchor = (*iter);
			if (anchor->id == old_anchor->id) {
				/* Let anchors be differentiated by their
				   position */
				EV << anchor->pos.info() << "<>" 
				   << old_anchor->pos.info() << " ";
				if (anchor->pos == old_anchor->pos) {
					found = true;
					break;
				}
			}
		}
		if (!found) {
			if ((int)anc.size() < flood_limit || flood_limit == -1)
				anchor->flood = true;
			else
				anchor->flood = false;
			anchor->last_hop_idx = node->id;
			anchor->cnt = repeats;
			changed = true;
			EV_clear << "NEW ANCHOR:" << anchor->info() << endl;
			PUSH_BACK(anc,anchor);
			setTimer(ANCHOR_TIMER, anchor_timer_interval);
			update_rectangle(anchor);
		} else {
			EV_clear << "no new anchor" << endl;
			/* Don't update the path when last_hop_idx equals the anchor node. */
			if (
				old_anchor->last_hop_idx != old_anchor->id
				&&
				anchor->last_hop_idx != old_anchor->last_hop_idx
				&&
				anchor->path_dst < old_anchor->path_dst
				) 
			{	// Found it, but the new one has a shorter path.
// 				old_anchor->path_dst = anchor->path_dst;
// 				old_anchor->last_hop_idx = node->id;
// 				old_anchor->cnt = repeats;
// 				EV << "anchor UPDATED: " << old_anchor->info() << endl;
// 				setTimer(ANCHOR_TIMER, anchor_timer_interval);
// 				changed = true;
// 				update_rectangle(anchor);
			}
			delete anchor;
		}
	}
	return changed;
}

NodeInfo * Savvides::handleNewNeighbor(NodeInfo * _neighbor)
{
// 	NeighborInfo *neighbor = new NeighborInfo(_neighbor);
	delete _neighbor;
	return NULL;
// 	return neighbor;
}

void Savvides::sendMsg(cMessage * msg)
{
	msg->setControlInfo(new NetwControlInfo(L3BROADCAST));

	sendDown(encapsMsg(msg, msg->kind()));
}

void Savvides::handleMsg(cMessage * msg)
{
	LocPkt * pkt = static_cast<LocPkt *>(msg);
	if (pkt == NULL) {
		error("Savvides::handleMsg() should always get a LocPkt.");
	}
	NodeInfo * node = new NodeInfo(pkt, getPosition());
	cMessage * m = decapsMsg(msg);

	switch (m->kind()) {
	case ANCHOR_MSG: 
		if (checkAnchors(m, node) && !isAnchor) {
			savvides();
			setTimer(POSITION_TIMER, position_timer_interval);
		}
		break;
	case POSITION_MSG:
		if (!isAnchor) {
			checkNeighbors(m, node);

			if (neighbors.size() > (unsigned)nr_dims) {
				/* prospone triangulation to receive more neighbors */
				setTimer(TRIANGULATION_TIMER, triangulation_timer_interval);
			}
		}
		break;
	default:
		error("Unknown message type %d", msg->kind());
	}
	delete m;
	delete node;
}

void Savvides::update_rectangle(AnchorInfo * anchor)
{
	Coord path_dst = Coord(anchor->path_dst, 
			       anchor->path_dst, 
			       anchor->path_dst);
	Coord left = anchor->pos - path_dst;
	Coord right = anchor->pos + path_dst;
	// intersect anchor's "square" with existing convex rectangle
	if (!valid_rectangle) {
		rectangle.min = left;
		rectangle.max = right;
		valid_rectangle = true;
	} else {
		rectangle.min = rectangle.min.min(left);
		rectangle.max = rectangle.max.max(right);
	}
}


bool Savvides::inside_rectangle(Coord pos)
{
	assert(valid_rectangle);
	return pos > rectangle.min && pos < rectangle.max;
}

bool Savvides::inside_neighbors_range(Coord pos)
{
	for (list<NodeInfo *>::iterator iter = neighbors.begin();
	     iter != neighbors.end(); iter++) {
		NeighborInfo *neighbor = static_cast<NeighborInfo *>(*iter);

		if (neighbor->pos.getConfidence() > 2 * LOW_CONF &&
		    pos.distance(neighbor->pos) > range) {
			return false;
		}
	}
	return true;
}

//
// Two algorithms are executed: the first is Savvides_Mob' initial estimation
// phase, which creates a bounding box using the ranges from the anchors
// and takes the center of this box as position estimate. (phase1_err)
// The second takes a weighted average of all anchors. (phase2_err)
// Which value is used in the end is determined by the constant PHASE1
//
void Savvides::savvides(void)
{
	int n = anc.size();

	if (n >= phase1_min_anchors) {

		int i;
#ifndef NDEBUG
		EV << id << ": savvides' phase2 initialization with anchors:";
#endif
		/* used_anchors is limited by phase1_max_anchors */
		used_anchors = ((n < phase1_max_anchors) ||
				(phase1_max_anchors == -1)) ? n : phase1_max_anchors;
		FLOAT** pos_list = new FLOAT*[used_anchors + 1];

		if (range_list)
			delete range_list;
		range_list = new FLOAT[used_anchors + 1];
		if (real_range_list)
			delete real_range_list;
		real_range_list = new FLOAT[used_anchors + 1];	// Used for debug output only
		int* idx_list = new int[used_anchors];

		anc.sort(AnchorSortPredicate);

		i = 0;
		for (list<AnchorInfo *>::iterator iter = anc.begin();
		     iter != anc.end();
		     iter++) {
			AnchorInfo *anchor = (*iter);
#ifndef NDEBUG
			EV << " " << anchor->id << "@" << anchor->cnt;
#endif
			if (i < used_anchors) {
				pos_list[i] = coordToPosition(anchor->pos);
				range_list[i] = anchor->path_dst;
				real_range_list[i] = getPosition().distance(anchor);
				idx_list[i] = anchor->id;
			}
			i++;
		}

		Position next;
		pos_list[used_anchors] = next;

		// Calculate performance data for range error
// 		FLOAT sum_err = 0, sum_rel_err = 0, sum_abs_err =
// 		    0, sum_abs_rel_err = 0, sum_range = 0;
// 		for (i = 0; i < used_anchors; i++) {
// 			sum_err += range_list[i] - real_range_list[i];
// 			sum_rel_err += (range_list[i] - real_range_list[i]) / real_range_list[i];
// 			sum_abs_err += fabs(range_list[i] - real_range_list[i]);
// 			sum_abs_rel_err += fabs(range_list[i] - real_range_list[i]) / real_range_list[i];
// 			sum_range += range_list[i];
// 		}
// 		perf_data.anchor_range_error = sum_err / used_anchors;
// 		perf_data.rel_anchor_range_error = sum_rel_err / used_anchors;
// 		perf_data.abs_anchor_range_error = sum_abs_err / used_anchors;
// 		perf_data.abs_rel_anchor_range_error = sum_abs_rel_err / used_anchors;
// 		perf_data.anchor_range = sum_range / used_anchors;
// 		perf_data.anchor_range_error_count = used_anchors;

		FLOAT res;
		if (tri_alg == 0)
			res = triangulate(used_anchors, pos_list, range_list, NULL, id);
		else
			res = savvides_minmax(used_anchors, pos_list, range_list, NULL, id);

//              wait(n * nr_dims * nr_dims * msec);

		if (0 <= res && res <= range) {
			fillCoordFromPosition(&pos, next, nr_dims);
// 			perf_data.phase1_err = getPosition().distance(pos) / range;

			pos.setConfidence(LOW_CONF);
			status = STATUS_POSITIONED;	// After Savvides initialisation, a node is positioned regardless of its confidence.

			setTimer(POSITION_TIMER, position_timer_interval);
		}

		delete[] idx_list;
		for (i = 0; i < used_anchors; i++) {
			if (pos_list[i])
				delete pos_list[i];
		}
		delete[] pos_list;
	}
}

void Savvides::checkNeighbors(cMessage * msg, NodeInfo * node)
{
	NeighborInfo *neighbor = NULL;
	int src = node->id;

	bool found = false;
	for (list<NodeInfo *>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++) {
		neighbor = static_cast<NeighborInfo *>(*iter);

		if (neighbor->id == src) {
			found = true;
			break;
		}
	}

	if (!found) {
		neighbor = new NeighborInfo(node);
		PUSH_BACK(neighbors, neighbor);

		// Update summary
		int nb_size = neighbors.size();
		assert(summary.nr_nghbrs == nb_size - 1);

		if (summary.nr_nghbrs > 0) {
			delete summary.nghbr_idx;
		}
		int n = ++summary.nr_nghbrs;
		summary.nghbr_idx = new int[n];
		int i = 0;
		for (list<NodeInfo *>::iterator iter = neighbors.begin();
		     iter != neighbors.end(); iter++) {
			summary.nghbr_idx[i++] = (*iter)->id;
		}
		summary_update = true;

		// The summary is piggybacked on ordinary MSG_POSITION msgs, so
		// reset the associated timer if that has already been activated
		setTimer(POSITION_TIMER, position_timer_interval);
	}
	get_struct(msg, "pos", &neighbor->pos);
	neighbor->distance = node->distance;
	if (!neighbor->twin)
		neighbor->pos.setConfidence(msg->par("confidence"));

	int n = msg->par("#neighbors");
	if (n > 0) {
		if (neighbor->nr_nghbrs > 0) {
			delete neighbor->nghbr_idx;
		}
		neighbor->nr_nghbrs = n;
		neighbor->nghbr_idx = new int[n];
		get_array(msg, "id", neighbor->nghbr_idx, n);

		// Update may introduce new twins and/or remove old twins
		// Simply check all pairs for twins (updating is too difficult)
		for (list<NodeInfo *>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++) {
			NeighborInfo *m = static_cast<NeighborInfo *>(*iter);
			m->twin = false;
		}
		for (list<NodeInfo *>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++) {
			NeighborInfo *m = static_cast<NeighborInfo *>(*iter);

			// Skip anchors
			if (m->pos.getConfidence() == 1) {
				continue;
			}

			if (twins(m, &summary)) {
				i_am_a_twin = m->twin = true;
			}

			for (list<NodeInfo *>::iterator iter = neighbors.begin(); iter != neighbors.end();
			     iter++) {
				NeighborInfo *k = static_cast<NeighborInfo *>(*iter);

				// Skip anchors
				if (k->pos.getConfidence() == 1) {
					continue;
				}

				if (m->id != k->id && twins(m, k)) {
					m->twin = k->twin = true;
				}
			}
		}
	}
}

bool Savvides::twins(NeighborInfo * a, NeighborInfo * b)
{
	if (a->nr_nghbrs != b->nr_nghbrs)
		return false;

	int num_nodes = 0;
	for (int i = 0; i < a->nr_nghbrs; i++) {
		if (a->nghbr_idx[i] >=num_nodes) {
			num_nodes = a->nghbr_idx[i] + 1;
		}
	}

	bool* nghbr_a = new bool[num_nodes];

	for (int i = 0; i < num_nodes; i++) {
		nghbr_a[i] = false;
	}
	for (int i = 0; i < a->nr_nghbrs; i++) {
		nghbr_a[a->nghbr_idx[i]] = true;
	}

	if (!nghbr_a[b->id]) {
		delete[] nghbr_a;
		return false;
	}

	for (int i = 0; i < b->nr_nghbrs; i++) {
		int n = b->nghbr_idx[i];

		if (n != a->id && !nghbr_a[n]) {
			delete[] nghbr_a;		
			return false;
		}
	}
	delete[] nghbr_a;	
	return true;
}

static void delete_tria_data (void * _data) {
	tria_data * data = (tria_data *)_data;
	if (data)
		delete data;
}

void Savvides::doTriangulation()
{
	int n = neighbors.size();
	assert(n > nr_dims);
	FLOAT** pos_list = new FLOAT*[n + 1];
	FLOAT* range_list = new FLOAT[n + 1];
	FLOAT* weights = new FLOAT[n];
	Position pos;

#ifndef NDEBUG
	EV << id << ": triangulate with nodes (out of " << n << "):";
#endif
	int i = 0;
	FLOAT sum_conf = 0;
	for (list<NodeInfo *>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++) {
		NeighborInfo *neighbor = static_cast<NeighborInfo *>(*iter);
		double w = neighbor->twin ? LOW_CONF / 8 : neighbor->pos.getConfidence();

		pos_list[i] = coordToPosition(neighbor->pos);
		range_list[i] = neighbor->distance;
		weights[i] = w;
		sum_conf += w;
		i++;
#ifndef NDEBUG
		EV << ' ' << neighbor->id << (neighbor->
					      twin ? "t" : "") << '@'
		    << neighbor->distance;
#endif
	}
#ifndef NDEBUG
	EV << "\n";
#endif

	pos_list[i] = pos;
	FLOAT res = triangulate(i, pos_list, range_list, weights, id);

	tria_data * data = new tria_data;
	data->res = res;
	data->sum_conf = sum_conf;
	data->neighbor_count = i;
	data->pos = positionToCoord(pos, nr_dims);
	unsigned int timer = setTimer(i * nr_dims * nr_dims * msec);
	triangulation_timers.push_back(timer);
	setContextPointer(timer, data);
	setContextDestructor(timer, delete_tria_data);

	/* clear pos_list */
	for (int j = 0; j < i; j++) {
		delete pos_list[j];
	}

	delete[] pos_list;
	delete[] range_list;
	delete[] weights;
}

void Savvides::removeTriangulationTimer(unsigned int timer) {
	list<unsigned int>::iterator current;
	for (current = triangulation_timers.begin();
	     current != triangulation_timers.end();
	     current ++) {
		if (*current == timer) {
			triangulation_timers.erase(current);
			deleteTimer(timer);
			return;
		}
	}
	error("Non-existing triangulation_timer %d", timer);
	abort();
}

void Savvides::delayedDoTriangulation(unsigned int timer, void * _data) {
	tria_data * data = (tria_data *)_data;
	FLOAT res = data->res;
	FLOAT sum_conf = data->sum_conf;
	int i = data->neighbor_count;
	Coord pos = data->pos;

	removeTriangulationTimer(timer);
	delete data;

	// Filter out moves that violate the convex constraints imposed
	// by (distant) anchors and neighbors
	if (res >= 0 && !inside_rectangle(pos)) {
		//        savvides();
		res = -2;
	} else if (res >= 0 && !inside_neighbors_range(pos)) {
		res = -3;
	}

	if (res < 0 || res > range) {
		if (Savvides::pos.getConfidence() > ZERO_CONF) {
			Savvides::pos.setConfidence(ZERO_CONF);
			status = STATUS_BAD;
			setTimer(POSITION_TIMER, position_timer_interval);
		}
	} else {
		// Anchors must have a solid confidence compared to unknowns, so bound
		// derived confidence
		double conf = Min(0.5, sum_conf / i);

		// Only take action on significant moves
		if (Savvides::pos.distance(pos) > accuracy) {
			bool significant = false;

			if (res > 1.1 * residu && Savvides::pos.getConfidence() > 1.1 * LOW_CONF) {
				conf = LOW_CONF;
				// significant = true;
			}
			// We accept bad moves occasionally to get out of local minima
			if (res < residu || uniform(0, 1) < ACCEPT) {
				// record significant improvement and tell others
				Savvides::pos = Location(pos, simTime(), conf);
				residu = res;
#ifndef NDEBUG
				if (res < residu) {
					EV << id << ": UPDATE pos to " 
					   << pos.info() << " (" 
					   << 100 * getPosition().distance(pos) / range 
					   << "% error)" << "\n";
				} else {
					EV << id << ": ESCAPE pos to " 
					   << pos.info() << " (" 
					   << 100 * getPosition().distance(pos) / range
					   << "% error)" << "\n";
				}
#endif
				significant = true;
			} else {
#ifndef NDEBUG
				EV << id << ": REJECT move (" 
				   << res << " > " << residu << ")\n";
#endif
			}
			// Go tell others. Don't send message out directly, but use
			// retransmit queue instead. This will reduce the number of
			// xmitted msgs since multiple (buffered) incoming msgs
			// then yield one outgoing msg.

			if (significant) {
				setTimer(POSITION_TIMER, position_timer_interval);
			}
		}
		Savvides::pos.setConfidence((3 * Savvides::pos.getConfidence() + conf) / 4);
		status = Savvides::pos.getConfidence() > CONF_THR ? STATUS_POSITIONED : STATUS_BAD;
	}
}

void Savvides::receiveBBItem(int category,
			     const BBItem *details,
			     int scopeModuleId)
{
    	BaseModule::receiveBBItem(category, details, scopeModuleId);
	if (category != moveCategory) 
		return;

	if (!isAnchor)
		return;
#ifndef NDEBUG
	EV << "receiveBBItem()" << endl;
#endif
	move = *(static_cast<const Move *>(details));
        
	/* Add new position to anchor list */
	AnchorInfo *anchor = new AnchorInfo(id, isAnchor, getLocation(), 0.0);
	anchor->path_dst = 0.0;
	anchor->cnt = repeats;
	anchor->flood = true;
	/* Check if this anchor point exists in the anchor list */
	bool found = false;
	list<AnchorInfo *>::iterator iter;
	for (iter = anc.begin(); iter != anc.end(); iter++) {
		AnchorInfo *old_anchor = (*iter);
		if (anchor->id == old_anchor->id) {
#ifndef NDEBUG
			EV << anchor->pos.info() << "--" << old_anchor->pos.info() << endl;
#endif
			/* Let anchors be differentiated by their position */
			if (anchor->pos == old_anchor->pos) {
				found = true;
				break;
			}
		}
	}
	if (!found) {
		PUSH_BACK(anc,anchor);
		setTimer(ANCHOR_TIMER, anchor_timer_interval);
	} else {
		delete anchor;
	}
}
