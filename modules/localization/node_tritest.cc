#include "PositifLayer.h"
#include <string.h>

#define TIMER_COUNT 0

// Class for testing Triangulation and MinMax independently of the ranging algorithms.
// rangevariance is used to determine the error. The measured range is taken from a truncated
// normal distribution with stddev==rv and mean==the actual range.
// This is then multiplied by var1 to allow for a bias in the range result.
// The var0 nearest anchors are used. ('nearest' determined by the calculated range, including the error)



class Node_TriTest:public PositifLayer {
      public:
	Module_Class_Members(Node_TriTest, PositifLayer, 0)
	    // Implement Node's abstract functions.
	virtual void init(void);
	virtual void handleTimer(timer_info * timer);
	virtual void handleStartMessage(cMessage * msg);
	virtual void handleMessage(cMessage * msg, bool newNeighbor);
	virtual void handleStopMessage(cMessage * msg);
};

Define_Module_Like(Node_TriTest, PositifLayer);

void Node_TriTest::init(void)
{
	algorithm = 6;		// Output on raw data line only.
	version = 0;

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

// Dispatch this timer to the proper handler function.
void Node_TriTest::handleTimer(timer_info * timer)
{
}

void Node_TriTest::handleMessage(cMessage * msg, bool newNeighbor)
{
}

void Node_TriTest::handleStartMessage(cMessage * msg)
{
	int used_anchors = (int) var0;
	FLOAT *pos_list[used_anchors + 1];
	FLOAT range_list[used_anchors + 1];
	FLOAT real_range_list[used_anchors + 1];	// Used for debug output only
	int idx_list[used_anchors];

	if (node[me].anchor)
		return;

	int i = 0;
	for (int j = 0; j < num_nodes; j++) {
		if (!node[j].anchor)
			continue;

		int store_at = -1;
		FLOAT range = genk_truncnormal(1,
					       distance(node[me].true_pos,
							node[j].true_pos),
					       distance(node[me].true_pos,
							node[j].true_pos) *
					       range_variance);
		range *= (1.0 + var1);	// Allow for a bias in the error

		if (i < used_anchors) {
			store_at = i;
			i++;
		} else {
			for (int k = 0; k < used_anchors; k++)
				if ((store_at < 0 && range_list[k] > range)
				    || (store_at >= 0
					&& range_list[k] >
					range_list[store_at])) {
					store_at = k;
				}
		}

		if (store_at >= 0) {
			pos_list[store_at] = node[j].true_pos;
			range_list[store_at] = range;
			real_range_list[store_at] =
			    distance(node[me].true_pos, node[j].true_pos);
			idx_list[store_at] = j;
		}

	}

	// Calculate performance data for range error
	FLOAT sum_err = 0, sum_rel_err = 0, sum_abs_err = 0, sum_abs_rel_err =
	    0, sum_range = 0;
	for (i = 0; i < used_anchors; i++) {
		sum_err += range_list[i] - real_range_list[i];
		sum_rel_err +=
		    (range_list[i] - real_range_list[i]) / real_range_list[i];
		sum_abs_err += fabs(range_list[i] - real_range_list[i]);
		sum_abs_rel_err +=
		    fabs(range_list[i] -
			 real_range_list[i]) / real_range_list[i];
		sum_range += range_list[i];
	}
	node[me].perf_data.anchor_range_error = sum_err / used_anchors;
	node[me].perf_data.rel_anchor_range_error = sum_rel_err / used_anchors;
	node[me].perf_data.abs_anchor_range_error = sum_abs_err / used_anchors;
	node[me].perf_data.abs_rel_anchor_range_error =
	    sum_abs_rel_err / used_anchors;
	node[me].perf_data.anchor_range = sum_range / used_anchors;
	node[me].perf_data.anchor_range_error_count = used_anchors;


	Position next;
	pos_list[used_anchors] = next;

	FLOAT res;
	if (tri_alg == 0)
		res = triangulate(used_anchors, pos_list, range_list, NULL, me);
	else
		res =
		    savvides_minmax(used_anchors, pos_list, range_list, NULL,
				    me);

	if (0 <= res && res <= range) {
		memmove(position, next, sizeof(Position));
		status = STATUS_POSITIONED;	// After Hop-TERRAIN, a node is positioned regardless of its confidence.
		//      node[me].perf_data.phase1_err = distance(position,node[me].true_pos) / range;
	}

	logprintf(" p1->%4.0f,%4.0f (%1.1f) using ", position[0], position[1],
		  distance(position, node[me].true_pos));
	for (int j = 0; j < used_anchors; j++) {
		logprintf(" %d:(%4.0f,%4.0f)@%3.1f(r:%3.1f) ", idx_list[j],
			  pos_list[j][0], pos_list[j][1], range_list[j],
			  real_range_list[j]);
	}
	logprintf("\n");
}

void Node_TriTest::handleStopMessage(cMessage * msg)
{
	// send out final position for statistics
	cMessage *returnMsg = new cMessage("DONE", MSG_DONE);
	send(returnMsg);
}
