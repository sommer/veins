#include "PositifLayer.h"
#include <string.h>
#include "FWMath.h"

// Node subclass containing Niculescu's Euclidean method for
// initialisation and Refine for refinement

// TODO: Add flooding limit. (now all targets are flooded)


// Variable parameters used in this class:
// var0:
//     0 for the normal strict test in safe_diagonal
//     1 for a less strict test
//     2 for a loose test
// var1:
//     0    use nbr_vote, or common_nbr if nbr_vote fails (as was commented in the tcl code and used for the paper)
//     1    just common_nbr (as in tcl sourcecode)
//     2    just nbr_vote
//     3    use common_nbr, or nbr_vote if common fails

#define ZERO_DISTANCE 0.0001
#define MERROR        range_variance	// Use this as substitute for $merror

#define msec		(1e-3)
#define ACCEPT		0.1
#define POS_DELAY	9

#define CONF_THR	(LOW_CONF*1.01)
#define ZERO_CONF       0.01
#define LOW_CONF        0.1
#define TRIANGULATION_WAIT 1


#define FOLLOW          230

#define FOLLOW_TARGET   9

// Define timer types
#define TIMER_SND_POS 0
#define TIMER_BLAST 1
#define TIMER_DO_TRI 2
#define TIMER_COUNT 3

// Define message types
#define MSG_BLAST     MSG_TYPE_BASE+0
#define MSG_POSITION  MSG_TYPE_BASE+1



// Can be used for nbrs, nnbrs, targets, ntargets, nntargets.
// In the case of nbrs and nntargets, from_idx will be me.
typedef struct {
	int from_idx;
	int to_idx;
	FLOAT distance;
	Position position;
	int cnt;
	int min_hops;		// This is just the number of hops followed on the path from the node to the anchor and is used only for the update_rectangle function of the Refine phase.
} link_info;

typedef struct {
	FLOAT d1;
	FLOAT d2;
	int result;
} diagonal_result;

typedef struct {
	int idx;
	double confidence;
	FLOAT distance;
	Position position;
	int nr_nghbrs;
	int *nghbr_idx;
	bool twin;
} nghbor_info;

class Node_EuclRefine:public PositifLayer {
	timer_info *bc_blast;
	bool send_all_data;
	cLinkedList targets;
	cLinkedList ntargets;
	cLinkedList nntargets;
	cLinkedList nbrs;
	cLinkedList nnbrs;

	timer_info *bc_position;
	int bc_pos_delay;
	timer_info *comp_position;
	cLinkedList neighbors;
	bool valid_rectangle;
	nghbor_info summary;
	bool summary_update;
	FLOAT residu;
	double accuracy;
	struct {
		Position min, max;
	} rectangle;		// convex region I must be in
	bool i_am_a_twin;

      public:
	 //Module_Class_Members(Node_EuclRefine, PositifLayer, 0)

	void blast(void);
	void recva(cMessage * msg);
	void get_data_from_blast(cMessage * msg);	// returns a list of newly added targets
	void print_data_from_blast(cMessage * msg);
	void print_stored_data();
	FLOAT common_nbr(link_info * target, int *min_hops);
	FLOAT nbr_vote(link_info * target, int *min_hops);
	FLOAT vote_list(cLinkedList * votes);
	diagonal_result safe_diagonal(FLOAT a, FLOAT b, FLOAT c, FLOAT d,
				      FLOAT e);
	void target_based_triangulation(void);
	bool exists_link(cLinkedList * list, int from, int to);
	link_info *get_link(cLinkedList * list, int from, int to);
	FLOAT diagonal(FLOAT a, FLOAT b, FLOAT c, FLOAT d, FLOAT e, int sign);

	// Refine stuff
	void sendPosition(void *arg = NULL);
	void do_triangulation(void *arg = NULL);
	void update_neighbor(cMessage * msg);
	void update_rectangle(link_info * anchor);
	bool inside_rectangle(Position pos);
	bool inside_neighbors_range(Position pos);
	bool twins(nghbor_info * a, nghbor_info * b);

	// Implement Node's abstract functions.
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

//Define_Module_Like(Node_EuclRefine, PositifLayer);



void Node_EuclRefine::init(void)
{
	algorithm = 4;		// Output on raw data line only.
	version = 19;

	if (MAX_TIMERS < TIMER_COUNT) {
		error
		    ("MAX_TIMERS < TIMER_COUNT: (%d < %d), increase MAX_TIMERS in PositifLayer.h\n",
		     MAX_TIMERS, TIMER_COUNT);
		abort();
	} else {
		/* allocate the needed timers */
		allocateRepeatTimers(TIMER_COUNT);
	}
}



void Node_EuclRefine::handleStartMessage(cMessage * msg)
{
	comp_position = NULL;
	bc_blast = NULL;
	bc_position = NULL;

	send_all_data = false;
	valid_rectangle = false;
	residu = 10 * range;
	summary.idx = me;
	summary.nr_nghbrs = 0;
	summary_update = false;
	i_am_a_twin = false;
	confidence = (status == STATUS_ANCHOR) ? 1 : ZERO_CONF;
	accuracy = par("accuracy");


	get_struct(msg, "position", position);

	if (status == STATUS_ANCHOR) {
		link_info *target = new link_info;

		target->distance = 0;
		target->from_idx = me;
		target->to_idx = me;
		memcpy(target->position, position, sizeof(position));
		target->cnt = reps;
		target->min_hops = 0;

		targets.insert(target);
		update_rectangle(target);

		bc_blast = timer(reps, TIMER_BLAST);
		addTimer(bc_blast);

		bc_position = timer(reps, TIMER_SND_POS);
		bc_pos_delay = 0;
		addTimer(bc_position);
	} else
		bc_blast = NULL;
}



void Node_EuclRefine::handleTimer(timer_info * timer)
{
	switch (*timer) {
	case TIMER_SND_POS:
		sendPosition(contextPointer(*timer));
		break;

	case TIMER_BLAST:
		blast();
		break;

	case TIMER_DO_TRI:
		//    if( triangulation_wait_counter == TRIANGULATION_WAIT ) {
		do_triangulation(contextPointer(*timer));
		//  triangulation_wait_counter=0;
		// }
		//
		//else
		// triangulation_wait_counter++;
	}
}

void Node_EuclRefine::handleMessage(cMessage * msg, bool newNeighbor)
{
	if (newNeighbor) {
		// Activate all timer routines when we meet a new neighbor
//              for (cLinkedList::Iterator iter = getTimers(); !iter.end();
//                   iter++) {
//                      timer_info *ev = (timer_info *) iter();
//                      resetTimer(ev);
//              }
		resetAllTimers();
		// And be sure to resend all previously sent data
		send_all_data = true;

		link_info *nbr = new link_info;
		nbr->from_idx = me;
		nbr->to_idx = msg->par("src");
		nbr->distance = (double) msg->par("distance");
		nbr->cnt = reps;
		nbr->min_hops = 0;
		nbrs.insert(nbr);
	}


	if (msg->getKind() == MSG_BLAST)
		recva(msg);

	if (msg->getKind() == MSG_POSITION) {
		// Maybe we're only interested in the initial estimate this time.
		if (!do_2nd_phase || status == STATUS_ANCHOR)
			return;

		update_neighbor(msg);

		if (neighbors.length() > nr_dims) {
			// Don't perform triangulation yet, wait for other updates
			if (comp_position == NULL) {
				comp_position = timer(1, TIMER_DO_TRI);
				addTimer(comp_position);
			}
			resetTimer(comp_position);
		}
	}
}


void Node_EuclRefine::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);
	// Nothing special to be done for now.

	PositifLayer::handleStopMessage(msg);

	send(returnMsg);
}

