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

#ifndef __VEINS_KalmanFilterJ_SVI
#define __VEINS_KalmanFilterJ_SVI
#include <iostream>
#include "MatrixOp_SVI.h"
using namespace std;

class KalmanFilterJ_SVI {

	public:

		int n;
		int m; 

		float A[KLM_N_SVI][KLM_N_SVI];
		float AT[KLM_N_SVI][KLM_N_SVI]; 

		float B[KLM_N_SVI][KLM_M_SVI]; 

		float H[KLM_N_SVI][KLM_N_SVI]; 
		float HT[KLM_N_SVI][KLM_N_SVI]; 
		float Q[KLM_N_SVI][KLM_N_SVI];
		float R[KLM_N_SVI][KLM_N_SVI];
		float I[KLM_N_SVI][KLM_N_SVI]; 

		float X[KLM_N_SVI]; 
		float P[KLM_N_SVI][KLM_N_SVI]; 
		float K[KLM_N_SVI][KLM_N_SVI]; 

		float X0[KLM_N_SVI]; 
		float P0[KLM_N_SVI][KLM_N_SVI]; 

		float Temp_1[KLM_N_SVI];
		float Temp_2[KLM_N_SVI]; 
		float Temp_3[KLM_N_SVI]; 

		float TempN_1[KLM_N_SVI][KLM_N_SVI]; 
		float TempN_2[KLM_N_SVI][KLM_N_SVI]; 
		float TempN_3[KLM_N_SVI][KLM_N_SVI]; 
		float TempN_4[KLM_N_SVI][KLM_N_SVI]; 

		MatrixOp_SVI matrixOp_SVI = MatrixOp_SVI();
		

		KalmanFilterJ_SVI();

		void setFixed ( float _A [][KLM_N_SVI], float  _H [][KLM_N_SVI], float _Q [][KLM_N_SVI], float _R [][KLM_N_SVI]);

		void setFixed ( float _A [][KLM_N_SVI], float  _H [][KLM_N_SVI], float _Q [][KLM_N_SVI], float _R [][KLM_N_SVI] , float _B [][KLM_M_SVI]);

		void setA(float _A [][KLM_N_SVI]);

		void setB(float _B [][KLM_M_SVI]);

		void setR(float _R [][KLM_N_SVI]);

		void setInitial( float  _X0[], float _P0[][KLM_N_SVI] );

		void predict ( void );

		void predict( float  U []);
		void correct ( float Z []);

};

#endif



