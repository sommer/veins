/********************************
	File:		cores.h
	Project:	pos_test
	Author:		Chris Savarese
	Date:		2/28/01
 ********************************/

#ifndef __cores__h
#define __cores__h

#include "main.h"

// data structures


// function declarations
int triangulate(struct myParams *params, int neighbor_count, float **positions,
		float *ranges, float *weights, float *new_loc, float *dop);
void linearize_positions(int dim, int neighbor_count, float **positions,
			 float *ranges, float **a, float *b);
int hoptriangulate(struct myParams *params, int neighbor_count,
		   float **positions, float *ranges, float *weights,
		   float *new_loc, float *est_range);
void hoplinearize_positions(int dim, int neighbor_count, float **positions,
			    float *ranges, float **a, float *b);
int ls_trad(float **a, float *b, long m, long n, float *new_loc);
float *ls_qr(float **a, float *b, long m, long n);
float *ls_svd(float **a, float *b, long m, long n);
float *mmse(struct myParams *params, struct myScenario *scenario,
	    int target_id);
void apply_weights(float **a, float *b, float *w, long m, long n);
float compute_dop(float **a, float *b, long m, long n);
void triang_err_msg(struct myParams *params);

#endif