void Node_EuclRefine::blast(void)
{
	// TODO, just send data that wasn't previously sent or that has changed. (like geuclid.tcl)
	cMessage *msg = new cMessage("BLAST", MSG_BLAST);

	// Add all new neighbor, anchor, and nanchor data to the message.
	// (Don't send nnanchor and nneighbor because no one cares about
	//  nnnanchor or nnneighbor)
	// Don't check whether data is new if 'send_all_data' is set. This
	// allows us to update newly joining nodes.

	int i = 0, j = 0;
	char parname[100] = "";
	for (cLinkedList::Iterator iter(nbrs); !iter.end(); iter++) {
		link_info *nbr = (link_info *) iter();

		if (nbr->cnt != 0 || send_all_data) {
			sprintf(parname, "nbr_%d", i);
			add_struct2(msg, parname, nbr);
			i++;
			nbr->cnt--;
		}
	}
	msg->addPar("num_nbrs") = i;

	j += i;
	i = 0;
	for (cLinkedList::Iterator iter(targets); !iter.end(); iter++) {
		link_info *target = (link_info *) iter();

		if (target->cnt != 0 || send_all_data) {
			sprintf(parname, "target_%d", i);
			add_struct2(msg, parname, target);
			i++;
			target->cnt--;
		}
	}
	msg->addPar("num_targets") = i;

	j += i;
	i = 0;
	for (cLinkedList::Iterator iter(ntargets); !iter.end(); iter++) {
		link_info *ntarget = (link_info *) iter();

		if (ntarget->cnt != 0 || send_all_data) {
			sprintf(parname, "ntarget_%d", i);
			add_struct2(msg, parname, ntarget);
			i++;
			ntarget->cnt--;
		}
	}
	msg->addPar("num_ntargets") = i;

	send_all_data = false;

	j += i;
	if (j == 0)		// this shouldn't happen...
		fprintf(stderr, "IGNORE\n");
	else
		send(msg);
}

void Node_EuclRefine::get_data_from_blast(cMessage * msg)
{
	//int src = msg->par( "src" );
	int num_nbrs = msg->par("num_nbrs");
	int num_targets = msg->par("num_targets");
	int num_ntargets = msg->par("num_ntargets");

	char parname[100] = "";
	// For the next 3 blocks: new nbrs are stored in nnbr, targets in ntargets, and ntargets in nntargets.
	for (int i = 0; i < num_nbrs; i++) {
		// Get the data from the message
		sprintf(parname, "nbr_%d", i);
		link_info *nnbr = new link_info;
		get_struct2(msg, parname, nnbr);

		// Check if we received this one before (could happen because of the resending when new nodes appear.
		bool found = false;
		for (cLinkedList::Iterator iter(nnbrs); !iter.end(); iter++) {
			link_info *nnbr_old = (link_info *) iter();
			if (nnbr_old->from_idx == nnbr->from_idx
			    && nnbr_old->to_idx == nnbr->to_idx) {
				found = true;
				break;
			}
		}

		if (found)
			delete(nnbr);
		else
			nnbrs.insert(nnbr);
	}

	for (int i = 0; i < num_targets; i++) {
		// Get the data from the message
		sprintf(parname, "target_%d", i);
		link_info *ntarget = new link_info;
		get_struct2(msg, parname, ntarget);

		// Check if we received this one before (could happen because of the resending when new nodes appear.
		bool found = false;
		for (cLinkedList::Iterator iter(ntargets); !iter.end(); iter++) {
			link_info *ntarget_old = (link_info *) iter();
			if (ntarget_old->from_idx == ntarget->from_idx
			    && ntarget_old->to_idx == ntarget->to_idx) {
				found = true;
				break;
			}
		}

		if (found)
			delete(ntarget);
		else {
			ntargets.insert(ntarget);
		}
	}

	for (int i = 0; i < num_ntargets; i++) {
		// Get the data from the message
		sprintf(parname, "ntarget_%d", i);
		link_info *nntarget = new link_info;
		get_struct2(msg, parname, nntarget);

		// Check if we received this one before (could happen because of the resending when new nodes appear.
		bool found = false;
		for (cLinkedList::Iterator iter(nntargets); !iter.end(); iter++) {
			link_info *nntarget_old = (link_info *) iter();
			if (nntarget_old->from_idx == nntarget->from_idx
			    && nntarget_old->to_idx == nntarget->to_idx) {
				found = true;
				break;
			}
		}

		if (found)
			delete(nntarget);
		else
			nntargets.insert(nntarget);
	}
}

void Node_EuclRefine::print_stored_data()
{
	logprintf("ENV--");
	logprintf("%d NBRS: ", nbrs.length());
	if (nbrs.length() > 0)
		for (cLinkedList::Iterator iter(nbrs); !iter.end(); iter++) {
			link_info *link = (link_info *) iter();
			logprintf("%d-%d@%3.1f, ", link->from_idx, link->to_idx,
				  link->distance);
		}
	logprintf("\n      %d NNBRS: ", nnbrs.length());
	if (nnbrs.length() > 0)
		for (cLinkedList::Iterator iter(nnbrs); !iter.end(); iter++) {
			link_info *link = (link_info *) iter();
			logprintf("%d-%d@%3.1f, ", link->from_idx, link->to_idx,
				  link->distance);
		}
	logprintf("\n      %d TGTS: ", targets.length());
	if (targets.length() > 0)
		for (cLinkedList::Iterator iter(targets); !iter.end(); iter++) {
			link_info *link = (link_info *) iter();
			logprintf("%d-%d@%3.1f, ", link->from_idx, link->to_idx,
				  link->distance);
		}
	logprintf("\n      %d NTGTS: ", ntargets.length());
	if (ntargets.length() > 0)
		for (cLinkedList::Iterator iter(ntargets); !iter.end(); iter++) {
			link_info *link = (link_info *) iter();
			logprintf("%d-%d@%3.1f, ", link->from_idx, link->to_idx,
				  link->distance);
		}
	logprintf("\n      %d NNTGTS: ", nntargets.length());
	if (nntargets.length() > 0)
		for (cLinkedList::Iterator iter(nntargets); !iter.end(); iter++) {
			link_info *link = (link_info *) iter();
			logprintf("%d-%d@%3.1f, ", link->from_idx, link->to_idx,
				  link->distance);
		}
	logprintf("\n");
}

