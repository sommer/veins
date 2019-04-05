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


#include "KalmanFilterJ_SI.h"
using namespace std;

KalmanFilterJ_SI::KalmanFilterJ_SI() {
	n = KLM_N_SI;
  m = KLM_M_SI;
}


void KalmanFilterJ_SI::setFixed( float _A  [][KLM_N_SI], float _H  [][KLM_N_SI], float _Q [][KLM_N_SI], float _R [][KLM_N_SI] ){
  matrixOp_SI.copy(A,_A,n,n);
  matrixOp_SI.copy(H,_H,n,n);
  matrixOp_SI.copy(Q,_Q,n,n);
  matrixOp_SI.copy(R,_R,n,n);

	for(int vari=0; vari<n;vari++){
		for(int varj=0; varj<n;varj++){
			if(vari==varj){
				I[vari][varj] = 1;
			}else{
				I[vari][varj] = 0;
			}
		}
	}
}


void KalmanFilterJ_SI::setFixed( float _A  [][KLM_N_SI], float _H  [][KLM_N_SI], float _Q [][KLM_N_SI], float _R [][KLM_N_SI], float _B [][KLM_M_SI] ){
  matrixOp_SI.copy(A,_A,n,n);
  matrixOp_SI.copy(H,_H,n,n);
  matrixOp_SI.copy(Q,_Q,n,n);
  matrixOp_SI.copy(R,_R,n,n);

  matrixOp_SI.copyM(B,_B,n,m);

  for(int vari=0; vari<n;vari++){
    for(int varj=0; varj<n;varj++){
      if(vari==varj){
        I[vari][varj] = 1;
      }else{
        I[vari][varj] = 0;
      }
    }
  }
}


void KalmanFilterJ_SI::setA(float _A [][KLM_N_SI]){
  matrixOp_SI.copy(A,_A,n,n);
}

void KalmanFilterJ_SI::setB(float _B [][KLM_M_SI]){
  matrixOp_SI.copyM(B,_B,n,m);
}


void KalmanFilterJ_SI::setR(float _R [][KLM_N_SI]){
  matrixOp_SI.copy(R,_R,n,n);
} 


void KalmanFilterJ_SI::setInitial( float _X0[], float _P0 [][KLM_N_SI]){
  matrixOp_SI.copy(X0,_X0,n);
  matrixOp_SI.copy(P0,_P0,n,n);
}



void KalmanFilterJ_SI::predict(void){

  matrixOp_SI.multiply21D(A,X0,X,n,n);

  matrixOp_SI.transpose(A,AT,n,n);

  matrixOp_SI.multiply(P0,AT,TempN_1,n,n,n);

  matrixOp_SI.multiply(A,TempN_1,TempN_2,n,n,n);

  matrixOp_SI.add(TempN_2,Q,P,n,n);
  

/*
  matrixOp_SI.printMat("A",A,n,n);
  matrixOp_SI.printVec("X0",X0,n);
  matrixOp_SI.printMat("P0",P0,n,n);
  matrixOp_SI.printVec("X",X,n);
  matrixOp_SI.printMat("AT",AT,n,n);
  matrixOp_SI.printMat("TempN_1",TempN_1,n,n);
  matrixOp_SI.printMat("TempN_2",TempN_2,n,n);
  matrixOp_SI.printMat("Q",Q,n,n);
  matrixOp_SI.printMat("P",P,n,n);
*/
  
}

void KalmanFilterJ_SI::predict( float  U[] ){

  matrixOp_SI.multiply21D(A,X0,Temp_1,n,n);
  matrixOp_SI.multiply1D(B,U,Temp_2,n);

  matrixOp_SI.add1D(Temp_1,Temp_2,X,n);

  matrixOp_SI.transpose(A,AT,n,n);

  matrixOp_SI.multiply(P0,AT,TempN_1,n,n,n);

  matrixOp_SI.multiply(A,TempN_1,TempN_2,n,n,n);

  matrixOp_SI.add(TempN_2,Q,P,n,n);
  

}

void KalmanFilterJ_SI::correct ( float Z[] ) {

  matrixOp_SI.transpose(H,HT,n,n);
  matrixOp_SI.multiply(P,HT,TempN_1,n,n,n);
  matrixOp_SI.multiply(H,TempN_1,TempN_2,n,n,n);
  matrixOp_SI.add(TempN_2,R,TempN_3,n,n);
  matrixOp_SI.inverse(TempN_3,TempN_4,n);
  matrixOp_SI.multiply(TempN_1,TempN_4,K,n,n,n);


  matrixOp_SI.multiply21D(H,X,Temp_1,n,n);
  matrixOp_SI.substract1D(Z,Temp_1,Temp_2,n);
  matrixOp_SI.multiply21D(K,Temp_2,Temp_3,n,n);
  matrixOp_SI.add1D(X,Temp_3,X,n);

;
  matrixOp_SI.multiply(K,H,TempN_1,n,n,n);
  matrixOp_SI.substract(I,TempN_1,TempN_2,n,n);
  matrixOp_SI.multiply(TempN_2,P,TempN_3,n,n,n);
  matrixOp_SI.copy(P,TempN_3,n,n);

  matrixOp_SI.copy(X0,X,n);
  matrixOp_SI.copy(P0,P,n,n);
}
