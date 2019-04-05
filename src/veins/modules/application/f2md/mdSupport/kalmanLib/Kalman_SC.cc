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

#include "Kalman_SC.h"
using namespace std;

Kalman_SC::Kalman_SC() {

  float T = 1;

  A[0][0] = 1;
  A[0][1] = T;

  A[1][0] = 0;
  A[1][1] = 1;

  B[0][0] = 0.5*T*T;
  B[1][0] = T;

  float H[KLM_N_SC][KLM_N_SC];

  H[0][0] = 1;
  H[0][1] = 0;

  H[1][0] = 0;
  H[1][1] = 1;

  float Q[KLM_N_SC][KLM_N_SC];
  Q[0][0] = 10;
  Q[0][1] = 0;

  Q[1][0] = 0;
  Q[1][1] = 10;

  R[0][0] = 5;
  R[0][1] = 0;

  R[1][0] = 0;
  R[1][1] = 1;

  pos = 0;

  kalmanFilterJ_SC.setFixed(A, H, Q, R, B);

  init = false;

}

bool Kalman_SC::isInit(){
  return init;
}


void Kalman_SC::setInitial(float _X, float _Y){

  P0[0][0] = 10;
  P0[0][1] = 0;

  P0[1][0] = 0;
  P0[1][1] = 10;

  X0[0] = _X;
  X0[1] = _Y;

  pos = _X;

  kalmanFilterJ_SC.setInitial(X0, P0);

  init = true;
}

void Kalman_SC::setT(float T){

  A[0][0] = 1;
  A[0][1] = T;

  A[1][0] = 0;
  A[1][1] = 1;


  B[0][0] = 0.5*T*T;
  B[1][0] = T;

  kalmanFilterJ_SC.setA(A);
  kalmanFilterJ_SC.setB(B);
}

void Kalman_SC::setConfidence(float CX, float CY){

  R[0][0] = CX;
  R[0][1] = 0;

  R[1][0] = 0;
  R[1][1] = CY;

  kalmanFilterJ_SC.setR(R);
}

void Kalman_SC::getDeltaPos(float T, float _X, float _Y, float CX, float CY, float * Delta){

    setT(T);
    setConfidence(CX,CY);

    X0[0] = _X;
    X0[1] = _Y;

    kalmanFilterJ_SC.predict();
    kalmanFilterJ_SC.correct(X0);

    Delta[0] = fabs(kalmanFilterJ_SC.X[0] - X0[0]);
    Delta[1] = fabs(kalmanFilterJ_SC.X[1] - X0[1]);

}


void Kalman_SC::getDeltaPos(float T, float _X, float _Y, float _AX, float _AY, float CX, float CY, float * Delta){

    setT(T);
    setConfidence(CX,CY);

    pos = pos + _X;

    X0[0] = pos;
    X0[1] = _Y;

    U[0] = _AX;
    U[1] = _AY;

    kalmanFilterJ_SC.predict(U);
    kalmanFilterJ_SC.correct(X0);

    Delta[0] = fabs(kalmanFilterJ_SC.X[0] - X0[0]);
    Delta[1] = fabs(kalmanFilterJ_SC.X[1] - X0[1]);

}