void Node_EuclRefine::print_data_from_blast(cMessage * msg)
{
	int src = msg->par("src");
	int num_nbrs = msg->par("num_nbrs");
	int num_targets = msg->par("num_targets");
	int num_ntargets = msg->par("num_ntargets");

	char parname[100] = "";

	logprintf("MSGDATA src %d, ", src);

	logprintf("%d NBRS:, ", num_nbrs);
	for (int i = 0; i < num_nbrs; i++) {
		// Get the data from the message
		sprintf(parname, "nbr_%d", i);
		link_info *nnbr = new link_info;
		get_struct2(msg, parname, nnbr);


		// Check if we received this one before (could happen because of the resending when new nodes appear.
		if (exists_link(&nnbrs, nnbr->from_idx, nnbr->to_idx))
			logprintf("(%d@%3.1f), ", nnbr->to_idx, nnbr->distance);
		else
			logprintf("%d@%3.1f, ", nnbr->to_idx, nnbr->distance);
	}

	logprintf("%d TGTS:, ", num_targets);
	for (int i = 0; i < num_targets; i++) {
		// Get the data from the message
		sprintf(parname, "target_%d", i);
		link_info *ntarget = new link_info;
		get_struct2(msg, parname, ntarget);

		if (exists_link(&ntargets, ntarget->from_idx, ntarget->to_idx))
			logprintf("(%d@%3.1f), ", ntarget->to_idx,
				  ntarget->distance);
		else
			logprintf("%d@%3.1f, ", ntarget->to_idx,
				  ntarget->distance);
	}

	logprintf("%d NTGTS:, ", num_ntargets);
	for (int i = 0; i < num_ntargets; i++) {
		// Get the data from the message
		sprintf(parname, "ntarget_%d", i);
		link_info *nntarget = new link_info;
		get_struct2(msg, parname, nntarget);

		if (exists_link
		    (&nntargets, nntarget->from_idx, nntarget->to_idx))
			logprintf("(%d@%3.1f), ", nntarget->to_idx,
				  nntarget->distance);
		else
			logprintf("%d@%3.1f, ", nntarget->to_idx,
				  nntarget->distance);
	}
	logprintf("\n");
}


void Node_EuclRefine::recva(cMessage * msg)
{
	cLinkedList newtargets;
	bool changed = false;

	int num_targets = msg->par("num_targets");
	char parname[100] = "";

	//  if( FOLLOW==me )
	//  print_data_from_blast( msg );

	// Retrieve the data from the message. (first 17 loc in geuclid.tcl)
	get_data_from_blast(msg);

	// For each ntarget in the message
	for (int i = 0; i < num_targets; i++) {
		// Get the data from the message
		sprintf(parname, "target_%d", i);
		link_info *ntarget = new link_info;
		get_struct2(msg, parname, ntarget);

		// If we already have a distance to this target, continue.
		if (exists_link(&(targets), me, ntarget->to_idx))
			continue;

		// If we are an anchor/target (GPS node in Niculescu terms)
		// Calculate the exact distance, and store this.
		if (status == STATUS_ANCHOR) {
			link_info *newtarget = new link_info;
			newtarget->from_idx = me;
			newtarget->to_idx = ntarget->to_idx;
			newtarget->distance =
			    distance(ntarget->position, position);
			memcpy(newtarget->position, ntarget->position,
			       sizeof(ntarget->position));
			if (targets.length() <= flood_limit || flood_limit < 0)
				newtarget->cnt = reps;
			else
				newtarget->cnt = 0;	// Kinda pointless. If there's a timer running, we might as well send this one along. But this way the comparison with other algorithms is more fair.
			newtarget->min_hops = ntarget->min_hops + 1;
			targets.insert(newtarget);
			update_rectangle(newtarget);
			changed = true;
			logprintf("++Target adding target %d\n",
				  ntarget->to_idx);
			continue;
		}
		// If the node we got this message from is an anchor/target
		// distance in the message will be 0 (but float) and we can
		// just use the distance from the message data.
		if (ntarget->distance < ZERO_DISTANCE) {
			link_info *newtarget = new link_info;
			newtarget->from_idx = me;
			newtarget->to_idx = ntarget->to_idx;
			newtarget->distance = (double) msg->par("distance");
			memcpy(newtarget->position, ntarget->position,
			       sizeof(ntarget->position));
			if (targets.length() <= flood_limit || flood_limit < 0)
				newtarget->cnt = reps;
			else
				newtarget->cnt = 0;	// Kinda pointless. If there's a timer running, we might as well send this one along. But this way the comparison with other algorithms is more fair.
			newtarget->min_hops = 1;	//(==ntarget->min_hops+1)
			targets.insert(newtarget);
			update_rectangle(newtarget);
			changed = true;
			logprintf("++Adding neighbour target %d\n",
				  ntarget->to_idx);
			continue;
		}
		//if( FOLLOW==me )
		//  print_stored_data( );








		// Change from geuclid.tcl: enabled nbr_vote
		FLOAT estim, estim_v, estim_c;
		int min_hops = 0, min_hops_v, min_hops_c;
		estim_v = nbr_vote(ntarget, &min_hops_v);
		estim_c = common_nbr(ntarget, &min_hops_c);

		/* Var1 will determine which method is used:
		   0    use nbr_vote, or common_nbr if nbr_vote fails (as was commented in the tcl code and used for the paper)
		   1    just common_nbr (as in tcl sourcecode)
		   2    just nbr_vote
		   3    use common_nbr, or nbr_vote if common fails
		 */
		switch ((int) (var1 + 0.1)) {
		case 0:
			if (estim_v > 0 && finite(estim_v)) {
				estim = estim_v;
				min_hops = min_hops_v;
			} else if (finite(estim_c)) {
				estim = estim_c;
				min_hops = min_hops_c;
			} else
				estim = -1;
			break;
		case 1:
			{
				estim = estim_c;
				min_hops = min_hops_c;
			}
			break;
		case 2:
			{
				estim = estim_v;
				min_hops = min_hops_v;
			}
			break;
		case 3:
			if (estim_c > 0 && finite(estim_c)) {
				estim = estim_c;
				min_hops = min_hops_c;
			} else if (finite(estim_v)) {
				estim = estim_v;
				min_hops = min_hops_v;
			} else
				estim = -1;
			break;
		default:
			estim = -1;
		}

		if (FOLLOW == me)
			logprintf("-------%d %f %f %f\n", ntarget->to_idx,
				  estim_v, estim_c, estim);

		//    logprintf( "vote: %4.2f  common nbr: %4.2f  real: %4.2f\n", estim_v, estim_c, distance( node[me].true_pos, ntarget->position ) );

		link_info *newtarget;
		if (estim > 0) {
			newtarget = new link_info;
			newtarget->from_idx = me;
			newtarget->to_idx = ntarget->to_idx;
			memcpy(newtarget->position, ntarget->position,
			       sizeof(ntarget->position));
			newtarget->distance = estim;
			if (targets.length() <= flood_limit || flood_limit < 0)
				newtarget->cnt = reps;
			else
				newtarget->cnt = 0;	// Kinda pointless. If there's a timer running, we might as well send this one along. But this way the comparison with other algorithms is more fair.
			newtarget->min_hops = min_hops;
			targets.insert(newtarget);
			update_rectangle(newtarget);
			changed = true;
			logprintf
			    ("++Adding target using euclidean %d @ %4.0f (real: %4.0f)\n",
			     ntarget->to_idx, estim, distance(ntarget->position,
							      node[me].
							      true_pos));
			break;
		}
		// And repeat this for all targets we got new information on.
	}

	// TODO: I don't think this propagates stuff properly.
	// TODO: We should limit flooding in some way.
	if (changed && (targets.length() <= flood_limit || flood_limit < 0)) {
		seqno[MSG_BLAST]++;	// kludge: no filtering iso seqno per anchor event
		if (!bc_blast) {
			bc_blast = timer(reps, TIMER_BLAST);
			addTimer(bc_blast);
		}
		resetTimer(bc_blast);
	}
	if (FOLLOW == me)
		logprintf("----------------------------\n");

	if (status != STATUS_ANCHOR)
		target_based_triangulation();
}


