/********************************
	File:		cores.c
	Project:	pos_test
	Author:		Chris Savarese
	Date:		2/28/01
 ********************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cores.h"
#include "pos_tools.h"

// entry point.  performs triangulation using core algorithm and dimension specified by params.
// neighbor_count specifies the number of nodes being used to triangulate the target
// positions is a (neighbor_count x params->dim) matrix giving the x,y(,z) coordinates of the neighbors
// ranges is a (1 x neighbor_count) array containing the estimated ranges from the target to each of the neighbors
// weights is a (1 x neighbor_count) array containing the confidence weights associated with each of the neighbors
// returns two pointers: new_loc and dop.
// new_loc points to a (1 x params->dim) array containing the x,y(,z) coordinates of the target.
// new_loc must have been allocated externally.  only the pointer should be passed.
// dop contains the dillution of precision measurement, to be used to define a confidence metric
// returns 1 if successful triangulation, 0 if error
int triangulate(struct myParams *params, int neighbor_count, float **positions,
		float *ranges, float *weights, float *new_loc, float *dop)
{
	float **a, *b;

	a = matrix(0, neighbor_count - 2, 0, params->dim - 1);
	b = vector(0, neighbor_count - 2);

	switch (params->alg_sel)	// choose algorithm
	{
	case 0:		// Traditional least squares
		linearize_positions(params->dim, neighbor_count, positions,
				    ranges, a, b);
		*dop = compute_dop(a, b, neighbor_count - 1, params->dim);
		if (params->conf_mets)
			apply_weights(a, b, weights, neighbor_count - 1,
				      params->dim);
		if (!ls_trad(a, b, neighbor_count - 1, params->dim, new_loc)) {
			triang_err_msg(params);
			free_matrix(a, 0, neighbor_count - 2, 0,
				    params->dim - 1);
			free_vector(b, 0, neighbor_count - 2);
			return (0);
		}
		break;

	case 1:		// QR-based least squares
		printf("QR not implemented yet.\n");
		exit(1);
//              linearize_positions(params->dim,neighbor_count,positions,ranges,a,b);
//              x=ls_qr(a,b,neighbor_count-1,params->dim);              
		break;

	case 2:		// SVD-based least squares
		printf("SVD not implemented yet.\n");
		exit(1);
//              linearize_positions(params->dim,neighbor_count,positions,ranges,a,b);
//              x=ls_svd(a,b,neighbor_count-1,params->dim);
		break;

	case 3:		// minimum mean square error
		printf("MMSE not implemented yet.\n");
		exit(1);
//              x=mmse(a,b,neighbor_count-1,params->dim);       
		break;

	default:		// shouldn't ever get here
		printf("Error: Switch slip at alg_sel in triangulate().\n");
		exit(1);
	}			// switch

	// free data structures
	free_matrix(a, 0, neighbor_count - 2, 0, params->dim - 1);
	free_vector(b, 0, neighbor_count - 2);

	return (1);
}


// Koen: similar to triangulate but estimate hop distance too
int hoptriangulate(struct myParams *params, int neighbor_count,
		   float **positions, float *ranges, float *weights,
		   float *new_loc, float *est_range)
{
	float **a, *b, *c;

	if (neighbor_count < params->dim + 1)
		return 0;

	a = matrix(0, neighbor_count - 2, 0, params->dim);
	b = vector(0, neighbor_count - 2);
	c = vector(0, params->dim);

	switch (params->alg_sel)	// choose algorithm
	{
	case 0:		// Traditional least squares
		hoplinearize_positions(params->dim, neighbor_count, positions,
				       ranges, a, b);
		if (params->conf_mets)
			apply_weights(a, b, weights, neighbor_count - 1,
				      params->dim + 1);
		if (!ls_trad(a, b, neighbor_count - 1, params->dim + 1, c)
		    || c[params->dim] <= 0) {
			triang_err_msg(params);
			free_matrix(a, 0, neighbor_count - 2, 0, params->dim);
			free_vector(b, 0, neighbor_count - 2);
			free_vector(c, 0, params->dim);
			return (0);
		}
		memmove(new_loc, c, sizeof(float) * params->dim);
		*est_range = sqrt(c[params->dim]);
		break;

	case 1:		// QR-based least squares
		printf("QR not implemented yet.\n");
		exit(1);
		break;

	case 2:		// SVD-based least squares
		printf("SVD not implemented yet.\n");
		exit(1);
		break;

	case 3:		// minimum mean square error
		printf("MMSE not implemented yet.\n");
		exit(1);
		break;

	default:		// shouldn't ever get here
		printf("Error: Switch slip at alg_sel in triangulate().\n");
		exit(1);
	}			// switch

	// free data structures
	free_matrix(a, 0, neighbor_count - 2, 0, params->dim);
	free_vector(b, 0, neighbor_count - 2);
	free_vector(c, 0, params->dim);

	return (1);
}


// linearizes scenario for solution by least squares
// positions, ranges, a, and b data structures must be allocated externally.
// results returned in a and b
// positions is a (neighbor_count x dim) matrix
// ranges is a (neighbor_count x 1) vector
// a is a (neighbor_count-1 x dim) matrix
// b is a (neighbor_count-1 x 1) vector
void linearize_positions(int dim, int neighbor_count, float **positions,
			 float *ranges, float **a, float *b)
{
	int i, j;

	// linearize position and range information into A and b such that Ax=b
	// build A matrix as position matrix with last row subtracted from every other row
	// build b vector as (ri^2-rn^2-xi^2-yi^2-zi^2+xn^2+yn^2+zn^2) where i is the current row, and n is the last row
	for (i = 0; i < neighbor_count - 1; i++) {
		b[i] =
		    ranges[i] * ranges[i] - ranges[neighbor_count -
						   1] * ranges[neighbor_count -
							       1];

		for (j = 0; j < dim; j++) {
			a[i][j] =
			    positions[i][j] - positions[neighbor_count - 1][j];
			b[i] +=
			    positions[neighbor_count -
				      1][j] * positions[neighbor_count - 1][j]
			    - positions[i][j] * positions[i][j];
		}		// for j
	}			// for i

	// constant adjustment to correct equation Ax=b: multiply b by -.5
	for (i = 0; i < neighbor_count - 1; i++)
		b[i] = (float) -.5 * b[i];
}


// linearizes scenario for solution by least squares
// positions, ranges, a, and b data structures must be allocated externally.
// results returned in a and b
// positions is a (neighbor_count x dim) matrix
// ranges is a (neighbor_count x 1) vector
// a is a (neighbor_count-1 x dim+1) matrix
// b is a (neighbor_count-1 x 1) vector
void hoplinearize_positions(int dim, int neighbor_count, float **positions,
			    float *ranges, float **a, float *b)
{
	int i, j;

	// linearize position and range information into A and b such that Ax=b
	// build A matrix as position matrix with last row subtracted from every other row
	// build b vector as (ri^2-rn^2-xi^2-yi^2-zi^2+xn^2+yn^2+zn^2) where i is the current row, and n is the last row
	for (i = 0; i < neighbor_count - 1; i++) {
		a[i][dim] =
		    ranges[i] * ranges[i] - ranges[neighbor_count -
						   1] * ranges[neighbor_count -
							       1];

		for (j = 0; j < dim; j++) {
			a[i][j] =
			    2 * (positions[i][j] -
				 positions[neighbor_count - 1][j]);
			b[i] +=
			    positions[i][j] * positions[i][j] -
			    positions[neighbor_count -
				      1][j] * positions[neighbor_count - 1][j];
		}		// for j
	}			// for i
}


/********************************************************************************
 * Application of weights to position and range structures						*
 ********************************************************************************/
