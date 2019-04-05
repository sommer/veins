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


#include "KalmanFilterJ_SVI.h"
using namespace std;

KalmanFilterJ_SVI::KalmanFilterJ_SVI() {
	n = KLM_N_SVI;
	m = KLM_M_SVI;
}


void KalmanFilterJ_SVI::setFixed( float _A  [][KLM_N_SVI], float _H  [][KLM_N_SVI], float _Q [][KLM_N_SVI], float _R [][KLM_N_SVI] ){
  matrixOp_SVI.copy(A,_A,n,n);
  matrixOp_SVI.copy(H,_H,n,n);
  matrixOp_SVI.copy(Q,_Q,n,n);
  matrixOp_SVI.copy(R,_R,n,n);

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


void KalmanFilterJ_SVI::setFixed( float _A  [][KLM_N_SVI], float _H  [][KLM_N_SVI], float _Q [][KLM_N_SVI], float _R [][KLM_N_SVI], float _B [][KLM_M_SVI] ){
  matrixOp_SVI.copy(A,_A,n,n);
  matrixOp_SVI.copy(H,_H,n,n);
  matrixOp_SVI.copy(Q,_Q,n,n);
  matrixOp_SVI.copy(R,_R,n,n);
  matrixOp_SVI.copyM(B,_B,n,m);

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


void KalmanFilterJ_SVI::setA(float _A [][KLM_N_SVI]){
  matrixOp_SVI.copy(A,_A,n,n);
}

void KalmanFilterJ_SVI::setB(float _B [][KLM_M_SVI]){
  matrixOp_SVI.copyM(B,_B,n,m);
}


void KalmanFilterJ_SVI::setR(float _R [][KLM_N_SVI]){
  matrixOp_SVI.copy(R,_R,n,n);
} 


void KalmanFilterJ_SVI::setInitial( float _X0[], float _P0 [][KLM_N_SVI]){
  matrixOp_SVI.copy(X0,_X0,n);
  matrixOp_SVI.copy(P0,_P0,n,n);
}

void KalmanFilterJ_SVI::predict(void){

  matrixOp_SVI.multiply21D(A,X0,X,n,n);

  matrixOp_SVI.transpose(A,AT,n,n);

  matrixOp_SVI.multiply(P0,AT,TempN_1,n,n,n);

  matrixOp_SVI.multiply(A,TempN_1,TempN_2,n,n,n);

  matrixOp_SVI.add(TempN_2,Q,P,n,n);
  

/*
  matrixOp_SVI.printMat("A",A,n,n);
  matrixOp_SVI.printVec("X0",X0,n);
  matrixOp_SVI.printMat("P0",P0,n,n);
  matrixOp_SVI.printVec("X",X,n);
  matrixOp_SVI.printMat("AT",AT,n,n);
  matrixOp_SVI.printMat("TempN_1",TempN_1,n,n);
  matrixOp_SVI.printMat("TempN_2",TempN_2,n,n);
  matrixOp_SVI.printMat("Q",Q,n,n);
  matrixOp_SVI.printMat("P",P,n,n);
*/
  
}

void KalmanFilterJ_SVI::predict( float  U[] ){

  matrixOp_SVI.multiply21D(A,X0,Temp_1,n,n);

  matrixOp_SVI.multiply1D(B,U,Temp_2,n);

  matrixOp_SVI.add1D(Temp_1,Temp_2,X,n);

  matrixOp_SVI.transpose(A,AT,n,n);

  matrixOp_SVI.multiply(P0,AT,TempN_1,n,n,n);

  matrixOp_SVI.multiply(A,TempN_1,TempN_2,n,n,n);

  matrixOp_SVI.add(TempN_2,Q,P,n,n);
  

}

void KalmanFilterJ_SVI::correct ( float Z[] ) {

  matrixOp_SVI.transpose(H,HT,n,n);
  matrixOp_SVI.multiply(P,HT,TempN_1,n,n,n);
  matrixOp_SVI.multiply(H,TempN_1,TempN_2,n,n,n);
  matrixOp_SVI.add(TempN_2,R,TempN_3,n,n);
  matrixOp_SVI.inverse(TempN_3,TempN_4,n);
  matrixOp_SVI.multiply(TempN_1,TempN_4,K,n,n,n);


  matrixOp_SVI.multiply21D(H,X,Temp_1,n,n);
  matrixOp_SVI.substract1D(Z,Temp_1,Temp_2,n);
  matrixOp_SVI.multiply21D(K,Temp_2,Temp_3,n,n);
  matrixOp_SVI.add1D(X,Temp_3,X,n);

;
  matrixOp_SVI.multiply(K,H,TempN_1,n,n,n);
  matrixOp_SVI.substract(I,TempN_1,TempN_2,n,n);
  matrixOp_SVI.multiply(TempN_2,P,TempN_3,n,n,n);
  matrixOp_SVI.copy(P,TempN_3,n,n);

  matrixOp_SVI.copy(X0,X,n);
  matrixOp_SVI.copy(P0,P,n,n);
}