void Node_EuclRefine::target_based_triangulation(void)
{
	int n = targets.length();
	if (n >= phase1_min_anchors) {

		used_anchors = (n < phase1_max_anchors
				|| phase1_max_anchors ==
				-1) ? n : phase1_max_anchors;
		FLOAT** pos_list = new FLOAT*[used_anchors + 1];
		if (range_list)
			delete range_list;
		range_list = new FLOAT[used_anchors + 1];
		if (real_range_list)
			delete real_range_list;
		real_range_list = new FLOAT[used_anchors + 1];	// Used for debug output only
		int* idx_list = new int[used_anchors];

		int i = 0;
		for (cLinkedList::Iterator iter(targets); !iter.end(); iter++) {
			link_info *target = (link_info *) iter();
			int store_at = -1;
			FLOAT range = target->distance;

			if (i < used_anchors) {
				store_at = i;
				i++;
			} else {
				for (int j = 0; j < used_anchors; j++)
					if ((store_at < 0
					     && range_list[j] > range)
					    || (store_at >= 0
						&& range_list[j] >
						range_list[store_at])) {
						store_at = j;
					}
			}

			if (store_at >= 0) {
				pos_list[store_at] = target->position;
				range_list[store_at] = range;
				real_range_list[store_at] =
				    distance(node[me].true_pos,
					     node[target->to_idx].true_pos);
				idx_list[store_at] = target->to_idx;
			}
		}

		// Calculate performance data for range error
		FLOAT sum_err = 0, sum_rel_err = 0, sum_abs_err =
		    0, sum_abs_rel_err = 0, sum_range = 0;
		for (i = 0; i < used_anchors; i++) {
			sum_err += range_list[i] - real_range_list[i];
			sum_rel_err +=
			    (range_list[i] -
			     real_range_list[i]) / real_range_list[i];
			sum_abs_err += fabs(range_list[i] - real_range_list[i]);
			sum_abs_rel_err +=
			    fabs(range_list[i] -
				 real_range_list[i]) / real_range_list[i];
			sum_range += range_list[i];
		}
		node[me].perf_data.anchor_range_error = sum_err / used_anchors;
		node[me].perf_data.rel_anchor_range_error =
		    sum_rel_err / used_anchors;
		node[me].perf_data.abs_anchor_range_error =
		    sum_abs_err / used_anchors;
		node[me].perf_data.abs_rel_anchor_range_error =
		    sum_abs_rel_err / used_anchors;
		node[me].perf_data.anchor_range = sum_range / used_anchors;
		node[me].perf_data.anchor_range_error_count = used_anchors;

		Position next;
		pos_list[used_anchors] = next;

		FLOAT res;
		if (tri_alg == 0)
			res =
			    triangulate(used_anchors, pos_list, range_list,
					NULL, me);
		else
			res =
			    savvides_minmax(used_anchors, pos_list, range_list,
					    NULL, me);

//              wait(n * nr_dims * nr_dims * msec);

		if (0 <= res && res <= range) {
			memmove(position, next, sizeof(Position));
			confidence = LOW_CONF;
			if (status == STATUS_UNKNOWN) {
				bc_position = timer(1, TIMER_SND_POS);
				bc_pos_delay = POS_DELAY;
				addTimer(bc_position);
			}
			status = STATUS_POSITIONED;	// After this phase, a node is positioned regardless of its confidence.
			//TODO: enable when we include Refine phase    residu = range;
			node[me].perf_data.phase1_err =
			    distance(position, node[me].true_pos) / range;
		} else {
			node[me].perf_data.phase2_err =
			    distance(next, node[me].true_pos) / range;
		}


		for (int j = 0; j < used_anchors; j++)
			logprintf("(%d %4.0f %4.0f @ %4.0f) ", idx_list[j],
				  pos_list[j][0], pos_list[j][1],
				  range_list[j]);


		logprintf(" p1->%4.0f,%4.0f (%1.1f) using ", position[0],
			  position[1], node[me].perf_data.phase1_err);
		for (cLinkedList::Iterator iter(targets); !iter.end(); iter++) {
			link_info *target = (link_info *) iter();
			bool used = false;
			int j;
			for (j = 0; j < used_anchors; j++)
				if (idx_list[j] == target->to_idx) {
					used = true;
					break;
				}
			if (used)
				logprintf
				    (" -->(%d:(%4.0f,%4.0f)@%4.0fd,%4.0fr)<-- ",
				     target->to_idx, target->position[0],
				     target->position[1], range_list[j],
				     distance(node[me].true_pos,
					      node[target->to_idx].true_pos));
			else
				logprintf(" (%d:(%4.0f,%4.0f)@%4.0fd,%4.0fr) ",
					  target->to_idx, target->position[0],
					  target->position[1], target->distance,
					  distance(node[me].true_pos,
						   node[target->to_idx].
						   true_pos));
		}
		logprintf("\n");
		delete[] idx_list;
		delete[] pos_list;
	}
}



