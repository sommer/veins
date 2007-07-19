/********************************
	File:		main.h
	Project:	pos_test
	Author:		Chris Savarese
	Date:		2/20/01
 ********************************/

#ifndef __main__h
#define __main__h

#include <stdio.h>
struct myScenario;
struct myParams;
#include "sim_control.h"

// constants
/*static const char *alg_str[] = {
	"Traditional least squares",	// algorithm 0
	"QR-based least squares",		// algorithm 1
	"SVD-based least squares",		// algorithm 2
	"MMSE" };						// algorithm 3
static const char *start_str[] = {
	"TERRAIN",						// algorithm 0
	"Hop-TERRAIN" };				// algorithm 1
static const char *conf_str[] = {
	"No",
	"Yes" };
static const char *verb_str[] = {
	"Summaries only",
	"Scenarios and summaries",
	"Full detail" };*/

// data structures
extern struct myScenario {
	float **true_positions;	// actual positions of all nodes 
	float **est_positions;	// estimated positions of all nodes
	float **true_ranges;	// actual ranges bewteen all pairs of nodes
	float **est_ranges;	// estimated ranges between all pairs of nodes
	int *anch_list;		// vector containing indexes of nodes that are anchors
	float *weights;		// contains weights for each node based on current confidence metric for each node
	int *valid_nodes;	// flag array: 1 implies that this node has a valid position estimate, 0 implies invalid
} scenario;

extern struct myParams {
	int dim;		// dimension of geographical space
	int grid_bound;		// maximum coordinate value
	int vis;		// radio visibility range
	int alg_sel;		// algorithm selection: 0=traditional least squares, 1=qr-ls, 2=svd-ls, 3=MMSE
	int start_alg;		// start-up algorithm selection: 0=terrain, 1=hop-terrain
	int conf_mets;		// flag: 0=do not use confidence metrics in determining positions, 1=use them
	int net_size;		// total number of nodes in network
	int anch_size;		// number of anchor nodes in network
	int ref_its;		// maximum number of refinement iterations to perform
	int range_err_var;	// variance of noise in range measurements (normal distrubution)
	int verbosity;		// controls level of detail in report log. 0=summaries only, 1=scenarios and summaries, 2=full detail
	int sim_suite[SIM_SUITE_SIZE];	// array containing parameter ranges for simulation suite
	FILE *logfile;		// pointer to log file
	char filename[32];	// name of log file
} params;

// function declarations
void init_environment(struct myParams *params, struct myScenario *scenario);
void quit_program(struct myParams *params, struct myScenario *scenario);
void free_scenario(struct myParams *params, struct myScenario *scenario);
void allocate_scenario(struct myParams *params, struct myScenario *scenario);
void edit_single_params(struct myParams *params, struct myScenario *scenario);
void edit_suite_params(struct myParams *params);
int single_loop(void);
int suite_loop(void);
int fileio_loop(void);
int main_menu_loop(void);
void print_single_params(struct myParams *params);
void print_suite_params(struct myParams *params);
void print_single_menu(void);
void print_suite_menu(void);
void print_fileio_menu(void);
void print_main_menu(void);
int get_input(char msg[255]);
void openlog(struct myParams *params);

#endif
