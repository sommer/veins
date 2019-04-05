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

#ifndef __VEINS_KalmanFilterJ_SC
#define __VEINS_KalmanFilterJ_SC
#include <iostream>
#include "MatrixOp_SC.h"
using namespace std;

class KalmanFilterJ_SC {

	public:

		int n;
		int m; 

		float A[KLM_N_SC][KLM_N_SC];
		float AT[KLM_N_SC][KLM_N_SC]; 

		float B[KLM_N_SC][KLM_M_SC]; 

		float H[KLM_N_SC][KLM_N_SC]; 
		float HT[KLM_N_SC][KLM_N_SC]; 
		float Q[KLM_N_SC][KLM_N_SC];
		float R[KLM_N_SC][KLM_N_SC];
		float I[KLM_N_SC][KLM_N_SC]; 

		float X[KLM_N_SC]; 
		float P[KLM_N_SC][KLM_N_SC]; 
		float K[KLM_N_SC][KLM_N_SC]; 

		float X0[KLM_N_SC]; 
		float P0[KLM_N_SC][KLM_N_SC]; 

		float Temp_1[KLM_N_SC];
		float Temp_2[KLM_N_SC]; 
		float Temp_3[KLM_N_SC]; 


		float TempN_1[KLM_N_SC][KLM_N_SC]; 
		float TempN_2[KLM_N_SC][KLM_N_SC]; 
		float TempN_3[KLM_N_SC][KLM_N_SC]; 
		float TempN_4[KLM_N_SC][KLM_N_SC]; 

		MatrixOp_SC matrixOp_SC = MatrixOp_SC();
		

		KalmanFilterJ_SC();

		void setFixed ( float _A [][KLM_N_SC], float  _H [][KLM_N_SC], float _Q [][KLM_N_SC], float _R [][KLM_N_SC]);

		void setFixed ( float _A [][KLM_N_SC], float  _H [][KLM_N_SC], float _Q [][KLM_N_SC], float _R [][KLM_N_SC] , float _B [][KLM_M_SC]);


		void setA(float _A [][KLM_N_SC]);

		void setB(float _B [][KLM_M_SC]);

		void setR(float _R [][KLM_N_SC]);

		void setInitial( float  _X0[], float _P0[][KLM_N_SC] );

		void predict ( void );

		void predict( float  U []);
		void correct ( float Z []);

};

#endif