bool Node_EuclRefine::exists_link(cLinkedList * list, int from, int to)
{
	for (cLinkedList::Iterator iter(*list); !iter.end(); iter++) {
		link_info *link = (link_info *) iter();
		if (link->from_idx == from && link->to_idx == to)
			return true;
	}
	return false;
}

link_info *Node_EuclRefine::get_link(cLinkedList * list, int from, int to)
{
	for (cLinkedList::Iterator iter(*list); !iter.end(); iter++) {
		link_info *link = (link_info *) iter();
		if (link->from_idx == from && link->to_idx == to)
			return link;
	}
	return NULL;
}

FLOAT Node_EuclRefine::diagonal(FLOAT a, FLOAT b, FLOAT c, FLOAT d, FLOAT e,
				int sign)
{
	FLOAT self_x = (e * e + d * d - a * a) / (2 * e);
	FLOAT self_y = sqrt(d * d - self_x * self_x);
	FLOAT t_x = (e * e + c * c - b * b) / (2 * e);
	FLOAT tmp = c * c - t_x * t_x;
	if (tmp > -0.1) {	// We'll allow for a bit of rounding error before rejecting results. (What's the "right" value to use here??)
		FLOAT t_y = (FLOAT) sign * sqrt(fabs(tmp));
		FLOAT d_x = self_x - t_x;
		FLOAT d_y = self_y - t_y;
		return sqrt(d_x * d_x + d_y * d_y);
	} else
		return -1;
}

diagonal_result Node_EuclRefine::safe_diagonal(FLOAT a, FLOAT b, FLOAT c,
					       FLOAT d, FLOAT e)
{
	diagonal_result result;

	result.result = 0;

	if (var0 < 0.5) {	// var0==0
		if (a + e < (1 + 2 * MERROR) * d)
			result.result = 1;
		if (a + d < (1 + 2 * MERROR) * e)
			result.result = 1;
		if (e + d < (1 + 2 * MERROR) * a)
			result.result = 1;
		if (b + c < (1 + 2 * MERROR) * e)
			result.result = 2;
		if (b + e < (1 + 2 * MERROR) * c)
			result.result = 2;
		if (e + c < (1 + 2 * MERROR) * b)
			result.result = 2;
	} else if (var0 < 1.5) {	// var0==1
		if (a + e < d)
			result.result = 1;
		if (a + d < e)
			result.result = 1;
		if (e + d < a)
			result.result = 1;
		if (b + c < e)
			result.result = 2;
		if (b + e < c)
			result.result = 2;
		if (e + c < b)
			result.result = 2;
	} else {		// var0==2
		if ((1 + 2 * MERROR) * (a + e) < d)
			result.result = 1;
		if ((1 + 2 * MERROR) * (a + d) < e)
			result.result = 1;
		if ((1 + 2 * MERROR) * (e + d) < a)
			result.result = 1;
		if ((1 + 2 * MERROR) * (b + c) < e)
			result.result = 2;
		if ((1 + 2 * MERROR) * (b + e) < c)
			result.result = 2;
		if ((1 + 2 * MERROR) * (e + c) < b)
			result.result = 2;
	}

	result.d1 = diagonal(a, b, c, d, e, -1);
	result.d2 = diagonal(a, b, c, d, e, 1);

	if (result.d1 < 0)
		result.result = 3;

	return result;
}

FLOAT Node_EuclRefine::vote_list(cLinkedList * votes)
{
#define A1D1 1
#define A1D2 2
#define A2D1 3
#define A2D2 4

	cStdDev a1, a2;
	cLinkedList::Iterator iter(*votes);
	diagonal_result *dres = (diagonal_result *) iter();

	a1 += dres->d1;
	a2 += dres->d2;

	for (; !iter.end(); iter++) {
		dres = (diagonal_result *) iter();

		// Find the combination with the minimum difference between a and d
		int min_opt = A1D1;
		FLOAT min_val = fabs(a1.getMean() - dres->d1);
		if (fabs(a1.getMean() - dres->d2) < min_val) {
			min_opt = A1D2;
			min_val = fabs(a1.getMean() - dres->d2);
		}
		if (fabs(a2.getMean() - dres->d1) < min_val) {
			min_opt = A2D1;
			min_val = fabs(a2.getMean() - dres->d1);
		}
		if (fabs(a2.getMean() - dres->d2) < min_val) {
			min_opt = A2D2;
			min_val = fabs(a2.getMean() - dres->d2);
		}

		switch (min_opt) {
		case A1D1:
		case A2D2:
			a1 += dres->d1;
			a2 += dres->d2;
			break;
		case A1D2:
		case A2D1:
			a1 += dres->d2;
			a2 += dres->d1;
			break;
		}
	}

	if (a1.getStddev() < a2.getStddev())
		return a1.getMean();
	else
		return a2.getMean();
}

