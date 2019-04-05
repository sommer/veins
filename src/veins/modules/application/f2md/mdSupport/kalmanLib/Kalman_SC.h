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
#ifndef __VEINS_Kalman_SC_SC
#define __VEINS_Kalman_SC_SC
#include <math.h>
#include <iostream>
#include "KalmanFilterJ_SC.h"
using namespace std;

class Kalman_SC {

private:	
	void setT(float T);
	void setConfidence(float CX, float CY);

	float pos = 0;
	float A[KLM_N_SC][KLM_N_SC];
	float B[KLM_N_SC][KLM_M_SC];
	float R[KLM_N_SC][KLM_N_SC];
	float P0[KLM_N_SC][KLM_N_SC]; 
	float X0[KLM_N_SC];

	float U[KLM_N_SC];

	bool init = false;
	
public:	
	KalmanFilterJ_SC kalmanFilterJ_SC;
	Kalman_SC();
	bool isInit();
	void setInitial(float _X, float _Y);
	void getDeltaPos(float T, float _X, float _Y, float CX, float CY, float * Delta);
	void getDeltaPos(float T, float _X, float _Y, float _AX, float _AY, float CX, float CY, float * Delta);

};
#endif




