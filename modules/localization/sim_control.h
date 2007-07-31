/********************************
	File:		sim_control.h
	Project:	pos_test
	Author:		Chris Savarese
	Date:		7/06/01
 ********************************/

#ifndef __sim_control__h
#define __sim_control__h

#include "main.h"
// constants
#define SIM_SUITE_SIZE 31	// number of elements in params->sim_suite array

// enumerated indicies for params->sim_suite array
#define topology_trials 0	// number of trials to perform, where each trial has a new topology
#define ranges_trials 1		// number of trials to perform, where each trial has new range estimates, but the same topology
#define dim_2 2			// 2-D flag: 1 = perform 2-D trials, else don't
#define dim_3 3			// 3-D flag: 1 = perform 3-D trials, else don't
#define grid_bounds_min 4
#define grid_bounds_max 5
#define grid_bounds_step 6
#define vis_min 7
#define vis_max 8
#define vis_step 9
#define alg_sel_0 10		// traditional least squares flag: 1=perform ls_trad trials, else don't
#define alg_sel_1 11		// qr least squares
#define alg_sel_2 12		// svd least squares
#define alg_sel_3 13		// mmse
#define start_alg_0 14		// terrain, 1=perform terrain trials, else don't
#define start_alg_1 15		// hop-terrain, 1=perform hop-terrain trials, else don't
#define conf_mets_yes 16	// 1=perform trials with confidence metrics, else don't
#define conf_mets_no 17		// 1=perform trials without confidence metrics, else don't
#define net_size_min 18
#define net_size_max 19
#define net_size_step 20
#define anch_size_min 21
#define anch_size_max 22
#define anch_size_step 23
#define ref_its_min 24
#define ref_its_max 25
#define ref_its_step 26
#define range_err_var_min 27
#define range_err_var_max 28
#define range_err_var_step 29
#define verbosity_code 30	// verbosity param for entire simulation suite.  note, independent of single simulation mode verbosity param

// data structures

// function declarations
int perform_trial(int suppress_console_status, struct myParams *params,
		  struct myScenario *scenario, float *avg_pos_err_start,
		  float *avg_pos_err_ref);
void run_single_simulation(struct myParams *params,
			   struct myScenario *scenario);
void run_suite_simulation(const struct myParams *params);
float compute_estimated_position_error(int i, struct myParams *params,
				       struct myScenario *scenario);
void gen_new_top(int suppress_console_status, struct myParams *params,
		 struct myScenario *scenario);
void gen_new_range_ests(int suppress_console_status, struct myParams *params,
			struct myScenario *scenario);
void assign_anchors(int suppress_console_status, struct myParams *params,
		    struct myScenario *scenario);

#endif