FLOAT Node_EuclRefine::nbr_vote(link_info * target, int *min_hops)
{
	//
	// the target structure give the distance from some neighbor, _to_ a target.
	// hence the idx of the target is to_idx, not from_idx.
	//
	// Try to find two neighbors that both have a distance estimate to t
	cLinkedList votes;

	int n2count = 0;
	int n1count = 0;
	*min_hops = 10000;

	if (me == FOLLOW && target->to_idx == FOLLOW_TARGET)
		logprintf("Entering vote function\n");

	for (cLinkedList::Iterator iter2(nbrs); !iter2.end(); iter2++, n2count++) {
		link_info *n2 = (link_info *) iter2();
		link_info *ntarget2;

		if (me == FOLLOW && target->to_idx == FOLLOW_TARGET)
			logprintf("***n2 %d\n", n2->to_idx);

		if (!
		    (ntarget2 =
		     get_link(&(ntargets), n2->to_idx, target->to_idx))) {
			if (me == FOLLOW && target->to_idx == FOLLOW_TARGET)
				logprintf(" reject %d (no target)\n",
					  n2->to_idx);

			continue;	// This neighbor has no range info for target.
		}

		n1count = 0;
		for (cLinkedList::Iterator iter(nbrs); !iter.end();
		     iter++, n1count++) {
			link_info *n1 = (link_info *) iter();
			link_info *ntarget1;	// link between n1 and the target
			link_info *n1_2;	// link between n1 and n2 ( $an2($n1) in Niculescu's code )

			if (me == FOLLOW && target->to_idx == FOLLOW_TARGET)
				logprintf(" +++n1 %d\n", n1->to_idx);

			if (n1count <= n2count) {
				if (me == FOLLOW
				    && target->to_idx == FOLLOW_TARGET)
					logprintf("  n1<n2 %d %d\n", n1count,
						  n2count);
				continue;
			}

			if (!
			    (ntarget1 =
			     get_link(&(ntargets), n1->to_idx,
				      target->to_idx))) {
				if (me == FOLLOW
				    && target->to_idx == FOLLOW_TARGET)
					logprintf
					    ("  reject %d %d (2nd has no target)\n",
					     n2->to_idx, n1->to_idx);

				continue;	// This neighbor has no range info for target.
			}

			if (!
			    (n1_2 = get_link(&(nnbrs), n2->to_idx, n1->to_idx)))
				//      if( !(n1_2=get_link( &(nnbrs), n1->to_idx, n2->to_idx ) ) ) // Not like Niculescu coded it. I think.
			{
				if (me == FOLLOW
				    && target->to_idx == FOLLOW_TARGET)
					logprintf
					    ("  reject %d %d (no neighbours)\n",
					     n2->to_idx, n1->to_idx);

				continue;	// There is no range information between n1 and n2
			}
			// NOTE: it is true that there may be a record in nnbr for n1->n2,
			// so this combination may be discarded unnecessarily. But if so,
			// the reverse combination (with n1 and n2 swapped) can be tried
			// later.


			// We now have: n1(me,n1), ntarget1(n1,t), ntarget2(n2,t), n2(me,n2)
			// (var(fromnode,tonode))
			diagonal_result *d12 = new diagonal_result;
			*d12 = safe_diagonal(n1->distance,
					     ntarget1->distance,
					     ntarget2->distance,
					     n2->distance, n1_2->distance);

			if (d12->result == 1 || d12->result == 3)	// Some error generated by safe_diagonal
			{
				if (me == FOLLOW
				    && target->to_idx == FOLLOW_TARGET)
					logprintf
					    ("  reject %d %d (bad diagonal)\n",
					     n2->to_idx, n1->to_idx);

				continue;
			}

			bool found = false;
			for (cLinkedList::Iterator iterv(votes); !iterv.end();
			     iterv++) {
				diagonal_result *dvote =
				    (diagonal_result *) iterv();
				if (fabs(1.0 - dvote->d1 / d12->d1) < 0.01
				    && fabs(1.0 - dvote->d2 / d12->d2) < 0.01) {
					found = true;
					break;
				}
				if (fabs(1.0 - dvote->d1 / d12->d2) < 0.01
				    && fabs(1.0 - dvote->d2 / d12->d1) < 0.01) {
					found = true;
					break;
				}
			}

			if (me == FOLLOW && target->to_idx == FOLLOW_TARGET)
				if (found)
					logprintf
					    ("  Already present! (%d %d)\n",
					     n2->to_idx, n1->to_idx);
				else
					logprintf("  Adding vote! (%d %d)\n",
						  n2->to_idx, n1->to_idx);

			if (!found) {
				*min_hops =
				    MIN(*min_hops,
					MIN(ntarget1->min_hops,
					    ntarget2->min_hops));
				votes.insert(d12);
			} else
				delete d12;

		}
	}

	if (me == FOLLOW && target->to_idx == FOLLOW_TARGET)
		logprintf("%d votes\n", votes.length());

	FLOAT res;
	if (votes.length() >= 2)
		res = vote_list(&votes);
	else
		res = -1;

	logprintf("--NBR_VOTE ");

	for (cLinkedList::Iterator iterv(votes); !iterv.end(); iterv++) {
		diagonal_result *vote = (diagonal_result *) iterv();
		logprintf("(%f %f) ", vote->d1, vote->d2);
		delete vote;
	}

	logprintf(" RESULT %f\n", res);

	(*min_hops)++;
	return res;
}

