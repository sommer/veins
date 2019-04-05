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
#ifndef __VEINS_Kalman_SI_SI
#define __VEINS_Kalman_SI_SI
#include <math.h>
#include <iostream>
#include "KalmanFilterJ_SI.h"
using namespace std;

class Kalman_SI {

private:	
	void setT(float T);
	void setConfidence(float CX, float CY);
	float A[KLM_N_SI][KLM_N_SI];
	float B[KLM_N_SI][KLM_M_SI];
	float R[KLM_N_SI][KLM_N_SI];
	float P0[KLM_N_SI][KLM_N_SI]; 
	float X0[KLM_N_SI];

	float U[KLM_N_SI];

	bool init = false;
	
public:	
	KalmanFilterJ_SI kalmanFilterJ_SI;
	Kalman_SI();
	bool isInit();
	void setInitial(float _X, float _Y);
	void getDeltaPos(float T, float _X, float _Y, float CX, float CY, float * Delta);
	void getDeltaPos(float T, float _X, float _Y, float _AX, float _AY, float CX, float CY, float * Delta);

};
#endif




