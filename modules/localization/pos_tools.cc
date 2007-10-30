/********************************
	File:		pos_tools.c
	Project:	pos_test
	Author:		Chris Savarese
	Date:		3/14/01
********************************/  
    
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "pos_tools.h"

// generates a rand whole number less then max
long randnum(long max) 
{
	return ((long) (max * rand() / RAND_MAX));
}

// generates a normally distributed random value with zero mean and var variance
float nrand(float var) 
{
	float u1, u2;		// random values on U[0,1]
	float x;		// N(0,1) random variable
	u1 = rand() / (float) RAND_MAX;
	u2 = rand() / (float) RAND_MAX;
	x = (float) (sqrt(-2 * log(u1)) * cos(2 * PI * u2));
	return ((float) sqrt(var) * x);
}

/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */ 
float **matrix(long nrl, long nrh, long ncl, long nch) 
{
	long i, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
	float **m;
	
	/* allocate pointers to rows */ 
	m = (float **) malloc((size_t) ((nrow + NR_END) * sizeof(float *)));
	if (!m)
		nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;
	
	/* allocate rows and set pointers to them */ 
	m[nrl] =
		(float *) malloc((size_t) ((nrow * ncol + NR_END) * sizeof(float)));
	if (!m[nrl])
		nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;
	for (i = nrl + 1; i <= nrh; i++)
		m[i] = m[i - 1] + ncol;
	
	/* return pointer to array of pointers to rows */ 
	return m;
}

/* allocate an int matrix with subscript range m[nrl..nrh][ncl..nch] */ 
int **imatrix(long nrl, long nrh, long ncl, long nch) 
{
	long i, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
	int **m;
	
	/* allocate pointers to rows */ 
	m = (int **) malloc((size_t) ((nrow + NR_END) * sizeof(int *)));
	if (!m)
		nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;
	
	/* allocate rows and set pointers to them */ 
	m[nrl] =
		(int *) malloc((size_t) ((nrow * ncol + NR_END) * sizeof(int)));
	if (!m[nrl])
		nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;
	for (i = nrl + 1; i <= nrh; i++)
		m[i] = m[i - 1] + ncol;
	
	/* return pointer to array of pointers to rows */ 
	return m;
}

/* Numerical Recipes standard error handler */ 
void nrerror(const char error_text[]) 
{
	fprintf(stderr, "Numerical Recipes run-time error...\n");
	fprintf(stderr, "%s\n", error_text);
	fprintf(stderr, "...now exiting to system...\n");
	exit(1);
}

/* free a float matrix allocated by matrix() */ 
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch) 
{
	free((FREE_ARG) (m[nrl] + ncl - NR_END));
	free((FREE_ARG) (m + nrl - NR_END));
}

/* free an int matrix allocated by imatrix() */ 
void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch) 
{
	free((FREE_ARG) (m[nrl] + ncl - NR_END));
	free((FREE_ARG) (m + nrl - NR_END));
}

/* print a float matrix allocated by matrix() */ 
void print_matrix(float **m, long nrl, long nrh, long ncl, long nch) 
{
	long i, j, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
	for (i = 0; i < nrow; i++) {
		for (j = 0; j < ncol; j++)
			printf("%f ", m[i][j]);
		printf("\n");
	}
}

/*print an integer vector allocated by ivector() */ 
void print_ivector(int *vv, long nl, long nh) 
{
	long i, n = nh - nl + 1;
	for (i = 0; i < n; i++)
		printf("%d ", vv[i]);
	printf("\n");
}

/* write a float matrix allocated by matrix() */ 
void set_matrix(float **m, long nrl, long nrh, long ncl, long nch) 
{
	long i, j, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
	for (i = 0; i < nrow; i++) {
		for (j = 0; j < ncol; j++)
			m[i][j] = (float) i + j;
	} 
}

/* zero a float matrix allocated by matrix() */ 
void zero_matrix(float **m, long nrl, long nrh, long ncl, long nch) 
{
	long i, j, nrow = nrh - nrl + 1, ncol = nch - ncl + 1;
	for (i = 0; i < nrow; i++) {
		for (j = 0; j < ncol; j++)
			m[i][j] = 0;
	}
}

/* allocate a float vector with subscript range v[nl..nh] */ 
float *vector(long nl, long nh) 
{
	float *v;
	v = (float *)
		malloc((size_t) ((nh - nl + 1 + NR_END) * sizeof(float)));
	if (!v)
		nrerror("allocation failure in vector()");
	return v - nl + NR_END;
}

/* free a float vector allocated with vector() */ 
void free_vector(float *v, long nl, long nh) 
{
	free((FREE_ARG) (v + nl - NR_END));
}