// applies the weights contained in w to a and b.
// a is a (mxn) matrix.
// b and w are (mx1) arrays.
void apply_weights(float **a, float *b, float *w, long m, long n)
{
// be careful...  linearization screwed up the order...
// for now, just assume linearize_positions() used the last node for linearization.  fix later to accomodate residue rotations
	int i, j;

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++)
			a[i][j] *= w[i];
		b[i] *= w[i];
	}
}				// apply_weights()


/********************************************************************************
 * Dillution of Precision														*
 ********************************************************************************/
// a is the matrix to be operated on, and has dimensions (m x n).  returns DOP value.
// a is not affected.
float compute_dop(float **a, float *b, long m, long n)
{
	int i;
	float **aT, **aTa, **aTainv;
	float dop_val;

	// allocate data structures
	aT = matrix(0, n - 1, 0, m - 1);
	aTa = matrix(0, n - 1, 0, n - 1);
	aTainv = matrix(0, n - 1, 0, n - 1);

	// perform matrix manipulations to get (A'A)^-1
	transpose_matrix(a, m, n, aT);
	mult_matrix(aT, a, n, n, m, aTa);
	inv_matrix(aTa, n, aTainv);

	// compute squared dop as sum of squared diagonal elements of (A'A)^-1
	dop_val = 0;
	for (i = 0; i < n; i++)
		dop_val += aTainv[i][i] * aTainv[i][i];

	free_matrix(aT, 0, n - 1, 0, m - 1);
	free_matrix(aTa, 0, n - 1, 0, n - 1);
	free_matrix(aTainv, 0, n - 1, 0, n - 1);

	return ((float) sqrt(fabs(dop_val)));
}				// compute_dop()


