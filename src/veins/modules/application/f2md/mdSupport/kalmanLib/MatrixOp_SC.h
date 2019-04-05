/*******************************************************************************
 * @author  Joseph Kamel
 * @email   josephekamel@gmail.com
 * @date    28/11/2018
 * @version 2.0
 *
 * SCA (Secure Cooperative Autonomous systems)
 * Copyright (c) 2013, 2018 Institut de Recherche Technologique SystemX
 * All rights reserved.
 *******************************************************************************/
#ifndef __VEINS_MatrixOp_SC
#define __VEINS_MatrixOp_SC

#include <algorithm>
#include <memory.h>
#include <iostream>

#define KLM_N_SC 2
#define KLM_M_SC 1

class MatrixOp_SC {

private:
	float determinant(float ** A, int n);
	void adjoint(float ** A, float ** adj, int N);
public:
	MatrixOp_SC();
	void multiply(float M1[][KLM_N_SC], float M2[][KLM_N_SC], float Mret[][KLM_N_SC], int r1,  int c2,  int c1);

	void multiply1D(float M1[][KLM_M_SC], float M2[], float Mret[], int r);

	void multiply21D(float M1[][KLM_N_SC], float * M2, float * Mret, int r1,  int c1);
	
	void add(float M1[][KLM_N_SC], float M2[][KLM_N_SC], float Mret[][KLM_N_SC], int r,  int c);
	void add1D(float * M1, float * M2, float * Mret, int r);
	
	void substract(float M1[][KLM_N_SC], float M2[][KLM_N_SC], float Mret[][KLM_N_SC], int r,  int c);
	void substract1D(float * M1, float * M2, float * Mret, int r);

	void transpose(float M[][KLM_N_SC], float  Mret[][KLM_N_SC],  int r,  int c);
	void cofactor(float ** A, float ** temp, int p, int q, int n);
	void inverse(float  A[][KLM_N_SC], float inverse[][KLM_N_SC], int N);

	void copy(float  M[][KLM_N_SC], float  Mret[][KLM_N_SC], int r,  int c);
	void copyM(float M[][KLM_M_SC], float  Mret[][KLM_M_SC], int r,  int c);

	void copy(float * M, float * Mret, int r);

	void printMat(std::string Sym, float A [][KLM_N_SC], int r, int c);
	void printVec(std::string Sym, float * A, int r);
};
#endif