/* allocate an int vector with subscript range v[nl..nh] */ 
int *ivector(long nl, long nh) 
{
	int *v;
	v = (int *) malloc((size_t) ((nh - nl + 1 + NR_END) * sizeof(int)));
	if (!v)
		nrerror("allocation failure in ivector()");
	return v - nl + NR_END;
}

/* free an int vector allocated with ivector() */ 
void free_ivector(int *v, long nl, long nh) 
{
	free((FREE_ARG) (v + nl - NR_END));
}

//find the inverse of matrix 'a' which will be overwritten
// result returned in y. y must be allocated as an (0,N-1,0,N-1) matrix externally
// returns 1 if successful, 0 if error
int inv_matrix(float **a, long N, float **y) 
{
	int i, j, *indx;
	float d, *col;
	col = vector(0, N - 1);
	indx = ivector(0, N - 1);
	if (!ludcmp(a, N, indx, &d))
	{
		free_vector(col, 0, N - 1);
		free_ivector(indx, 0, N - 1);
		return (0);
	}
	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++)
			col[i] = 0.0;
		col[j] = 1.0;
		lubksb(a, N, indx, col);
		for (i = 0; i < N; i++)
			y[i][j] = col[i];
	}
	free_vector(col, 0, N - 1);
	free_ivector(indx, 0, N - 1);
	return (1);
}

// returns the transpose of matrix a without affecting a.  a has dimensions mxn.
// result returned in aT.  aT must be allocated externally as a (0,n-1,0,m-1) matrix.
void transpose_matrix(float **a, long m, long n, float **aT) 
{
	int i, j;
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			aT[j][i] = a[i][j];
}

// returns the product of matricies a and b.  does not change a or b.  dimensions of return matrix are mxn. inner product dimension is l.
// warning: assumed that inner dimensions of product match (else a runtime error will result)
// returns result in x.  x must be allocated externally as a (0,m-1,0,n-1) matrix.
void mult_matrix(float **a, float **b, long m, long n, long l, float **x) 
{
	int i, j, k;
	zero_matrix(x, 0, m - 1, 0, n - 1);
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			for (k = 0; k < l; k++)
				x[i][j] += a[i][k] * b[k][j];
}

// returns 1 if successful, 0 if error
int ludcmp(float **a, int n, int *indx, float *d) 
{
	int i, imax, j, k;
	float big, dum, sum, temp;
	float *vv;
	vv = vector(0, n - 1);
	*d = 1.0;		/*No row interchanges yet. */
	for (i = 0; i < n; i++) {	/*Loop over rows to get the implicit scaling information */
		big = 0.0;
		for (j = 0; j < n; j++)
			if ((temp = (float) fabs(a[i][j])) > big)
				big = temp;
		
//              if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
		if (big == 0.0)
		{
			free_vector(vv, 0, n - 1);
			return (0);
		}
		vv[i] = (float) 1.0 / big;
	} 
	/*This is the loop over columns of Crout's method. */ 
	for (j = 0; j < n; j++) {
		for (i = 0; i < j; i++) {
			sum = a[i][j];
			for (k = 0; k < i; k++)
				sum -= a[i][k] * a[k][j];
			a[i][j] = sum;
		}
		
		/*Initialize for the search for largest pivot element. */ 
		big = 0.0;
		imax = j;
		
		/*This is i = j of equation (2.3.12/13) and i = j+1..N */ 
		for (i = j; i < n; i++) {
			sum = a[i][j];
			for (k = 0; k < j; k++)
				sum -= a[i][k] * a[k][j];
			a[i][j] = sum;
			if ((dum = vv[i] * (float) fabs(sum)) >= big) {
				big = dum;
				imax = i;
			}
		} if (j != imax) {	/*Do we need to interchange rows? */
			for (k = 0; k < n; k++) {	/*Yes, do so... */
				dum = a[imax][k];
				a[imax][k] = a[j][k];
				a[j][k] = dum;
			}
			*d = -(*d);	/*...and change the parity of d. */
			vv[imax] = vv[j];	/*Also interchange the scale factor. */
		}
		indx[j] = imax;
		if (a[j][j] == 0.0)
			a[j][j] = (float) TINY;
		
		/*
		  If the pivot element is zero the matrix is singular (at least to the 
		  precision of the algorithm). For some applications on singular matrices, 
		  it is desirable to substitute TINY for zero.
		*/ 
		if (j != (n - 1)) {	/*Now, finally, divide by the pivot element. */
			dum = (float) 1.0 / (a[j][j]);
			for (i = j + 1; i < n; i++)
				a[i][j] *= dum;
		}
	}			/*Go back for the next column in the reduction. */
	free_vector(vv, 0, n - 1);
	return (1);
}

