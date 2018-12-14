/* Solve for real or complex roots of the quartic equation
 * x^4 + a x^3 + b x^2 + c x + d = 0,
 * returning the number of such roots.
 *
 * Roots are returned ordered.
 * Author: Andrew Steiner
 */

#ifndef __VEINS_SupportEllipse_H_
#define __VEINS_SupportEllipse_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define EPS                            +1.0E-05
#define ERROR_ELLIPSE_PARAMETERS           -100
#define ERROR_INTERSECTION_PTS             -109
#define ELLIPSE2_INSIDE_ELLIPSE1            111
#define DISJOINT_ELLIPSES                   103
#define ELLIPSE1_INSIDE_ELLIPSE2            110
#define ELLIPSES_ARE_IDENTICAL              112
#define TANGENT_POINT                       114
#define INTERSECTION_POINT                  113
#define TWO_INTERSECTION_POINTS             107
#define THREE_INTERSECTION_POINTS           108
#define FOUR_INTERSECTION_POINTS            109

#define pi     (2.0*asin (1.0)) //-- a maximum-precision value of pi
#define twopi  (2.0*pi)         //-- a maximum-precision value of 2*pi

//---------------------------------------------------------------------------
//-- functions for solving the quartic equation from Netlib/TOMS
void BIQUADROOTS (double p[], double r[][5]);
void CUBICROOTS (double p[], double r[][5]);
void QUADROOTS (double p[], double r[][5]);

//---------------------------------------------------------------------------
double nointpts (double A1, double B1, double A2, double B2, double H1,
                 double K1, double H2, double K2, double PHI_1, double PHI_2,
                 double H2_TR, double K2_TR, double AA, double BB,
                 double CC, double DD, double EE, double FF, int *rtnCode);

double twointpts (double xint[], double yint[], double A1, double B1,
                  double PHI_1, double A2, double B2, double H2_TR,
                  double K2_TR, double PHI_2, double AA, double BB,
                  double CC, double DD, double EE, double FF, int *rtnCode);

double threeintpts (double xint[], double yint[], double A1, double B1,
                    double PHI_1, double A2, double B2, double H2_TR,
                    double K2_TR, double PHI_2, double AA, double BB,
                    double CC, double DD, double EE, double FF,
                    int *rtnCode);

double fourintpts (double xint[], double yint[], double A1, double B1,
                   double PHI_1, double A2, double B2, double H2_TR,
                   double K2_TR, double PHI_2, double AA, double BB,
                   double CC, double DD, double EE, double FF, int *rtnCode);

int istanpt (double x, double y, double A1, double B1, double AA, double BB,
             double CC, double DD, double EE, double FF);

double ellipse2tr (double x, double y, double AA, double BB,
                   double CC, double DD, double EE, double FF);

//===========================================================================
//== ELLIPSE-ELLIPSE OVERLAP ================================================
//===========================================================================
//choice=1: use gsl_poly_complex_solve()
//choice=2: use Andrew Steiner's gsl_poly_complex_solve_quartic()
double ellipse_ellipse_overlap_netlibs(double PHI_1, double A1, double B1,
                                double H1, double K1, double PHI_2,
                                double A2, double B2, double H2, double K2,
                                double X[4], double Y[4], int * NROOTS,
                                int *rtnCode);

int double_cmp(const void *a, const void *b) ;

#endif
