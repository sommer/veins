/********************************
	File:		pos_tools.h
	Project:	pos_test
	Author:		Chris Savarese
	Date:		3/14/01
 ********************************/

#ifndef __pos_tools__h
#define __pos_tools__h

#include <omnetpp.h>

// constants
#ifndef RAND_MAX
#define RAND_MAX 0x7fff		// maximum return from rand()
#endif
#define FREE_ARG char*
#define NR_END 0
#define TINY 1.0e-20

// data structures


// function declarations
long randnum(long max);		// returns a randum whole number less than max
float nrand(float var);		// returns a normally distributed random variable with 0 mean and var variance
void nrerror(const char error_text[]);
float **matrix(long nrl, long nrh, long ncl, long nch);
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void print_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void print_ivector(int *vv, long nl, long nh);
void set_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void zero_matrix(float **m, long nrl, long nrh, long ncl, long nch);
void free_vector(float *v, long nl, long nh);
float *vector(long nl, long nh);
int *ivector(long nl, long nh);
void free_ivector(int *v, long nl, long nh);
int inv_matrix(float **a, long N, float **y);
void transpose_matrix(float **a, long m, long n, float **aT);
void mult_matrix(float **a, float **b, long m, long n, long l, float **x);
int ludcmp(float **a, int n, int *indx, float *d);
void lubksb(float **a, int n, int *indx, float b[]);
int qrdcmp(float **a, float **q, float **r, long nrow, long ncol);
int solveQRb(float **r, float **q, float *b, long qNODES, long rDIM,
	     float *col);
int find_array_int(int target, int array[], int start_ind, int stop_ind);
void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch);
int **imatrix(long nrl, long nrh, long ncl, long nch);

#endif
