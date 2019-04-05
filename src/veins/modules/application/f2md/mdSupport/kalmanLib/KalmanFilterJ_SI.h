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

#ifndef __VEINS_KalmanFilterJ_SI
#define __VEINS_KalmanFilterJ_SI
#include <iostream>
#include "MatrixOp_SI.h"
using namespace std;

class KalmanFilterJ_SI {

	public:

		int n;
		int m; 

		float A[KLM_N_SI][KLM_N_SI];
		float AT[KLM_N_SI][KLM_N_SI]; 

		float B[KLM_N_SI][KLM_M_SI]; 

		float H[KLM_N_SI][KLM_N_SI]; 
		float HT[KLM_N_SI][KLM_N_SI]; 
		float Q[KLM_N_SI][KLM_N_SI];
		float R[KLM_N_SI][KLM_N_SI];
		float I[KLM_N_SI][KLM_N_SI]; 

		float X[KLM_N_SI]; 
		float P[KLM_N_SI][KLM_N_SI]; 
		float K[KLM_N_SI][KLM_N_SI]; 

		float X0[KLM_N_SI]; 
		float P0[KLM_N_SI][KLM_N_SI]; 

		float Temp_1[KLM_N_SI];
		float Temp_2[KLM_N_SI]; 
		float Temp_3[KLM_N_SI]; 


		float TempN_1[KLM_N_SI][KLM_N_SI]; 
		float TempN_2[KLM_N_SI][KLM_N_SI]; 
		float TempN_3[KLM_N_SI][KLM_N_SI]; 
		float TempN_4[KLM_N_SI][KLM_N_SI]; 

		MatrixOp_SI matrixOp_SI = MatrixOp_SI();
		

		KalmanFilterJ_SI();

		void setFixed ( float _A [][KLM_N_SI], float  _H [][KLM_N_SI], float _Q [][KLM_N_SI], float _R [][KLM_N_SI]);

		void setFixed ( float _A [][KLM_N_SI], float  _H [][KLM_N_SI], float _Q [][KLM_N_SI], float _R [][KLM_N_SI] , float _B [][KLM_M_SI]);


		void setA(float _A [][KLM_N_SI]);

		void setB(float _B [][KLM_M_SI]);

		void setR(float _R [][KLM_N_SI]);

		void setInitial( float  _X0[], float _P0[][KLM_N_SI] );

		void predict ( void );

		void predict( float  U []);
		void correct ( float Z []);

};

#endif