/********************************************************************************
 * Least Squares, traditional method: x = (A' * A)^-1 * A' * b					*
 ********************************************************************************/
// traditional least squares solution to Ax=b, where dimensions of A are mxn, dimensions of b are mx1, and dimensions of new_loc are nx1
// a and b should be constructed using linearize_positions()
// new_loc must be allocated externally.
// returns 1 if successful triangulation, 0 if error
int ls_trad(float **a, float *b, long m, long n, float *new_loc)
{
	int i, j;
	float **aT, **aTa, **aTainv, **aTaaT;

	// allocate structures
	aT = matrix(0, n - 1, 0, m - 1);
	aTa = matrix(0, n - 1, 0, n - 1);
	aTainv = matrix(0, n - 1, 0, n - 1);
	aTaaT = matrix(0, n - 1, 0, m - 1);

	// perform least squares computations
	transpose_matrix(a, m, n, aT);
	mult_matrix(aT, a, n, n, m, aTa);
	if (!inv_matrix(aTa, n, aTainv)) {
		free_matrix(aT, 0, n - 1, 0, m - 1);
		free_matrix(aTa, 0, n - 1, 0, n - 1);
		free_matrix(aTainv, 0, n - 1, 0, n - 1);
		free_matrix(aTaaT, 0, n - 1, 0, m - 1);
		return (0);
	}
	mult_matrix(aTainv, aT, n, m, n, aTaaT);

	for (i = 0; i < n; i++) {
		new_loc[i] = 0;
		for (j = 0; j < m; j++)
			new_loc[i] += aTaaT[i][j] * b[j];
	}

	// clean up and return
	free_matrix(aT, 0, n - 1, 0, m - 1);
	free_matrix(aTa, 0, n - 1, 0, n - 1);
	free_matrix(aTainv, 0, n - 1, 0, n - 1);
	free_matrix(aTaaT, 0, n - 1, 0, m - 1);

	return (1);		// successful triangulation
}				// ls_trad()



/********************************************************************************
 * Least Squares, using QR decomp and LU backsubstitution						*
 ********************************************************************************/

	/**************************
	 * THIS DOES NOT WORK YET *
	 **************************/

// QR least squares solution to Ax=b, where dimensions of A are mxn, dimensions of b are mx1, and dimensions of x are nx1
// a and b should be constructed using linearize_scenario()
// x allocated internally
// float *ls_qr(float **a, float *b, long m, long n)
// {
// 	int i;
// 	float **A, **Q, **R, *b, *x;
	
// 	// allocate matricies
// //	A = matrix(0,params->net_size-3,0,params->dim-1);
// //	Q = matrix(0,params->net_size-3,0,params->dim-1);
// //	R = matrix(0,params->dim-1,0,params->dim-1);
// //	b = vector(0,params->net_size-3);

// 	// allocate matricies
// 	A = matrix(0,params->net_size-3,0,params->dim-1);
// 	Q = matrix(0,params->net_size-3,0,params->net_size-3);
// 	R = matrix(0,params->net_size-3,0,params->dim-1);
// 	b = vector(0,params->net_size-3);

// 	//QR decomposition of matrix A
//   	qrdcmp(A,Q,R,params->net_size-2,params->dim);

// 	//Solve x = inv(R)*trans(Q)*b
// 	x = solveQRb(R,Q,b,params->net_size-2,params->dim);

// 	// debug
// 	printf("A:\n");
// 	print_matrix(A,0,params->net_size-3,0,params->dim-1);
// 	printf("\nb:\n");
// 	for (i=0;i<params->net_size-2;i++)
// 		printf("%f\n",b[i]);
// //	printf("\nQ:\n");
// //	print_matrix(Q,0,params->net_size-3,0,params->net_size-3);
// //	printf("\nR:\n");
// //	print_matrix(R,0,params->net_size-3,0,params->dim-1);

// 	return (0);
// }


/********************************************************************************
 * Least Squares using SVD														*
 ********************************************************************************/
// SVD least squares solution to Ax=b, where dimensions of A are mxn, dimensions of b are mx1, and dimensions of x are nx1
// a and b should be constructed using linearize_scenario()
// x allocated internally
// float *ls_svd(float **a, float *b, long m, long n)
// {
// 	return (0);
// }


/********************************************************************************
 * MMSE																			*
 ********************************************************************************/
// minimum mean square error triangulation solution
// x allocated internally
// float *mmse(struct myParams *params, struct myScenario *scenario, int target_id)
// {
// 	// shouldn't be passing params and scenario... set up externally, as in ls methods
// 	return (0);
// }


/********************************************************************************
 * triang_err_msg																*
 ********************************************************************************/
void triang_err_msg(struct myParams *params)
{
}				// triang_err_msg()
