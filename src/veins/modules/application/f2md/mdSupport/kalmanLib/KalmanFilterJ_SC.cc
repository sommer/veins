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


#include "KalmanFilterJ_SC.h"
using namespace std;

KalmanFilterJ_SC::KalmanFilterJ_SC() {
	n = KLM_N_SC;
  m = KLM_M_SC;
}


void KalmanFilterJ_SC::setFixed( float _A  [][KLM_N_SC], float _H  [][KLM_N_SC], float _Q [][KLM_N_SC], float _R [][KLM_N_SC] ){
  matrixOp_SC.copy(A,_A,n,n);
  matrixOp_SC.copy(H,_H,n,n);
  matrixOp_SC.copy(Q,_Q,n,n);
  matrixOp_SC.copy(R,_R,n,n);

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


void KalmanFilterJ_SC::setFixed( float _A  [][KLM_N_SC], float _H  [][KLM_N_SC], float _Q [][KLM_N_SC], float _R [][KLM_N_SC], float _B [][KLM_M_SC] ){
  matrixOp_SC.copy(A,_A,n,n);
  matrixOp_SC.copy(H,_H,n,n);
  matrixOp_SC.copy(Q,_Q,n,n);
  matrixOp_SC.copy(R,_R,n,n);

  matrixOp_SC.copyM(B,_B,n,m);

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


void KalmanFilterJ_SC::setA(float _A [][KLM_N_SC]){
  matrixOp_SC.copy(A,_A,n,n);
}

void KalmanFilterJ_SC::setB(float _B [][KLM_M_SC]){
  matrixOp_SC.copyM(B,_B,n,m);
}


void KalmanFilterJ_SC::setR(float _R [][KLM_N_SC]){
  matrixOp_SC.copy(R,_R,n,n);
} 


void KalmanFilterJ_SC::setInitial( float _X0[], float _P0 [][KLM_N_SC]){
  matrixOp_SC.copy(X0,_X0,n);
  matrixOp_SC.copy(P0,_P0,n,n);
}



void KalmanFilterJ_SC::predict(void){

  matrixOp_SC.multiply21D(A,X0,X,n,n);

  matrixOp_SC.transpose(A,AT,n,n);

  matrixOp_SC.multiply(P0,AT,TempN_1,n,n,n);

  matrixOp_SC.multiply(A,TempN_1,TempN_2,n,n,n);

  matrixOp_SC.add(TempN_2,Q,P,n,n);
  

/*
  matrixOp_SC.printMat("A",A,n,n);
  matrixOp_SC.printVec("X0",X0,n);
  matrixOp_SC.printMat("P0",P0,n,n);
  matrixOp_SC.printVec("X",X,n);
  matrixOp_SC.printMat("AT",AT,n,n);
  matrixOp_SC.printMat("TempN_1",TempN_1,n,n);
  matrixOp_SC.printMat("TempN_2",TempN_2,n,n);
  matrixOp_SC.printMat("Q",Q,n,n);
  matrixOp_SC.printMat("P",P,n,n);
*/
  
}

void KalmanFilterJ_SC::predict( float  U[] ){

  matrixOp_SC.multiply21D(A,X0,Temp_1,n,n);
  matrixOp_SC.multiply1D(B,U,Temp_2,n);

  matrixOp_SC.add1D(Temp_1,Temp_2,X,n);

  matrixOp_SC.transpose(A,AT,n,n);

  matrixOp_SC.multiply(P0,AT,TempN_1,n,n,n);

  matrixOp_SC.multiply(A,TempN_1,TempN_2,n,n,n);

  matrixOp_SC.add(TempN_2,Q,P,n,n);
  

}

void KalmanFilterJ_SC::correct ( float Z[] ) {

  matrixOp_SC.transpose(H,HT,n,n);
  matrixOp_SC.multiply(P,HT,TempN_1,n,n,n);
  matrixOp_SC.multiply(H,TempN_1,TempN_2,n,n,n);
  matrixOp_SC.add(TempN_2,R,TempN_3,n,n);
  matrixOp_SC.inverse(TempN_3,TempN_4,n);
  matrixOp_SC.multiply(TempN_1,TempN_4,K,n,n,n);

  matrixOp_SC.multiply21D(H,X,Temp_1,n,n);
  matrixOp_SC.substract1D(Z,Temp_1,Temp_2,n);
  matrixOp_SC.multiply21D(K,Temp_2,Temp_3,n,n);
  matrixOp_SC.add1D(X,Temp_3,X,n);

  matrixOp_SC.multiply(K,H,TempN_1,n,n,n);
  matrixOp_SC.substract(I,TempN_1,TempN_2,n,n);
  matrixOp_SC.multiply(TempN_2,P,TempN_3,n,n,n);
  matrixOp_SC.copy(P,TempN_3,n,n);

  matrixOp_SC.copy(X0,X,n);
  matrixOp_SC.copy(P0,P,n,n);
}