FLOAT Node_EuclRefine::common_nbr(link_info * target, int *min_hops)
{
	//
	// the target structure give the distance from some neighbor, _to_ a target.
	// hence the idx of the target is to_idx, not from_idx.
	//
	// Try to find two neighbors that both have a distance estimate to t

	for (cLinkedList::Iterator iter2(nbrs); !iter2.end(); iter2++) {
		link_info *n2 = (link_info *) iter2();
		link_info *ntarget2;

		if (!
		    (ntarget2 =
		     get_link(&(ntargets), n2->to_idx, target->to_idx)))
			continue;	// This neighbor has no range info for target.

		for (cLinkedList::Iterator iter(nbrs); !iter.end(); iter++) {
			link_info *n1 = (link_info *) iter();
			link_info *ntarget1;	// link between n1 and the target
			link_info *n1_2;	// link between n1 and n2 ( $an2($n1) in Niculescu's code )

			if (!
			    (ntarget1 =
			     get_link(&(ntargets), n1->to_idx, target->to_idx)))
				continue;	// This neighbor has no range info for target.

			if (!
			    (n1_2 = get_link(&(nnbrs), n2->to_idx, n1->to_idx)))
				continue;	// There is no range information between n1 and n2
			// NOTE: it is true that there may be a record in nnbr for n1->n2,
			// so this combination may be discarded unnecessarily. But if so,
			// the reverse combination (with n1 and n2 swapped) can be tried
			// later.


			// We now have: n1(me,n1), ntarget1(n1,t), ntarget2(n2,t), n2(me,n2)
			// (var(fromnode,tonode))
			diagonal_result d12 = safe_diagonal(n1->distance,
							    ntarget1->distance,
							    ntarget2->distance,
							    n2->distance,
							    n1_2->distance);

			if (d12.result == 1 || d12.result == 3)	// Some error generated by safe_diagonal
				continue;

			// Now, find common neighbors of n1 and n2. (cn)
			for (cLinkedList::Iterator citer(nnbrs); !citer.end();
			     citer++) {
				link_info *cn1 = (link_info *) citer();
				link_info *cn2;
				link_info *cntarget;

				if (cn1->to_idx == me	// Don't want to be our own common node. This test should be unnecessary (cover by nntargets test), but it's implemented in Niculescu's code as well.
				    || cn1->from_idx != n1->to_idx	// Make sure it has a link with n1 (stored in cn1)
				    || !(cn2 = get_link(&(nnbrs), n2->to_idx, cn1->to_idx))	// and with n2 (stored in cn2)
				    || !(cntarget = get_link(&(nntargets), cn1->to_idx, ntarget1->to_idx))	// and with the target (stored in cntarget)
				    )
					continue;
				// We now have: n1(me,n1), ntarget1(n1,t), ntarget2(n2,t), n2(me,n2),
				// cn1(n1,cn), cn2(n2,cn), cntarget(cn,t)
				// (var(fromnode,tonode))

				diagonal_result dab =
				    safe_diagonal(n1->distance,
						  cn1->distance,
						  cn2->distance,
						  n2->distance,
						  n1_2->distance);
				if (dab.result > 0)
					continue;	// Any error generated by safe_diagonal->abort for this cn.

				int sidecn = 123;
				link_info *cn;	// Direct link to common node?
				if ((cn = get_link(&(nbrs), me, cn1->to_idx))) {
					// Don't think this part can be directly translated to our framework.
					// Niculescu seems to assume a maximum error?
					// Instead just select the side that is closest to the real answer.
					if (fabs(dab.d1 - cn->distance) <
					    fabs(dab.d2 - cn->distance))
						sidecn = -1;
					else
						sidecn = 1;
				} else {	// No direct link to common node
					// 20020827 N. Reijers: Note, this test may have worked in Niculescu's simulations, but in our framework there may be collisions which means that we cannot assume we would have heard from the node if it is within radio range. This test was responsible for quite a lot of errors in our test case.
					if (0.9 * range > dab.d2)	// but should have been
						continue;
					//        sidecn=-1;
					else
						continue;	// can't do anything with this common node. try another.
				}

				diagonal_result dsl =
				    safe_diagonal(cn1->distance,
						  ntarget1->distance,
						  ntarget2->distance,
						  cn2->distance,
						  n1_2->distance);
				if (dsl.result == 3)
					continue;

				int sidet = 456;

				if (fabs(dsl.d1 - cntarget->distance) >
				    fabs(dsl.d2 - cntarget->distance))
					sidet = 1;
				else
					sidet = -1;

				if (dsl.result == 1) {
					// From geuclid.tcl:
					// If cn, n1, n2 were colinear, this would have been taken care of
					// by self-cn test above.
					// If t, n1, n2 are colinear, then d1=d2 (side does not matter)
					// continue
					if (fabs(d12.d1 - d12.d2) < MERROR * d12.d1)	// Assuming MERROR is the range_variance (percentage of range) AND the tcl compare function ([compare a b c]) checks if a and b differ by at most c percent. (so |a/b-1|<c)
						sidet = 1;
					else
						continue;
				}

				logprintf
				    ("--COMMON_NBR n1 %d@%3.0f n2 %d@%3.0f n1t %3.0f n2t %3.0f n1-2 %3.0f RESULT %3.0f %3.0f  (sel: %3.0f)\n",
				     n1->to_idx, n1->distance, n2->to_idx,
				     n2->distance, ntarget1->distance,
				     ntarget2->distance, n1_2->distance, d12.d1,
				     d12.d2,
				     sidet * sidecn == -1 ? d12.d1 : d12.d2);
				logprintf
				    ("--COMMON_NBR USING cn %d cn1 %3.0f cn2 %3.0f cnt %3.0f  sidecn %i sidet %i\n",
				     cn1->to_idx, cn1->distance, cn2->distance,
				     cntarget->distance, sidecn, sidet);


				*min_hops =
				    MIN(cntarget->min_hops,
					MIN(ntarget1->min_hops,
					    ntarget2->min_hops)) + 1;
				if (sidet * sidecn == -1)
					return d12.d1;
				else
					return d12.d2;
			}
		}
	}
	return -1;
}












void Node_EuclRefine::update_neighbor(cMessage * msg)
{
	nghbor_info *neighbor = NULL;
	int src = msg->par("src");

	bool found = false;
	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
		neighbor = (nghbor_info *) iter();

		if (neighbor->idx == src) {
			found = true;
			break;
		}
	}

	if (!found) {
		neighbor = new nghbor_info;
		neighbor->idx = src;
		neighbor->nr_nghbrs = 0;
		neighbor->twin = false;
		neighbors.insert(neighbor);

		// Update summary
		assert(summary.nr_nghbrs == neighbors.length() - 1);

		if (summary.nr_nghbrs > 0) {
			delete summary.nghbr_idx;
		}
		int n = ++summary.nr_nghbrs;
		summary.nghbr_idx = new int[n];
		int i = 0;
		for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
			summary.nghbr_idx[i++] = ((nghbor_info *) iter())->idx;
		}
		summary_update = true;

		// The summary is piggybacked on ordinary MSG_POSITION msgs, so
		// reset the associated timer if that has already been activated
		if (bc_position != NULL) {
			resetTimer(bc_position);
			seqno[MSG_POSITION]++;
		}
	}

	get_struct(msg, "pos", *(Position *) neighbor->position);
	neighbor->distance = (double) msg->par("distance");
	if (!neighbor->twin)
		neighbor->confidence = msg->par("confidence");

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
		for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
			nghbor_info *m = (nghbor_info *) iter();

			m->twin = false;
		}
		for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
			nghbor_info *m = (nghbor_info *) iter();

			// Skip anchors
			if (m->confidence == 1) {
				continue;
			}

			if (twins(m, &summary)) {
				i_am_a_twin = m->twin = true;
			}

			for (cLinkedList::Iterator iter(neighbors); !iter.end();
			     iter++) {
				nghbor_info *k = (nghbor_info *) iter();

				// Skip anchors
				if (k->confidence == 1) {
					continue;
				}

				if (m->idx != k->idx && twins(m, k)) {
					m->twin = k->twin = true;
				}
			}
		}
	}
}

void Node_EuclRefine::do_triangulation(void *arg)
{
	flops++;
	int n = neighbors.length();
	assert(n > nr_dims);
	FLOAT** pos_list = new FLOAT*[n + 1];
	FLOAT* range_list = new FLOAT[n + 1];
	FLOAT* weights = new FLOAT[n];
	Position pos;

#ifndef NDEBUG
	EV << node[me].ID << ": triangulate with nodes (out of " << n << "):";
#endif
	int i = 0;
	FLOAT sum_conf = 0;
	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();
		double w = neighbor->twin ? LOW_CONF / 8 : neighbor->confidence;

		pos_list[i] = neighbor->position;
		range_list[i] = neighbor->distance;
		weights[i] = w;
		sum_conf += w;
		i++;
#ifndef NDEBUG
		EV << ' ' << node[neighbor->idx].ID << (neighbor->
							twin ? "t" : "") << '@'
		    << neighbor->distance;
#endif
	}
#ifndef NDEBUG
	EV << "\n";
#endif

	pos_list[i] = pos;

	FLOAT res = triangulate(i, pos_list, range_list, weights, me);
	//FLOAT res = savvides_minmax( i, pos_list, range_list, weights, me);