/*Solves the set of n linear equations A*X = B. Herea[0..n-1][0..n-1] is input, 
  not as the matrix A but rather as its LU decomposition, determined by the routine ludcmp.
  indx[1..n] is input as the permutation vector returned by ludcmp. b[1..n] is input as
  the right-hand side vector B, and returns with the solution vector X. a, n, and indx 
  are not modified by this routine and can be left in place for successive calls with 
  different right-hand sides b. This routine takes into account the possibility that 
  will begin with many zero elements, so it is efficient for use in matrix inversion. 
*/ 
void lubksb(float **a, int n, int *indx, float b[]) 
{
	int i, ii = 0, ip, j;
	float sum;
	for (i = 0; i < n; i++) {
		
		/*When ii is set to a positive value, it will become the index of the first 
		  nonvanishing element of b. We now do the forward substitution, equation 
		  (2.3.6). The only new wrinkle is to unscramble the permutation as we go. */ 
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii)
			for (j = ii - 1; j <= i - 1; j++)
				sum -= a[i][j] * b[j];
		
		else if (sum)
			ii = i + 1;
		
		/*A nonzero element was encountered, so from now on we will 
		  have to do the sums in the loop above. */ 
		b[i] = sum;
	}
	for (i = (n - 1); i >= 0; i--) {	/*Now we do the backsubstitution, equation (2.3.7). */
		sum = b[i];
		for (j = i + 1; j < n; j++)
			sum -= a[i][j] * b[j];
		b[i] = sum / a[i][i];	/*Store a component of the solution vector X. */
	}			/*All done! */
}

/*Q-R Decomposition of matrix A */ 
// returns 1 if successful, 0 if error
int qrdcmp(float **a, float **q, float **r, long nrow, long ncol) 
{
	long i, j, k;
	for (i = 0; i < ncol; i++) {
		for (j = 0; j < nrow; j++)
			q[j][i] = a[j][i];
		for (j = 0; j < i; j++) {
			r[j][i] = 0;
			for (k = 0; k < nrow; k++)
				r[j][i] += q[k][j] * q[k][i];
			for (k = 0; k < nrow; k++)
				q[k][i] = q[k][i] - r[j][i] * q[k][j];
		}
		r[i][i] = 0;
		for (k = 0; k < nrow; k++)
			r[i][i] += q[k][i] * q[k][i];
		r[i][i] = (float) sqrt(r[i][i]);
		if (r[i][i] == 0)
			
//                      nrerror("Singularity observed. R[i][i] = 0. Quitting...\n");
			return (0);
		for (k = 0; k < nrow; k++)
			q[k][i] = q[k][i] / r[i][i];
	}
	return (1);
}

/* return x = inv(R)*trans(Q)*b by backsubstituting w/ cols of trans(Q)*/ 
// return parameter is pointer col.  col must be allocated externally as a (0,rDIM-1) vector.
// returns 1 if successful, 0 if error
int solveQRb(float **r, float **q, float *b, long qNODES, long rDIM,
	     float *col) 
{
	int i, j, *indx;
	float **y, d;
	y = matrix(0, rDIM - 1, 0, qNODES - 1);
	indx = ivector(0, rDIM - 1);
	if (!ludcmp(r, rDIM, indx, &d))
	{
		free_ivector(indx, 0, rDIM - 1);
		free_matrix(y, 0, rDIM - 1, 0, qNODES - 1);
		return (0);
	}
	for (j = 0; j < qNODES; j++) {
		for (i = 0; i < rDIM; i++)
			col[i] = q[j][i];	/*backsubs w/ cols of trans(q) */
		lubksb(r, rDIM, indx, col);
		for (i = 0; i < rDIM; i++)
			y[i][j] = col[i];
	}
	for (i = 0; i < rDIM; i++) {
		col[i] = 0;
		for (j = 0; j < qNODES; j++)
			col[i] += b[j] * y[i][j];
	}
	free_ivector(indx, 0, rDIM - 1);
	free_matrix(y, 0, rDIM - 1, 0, qNODES - 1);
	return (1);
}

// searches for target in array.  searching starts at array[start_ind] and stops at array[stop_ind], inclusive.
// the absolute index of the first occurence of target in array is returned. if target is not found, -1 is returned.
int find_array_int(int target, int array[], int start_ind, int stop_ind) 
{
	int i;
	for (i = start_ind; i <= stop_ind; i++)
		if (array[i] == target)
			return (i);
	return (-1);
}
