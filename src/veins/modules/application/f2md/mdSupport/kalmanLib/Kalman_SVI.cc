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

#include "Kalman_SVI.h"
using namespace std;

Kalman_SVI::Kalman_SVI() {

  float T = 1;

  A[0][0] = 1;
  A[0][1] = T;
  A[0][2] = 0;
  A[0][3] = 0;

  A[1][0] = 0;
  A[1][1] = 1;
  A[1][2] = 0;
  A[1][3] = 0;
  
  A[2][0] = 0;
  A[2][1] = 0;
  A[2][2] = 1;
  A[2][3] = T;

  A[3][0] = 0;
  A[3][1] = 0;
  A[3][2] = 0;
  A[3][3] = 1;

  B[0][0] = T*T/2.0;
  B[1][0] = T;
  B[2][0] = T*T/2.0;
  B[3][0] = T;

  float H[KLM_N_SVI][KLM_N_SVI];

  H[0][0] = 1;
  H[0][1] = 0;
  H[0][2] = 0;
  H[0][3] = 0;

  H[1][0] = 0;
  H[1][1] = 1;
  H[1][2] = 0;
  H[1][3] = 0;
  
  H[2][0] = 0;
  H[2][1] = 0;
  H[2][2] = 1;
  H[2][3] = 0;

  H[3][0] = 0;
  H[3][1] = 0;
  H[3][2] = 0;
  H[3][3] = 1;


  float Q[KLM_N_SVI][KLM_N_SVI];
  Q[0][0] = 50;
  Q[0][1] = 0;
  Q[0][2] = 0;
  Q[0][3] = 0;

  Q[1][0] = 0;
  Q[1][1] = 10;
  Q[1][2] = 0;
  Q[1][3] = 0;
  
  Q[2][0] = 0;
  Q[2][1] = 0;
  Q[2][2] = 50;
  Q[2][3] = 0;

  Q[3][0] = 0;
  Q[3][1] = 0;
  Q[3][2] = 0;
  Q[3][3] = 10;

  R[0][0] = 5;
  R[0][1] = 0;
  R[0][2] = 0;
  R[0][3] = 0;

  R[1][0] = 0;
  R[1][1] = 1;
  R[1][2] = 0;
  R[1][3] = 0;
  
  R[2][0] = 0;
  R[2][1] = 0;
  R[2][2] = 5;
  R[2][3] = 0;

  R[3][0] = 0;
  R[3][1] = 0;
  R[3][2] = 0;
  R[3][3] = 1;


  P0[0][0] = 0;
  P0[0][1] = 0;
  P0[0][2] = 0;
  P0[0][3] = 0;

  P0[1][0] = 0;
  P0[1][1] = 0;
  P0[1][2] = 0;
  P0[1][3] = 0;

  P0[2][0] = 0;
  P0[2][1] = 0;
  P0[2][2] = 0;
  P0[2][3] = 0;

  P0[3][0] = 0;
  P0[3][1] = 0;
  P0[3][2] = 0;
  P0[3][3] = 0;

  X0[0] = 0;
  X0[1] = 0;
  X0[2] = 0;
  X0[3] = 0;

  U[0] = 0;
  U[1] = 0;
  U[2] = 0;
  U[3] = 0;

  kalmanFilterJ_SVI.setFixed(A, H, Q, R, B);

  init = false;

}

bool Kalman_SVI::isInit(){
  return init;
}


void Kalman_SVI::setInitial(float _X, float _Y, float _VX, float _VY){

  Kalman_SVI();

  P0[0][0] = 1;
  P0[0][1] = 0;
  P0[0][2] = 0;
  P0[0][3] = 0;

  P0[1][0] = 0;
  P0[1][1] = 1;
  P0[1][2] = 0;
  P0[1][3] = 0;
  
  P0[2][0] = 0;
  P0[2][1] = 0;
  P0[2][2] = 1;
  P0[2][3] = 0;

  P0[3][0] = 0;
  P0[3][1] = 0;
  P0[3][2] = 0;
  P0[3][3] = 1;

  X0[0] = _X;
  X0[1] = _VX;
  X0[2] = _Y;
  X0[3] = _VY;

  kalmanFilterJ_SVI.setInitial(X0, P0);

  init = true;
}

void Kalman_SVI::setT(float T){

  A[0][0] = 1;
  A[0][1] = T;
  A[0][2] = 0;
  A[0][3] = 0;

  A[1][0] = 0;
  A[1][1] = 1;
  A[1][2] = 0;
  A[1][3] = 0;
  
  A[2][0] = 0;
  A[2][1] = 0;
  A[2][2] = 1;
  A[2][3] = T;

  A[3][0] = 0;
  A[3][1] = 0;
  A[3][2] = 0;
  A[3][3] = 1;

  B[0][0] = T*T/2.0;
  B[1][0] = T;
  B[2][0] = T*T/2.0;
  B[3][0] = T;

  kalmanFilterJ_SVI.setA(A);
  kalmanFilterJ_SVI.setB(B);
}

void Kalman_SVI::setConfidence(float CX, float CY, float CVX, float CVY){

  R[0][0] = CX;
  R[0][1] = 0;
  R[0][2] = 0;
  R[0][3] = 0;

  R[1][0] = 0;
  R[1][1] = CVX;
  R[1][2] = 0;
  R[1][3] = 0;
  
  R[2][0] = 0;
  R[2][1] = 0;
  R[2][2] = CY;
  R[2][3] = 0;

  R[3][0] = 0;
  R[3][1] = 0;
  R[3][2] = 0;
  R[3][3] = CVY;

  kalmanFilterJ_SVI.setR(R);
}

void Kalman_SVI::getDeltaPos(float T, float _X, float _Y, float _VX, float _VY, float CX, float CY, float CVX, float CVY, float * Delta){

    setT(T);
    setConfidence(CX,CY,CVX,CVY);

    X0[0] = _X;
    X0[1] = _VX;
    X0[2] = _Y;
    X0[3] = _VY;

    kalmanFilterJ_SVI.predict();
    kalmanFilterJ_SVI.correct(X0);

    Delta[0] = fabs(kalmanFilterJ_SVI.X[0] - X0[0]);
    Delta[1] = fabs(kalmanFilterJ_SVI.X[1] - X0[1]);
    Delta[2] = fabs(kalmanFilterJ_SVI.X[2] - X0[2]);
    Delta[3] = fabs(kalmanFilterJ_SVI.X[3] - X0[3]);
}


void Kalman_SVI::getDeltaPos(float T, float _X, float _Y, float _VX, float _VY, float _AX, float _AY, float CX, float CY, float CVX, float CVY, float * Delta){

    setT(T);
    setConfidence(CX,CY,CVX,CVY);

    X0[0] = _X;
    X0[1] = _VX;
    X0[2] = _Y;
    X0[3] = _VY;

    U[0] = _AX;
    U[1] = _AX;
    U[2] = _AY;
    U[3] = _AY;

    kalmanFilterJ_SVI.predict(U);
    kalmanFilterJ_SVI.correct(X0);

    Delta[0] = fabs(kalmanFilterJ_SVI.X[0] - X0[0]);
    Delta[1] = fabs(kalmanFilterJ_SVI.X[1] - X0[1]);
    Delta[2] = fabs(kalmanFilterJ_SVI.X[2] - X0[2]);
    Delta[3] = fabs(kalmanFilterJ_SVI.X[3] - X0[3]);
}