//      wait(i * nr_dims * nr_dims * msec);

	// Filter out moves that violate the convex constraints imposed
	// by (distant) anchors and neighbors
	if (res >= 0 && !inside_rectangle(pos)) {
		logprintf("Convex constraint violated\n");
		target_based_triangulation();
		res = -2;
	} else if (res >= 0 && !inside_neighbors_range(pos)) {
		res = -3;
	}

	if (res < 0 || res > range) {
		if (confidence > ZERO_CONF) {
			confidence = ZERO_CONF;
			status = STATUS_BAD;

			resetTimer(bc_position);
			seqno[MSG_POSITION]++;
		}
	} else {
		// Anchors must have a solid confidence compared to unknowns, so bound
		// derived confidence
		double conf = Min(0.5, sum_conf / i);

		// Only take action on significant moves
		if (distance(position, pos) > accuracy) {
			bool significant = false;


			logprintf
			    ("simtime %f old %3.2f %3.2f new %3.2f %3.2f\n",
			     simTime(), position[0], position[1], pos[0],
			     pos[1]);

			if (res > 1.1 * residu && confidence > 1.1 * LOW_CONF) {
				conf = LOW_CONF;
				// significant = true;
			}
			// We accept bad moves occasionally to get out of local minima
			if (res < residu || uniform(0, 1) < ACCEPT) {
				// record significant improvement and tell others
				memmove(position, pos, sizeof(Position));
				residu = res;
#ifndef NDEBUG
				if (res < residu) {
					EV << node[me].
					    ID << ": UPDATE pos to " <<
					    pos2str(position) << " (" << 100 *
					    distance(position,
						     node[me].true_pos) /
					    range << "% error)" << "\n";
				} else {
					EV << node[me].
					    ID << ": ESCAPE pos to " <<
					    pos2str(position) << " (" << 100 *
					    distance(position,
						     node[me].true_pos) /
					    range << "% error)" << "\n";
				}
#endif
				significant = true;
			} else {
#ifndef NDEBUG
				EV << node[me].
				    ID << ": REJECT move (" << res << " > " <<
				    residu << ")\n";
#endif
			}
			// Go tell others. Don't send message out directly, but use
			// retransmit queue instead. This will reduce the number of
			// xmitted msgs since multiple (buffered) incoming msgs
			// then yield one outgoing msg.

			if (significant) {
				resetTimer(bc_position);
				seqno[MSG_POSITION]++;
			}
		}
		confidence = (3 * confidence + conf) / 4;
		status = confidence > CONF_THR ? STATUS_POSITIONED : STATUS_BAD;
	}
	delete[] pos_list;
	delete[] range_list;
	delete[] weights;
}


void Node_EuclRefine::sendPosition(void *arg)
{
	// Maybe we're only interested in Hop-TERRAIN this time.
	if (!do_2nd_phase)
		return;

	// Wait some time for hop-TERRAIN to stabilize
#if 1
	if (bc_pos_delay > 0) {
		bc_pos_delay--;
#else
	if (simTime() < 99) {
#endif
		resetTimer(bc_position);
		return;
	}

	if ((status != STATUS_ANCHOR) && neighbors.length() <= nr_dims) {
		// Find out if I am a sound node
		bool* sound = new bool[num_nodes];

		for (int n = 0; n < num_nodes; n++)
			sound[n] = false;

		for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
			sound[((nghbor_info *) iter())->idx] = true;
		}
		/*niels        for (cLinkedList::Iterator iter(targets); !iter.end(); iter++) {
		   sound[((link_info *) iter())->last_hop_idx] = true;
		   } */

		int cnt = 0;
		for (int n = 0; n < num_nodes; n++)
			if (sound[n])
				cnt++;

		if (cnt <= nr_dims) {
			return;
		}
		delete[] sound;
	}

	if (refine_limit > -1 && refine_count >= refine_limit)
		return;

	refine_count++;

	cMessage *msg = new cMessage("POSITION", MSG_POSITION);
	msg->addPar("confidence") = confidence;
	add_struct(msg, "pos", position);

	if (summary_update) {
		msg->addPar("#neighbors") = summary.nr_nghbrs;
		add_array(msg, "id", summary.nghbr_idx, summary.nr_nghbrs);
		summary_update = false;
	} else {
		msg->addPar("#neighbors") = 0;
	}

	char tmp[100];
	sprintf(tmp, "p=%d,%d;i=ball_vs",
		(int) (100 * position[0] / sqrt(area)),
		(int) (100 * position[1] / sqrt(area)));
	setDisplayString(dispSUBMOD, (const char *) tmp, true);

	send(msg);
}


void Node_EuclRefine::update_rectangle(link_info * anchor)
{
	// intersect anchor's "square" with existing convex rectangle
	for (int d = 0; d < nr_dims; d++) {
		FLOAT left = anchor->position[d] - anchor->min_hops * range;
		FLOAT right = anchor->position[d] + anchor->min_hops * range;

		if (!valid_rectangle) {
			rectangle.min[d] = left;
			rectangle.max[d] = right;
		} else {
			rectangle.min[d] = Min(rectangle.min[d], left);
			rectangle.max[d] = Max(rectangle.max[d], right);
		}
	}
	valid_rectangle = true;
}


bool Node_EuclRefine::inside_rectangle(Position pos)
{
	return true;
	assert(valid_rectangle);

	// Check if this position fits in the convex rectangle
	for (int d = 0; d < nr_dims; d++) {
		if (pos[d] < rectangle.min[d] || pos[d] > rectangle.max[d]) {
			return false;
		}
	}
	return true;
}


bool Node_EuclRefine::inside_neighbors_range(Position pos)
{
	for (cLinkedList::Iterator iter(neighbors); !iter.end(); iter++) {
		nghbor_info *neighbor = (nghbor_info *) iter();

		if (neighbor->confidence > 2 * LOW_CONF &&
		    distance(pos, neighbor->position) > range) {
			return false;
		}
	}
	return true;
}

bool Node_EuclRefine::twins(nghbor_info * a, nghbor_info * b)
{
	if (a->nr_nghbrs != b->nr_nghbrs)
		return false;

	bool* nghbr_a = new bool[num_nodes];

	for (int i = 0; i < num_nodes; i++) {
		nghbr_a[i] = false;
	}
	for (int i = 0; i < a->nr_nghbrs; i++) {
		nghbr_a[a->nghbr_idx[i]] = true;
	}

	if (!nghbr_a[b->idx]) {
		delete[] nghbr_a;
		return false;
	}

	for (int i = 0; i < b->nr_nghbrs; i++) {
		int n = b->nghbr_idx[i];

		if (n != a->idx && !nghbr_a[n]) {
			delete[] nghbr_a;
			return false;
		}
	}
	delete[] nghbr_a;
	return true;
}
