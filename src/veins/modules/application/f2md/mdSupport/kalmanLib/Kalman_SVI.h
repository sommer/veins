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
#ifndef __VEINS_Kalman_SVI_SVI
#define __VEINS_Kalman_SVI_SVI
#include <math.h>
#include <iostream>
#include "KalmanFilterJ_SVI.h"
using namespace std;

class Kalman_SVI {

private:	
	void setT(float T);
	void setConfidence(float CX, float CY, float CVX, float CVY);
	float A[KLM_N_SVI][KLM_N_SVI];
	float B[KLM_N_SVI][KLM_M_SVI];
	float R[KLM_N_SVI][KLM_N_SVI];
	float P0[KLM_N_SVI][KLM_N_SVI]; 
	float X0[KLM_N_SVI];

	float U[KLM_N_SVI];

	bool init = false;
	
public:	
	KalmanFilterJ_SVI kalmanFilterJ_SVI;
	Kalman_SVI();
	bool isInit();
	void setInitial(float _X, float _Y, float _VX, float _VY);
	void getDeltaPos(float T, float _X, float _Y, float _VX, float _VY, float CX, float CY, float CVX, float CVY, float * Delta);
	void getDeltaPos(float T, float _X, float _Y, float _VX, float _VY, float _AX, float _AY, float CX, float CY, float CVX, float CVY, float * Delta);

};
#endif




