
/* poly/solve_quartic.c
 *
 * Copyright (C) 2003 CERN and K.S. K\"{o}lbig
 *
 * Converted to C and implemented into the GSL Library
 * by Andrew W. Steiner and Andy Buckley
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* solve_quartic.c - finds the real roots of
 *  x^4 + a x^3 + b x^2 + c x + d = 0
 */

#include <veins/modules/application/f2md/mdSupport/ellipseIntLib/SupportEllipse.h>

void CUBICROOTS(double p[], double r[][5])
{
     /*
      Array r[3][5]  p[5]
      Roots of poly p[0]*x^3 + p[1]*x^2 + p[2]*x + p[3] = 0
      x=r[1][k] + i r[2][k]  k=1,...,3
      Assumes 0<arctan(x)<pi/2 for x>0
    */
     double s,t,b,c,d;
     int k;
     if(p[0]!=1.0)
     {
          for(k=1;k<4;k++)
               p[k]=p[k]/p[0];
          p[0]=1.0;
     }
     s=p[1]/3.0;
     t=s*p[1];
     b=0.5*(s*(t/1.5-p[2])+p[3]);
     t=(t-p[2])/3.0;
     c=t*t*t;
     d=b*b-c;
     if(d>=0.0)
     {
          d=pow((sqrt(d)+fabs(b)),1.0/3.0);
          if(d!=0.0)
          {
               if(b>0.0)
                    b=-d;
               else
                    b=d;
               c=t/b;
          }
          d=r[2][2]=sqrt(0.75)*(b-c);
          b=b+c;
          c=r[1][2]=-0.5*b-s;
          if((b>0.0 && s<=0.0) || (b<0.0 && s>0.0))
          {
               r[1][1]=c;
               r[2][1]=-d;
               r[1][3]=b-s;
               r[2][3]=0.0;
          }
          else
          {
               r[1][1]=b-s;
               r[2][1]=0.0;
               r[1][3]=c;
               r[2][3]=-d;
          }
     }  /* end 2 equal or complex roots */
     else
     {
          if(b==0.0)
               d=atan(1.0)/1.5;
          else
               d=atan(sqrt(-d)/fabs(b))/3.0;
          if(b<0.0)
               b=2.0*sqrt(t);
          else
               b=-2.0*sqrt(t);
          c=cos(d)*b;
          t=-sqrt(0.75)*sin(d)*b-0.5*c;
          d=-t-c-s;
          c=c-s;
          t=t-s;

          if(fabs(c)>fabs(t))
          {
               r[1][3]=c;
          }
          else
          {
               r[1][3]=t;
               t=c;
          }
          if(fabs(d)>fabs(t))
          {
               r[1][2]=d;
          }
          else
          {
               r[1][2]=t;
               t=d;
          }
          r[1][1]=t;
          for(k=1;k<4;k++)
               r[2][k]=0.0;
     }
     return;
}

void BIQUADROOTS(double p[],double r[][5])
{
     /*
      Array r[3][5]  p[5]
      Roots of poly p[0]*x^4 + p[1]*x^3 + p[2]*x^2 + p[3]*x + p[4] = 0
      x=r[1][k] + i r[2][k]  k=1,...,4
    */
     double a,b,c,d,e;
     int k,j;
     if(p[0] != 1.0)
     {
          for(k=1;k<5;k++)
               p[k]=p[k]/p[0];
          p[0]=1.0;
     }
     e=0.25*p[1];
     b=2.0*e;
     c=b*b;
     d=0.75*c;
     b=p[3]+b*(c-p[2]);
     a=p[2]-d;
     c=p[4]+e*(e*a-p[3]);
     a=a-d;
     p[1]=0.5*a;
     p[2]=(p[1]*p[1]-c)*0.25;
     p[3]=b*b/(-64.0);
     if(p[3]<0.0)
     {
          CUBICROOTS(p,r);
          for(k=1;k<4;k++)
          {
               if(r[2][k]==0.0 && r[1][k]>0.0)
               {
                    d=r[1][k]*4.0;
                    a=a+d;
                    if(a>=0.0 && b>=0.0)
                         p[1]=sqrt(d);
                    else if(a<=0.0 && b<=0.0)
                         p[1]=sqrt(d);
                    else
                         p[1]=-sqrt(d);
                    b=0.5*(a+b/p[1]);
                    goto QUAD;
               }
          }
     }
     if(p[2]<0.0)
     {
          b=sqrt(c);
          d=b+b-a;
          p[1]=0.0;
          if(d>0.0)
               p[1]=sqrt(d);
     }
     else
     {
          if(p[1]>0.0)
               b=sqrt(p[2])*2.0+p[1];
          else
               b=-sqrt(p[2])*2.0+p[1];
          if(b!=0.0)
          {
               p[1]=0.0;
          }
          else
          {
               for(k=1;k<5;k++)
               {
                    r[1][k]=-e;
                    r[2][k]=0.0;
               }
               goto END;
          }
     }
QUAD:

     p[2]=c/b;
     QUADROOTS(p,r);
     for(k=1;k<3;k++)
          for(j=1;j<3;j++)
               r[j][k+2]=r[j][k];
     p[1]=-p[1];
     p[2]=b;
     QUADROOTS(p,r);
     for(k=1;k<5;k++)
     {
          r[1][k]=r[1][k]-e;
     }
END:
     for(k=1;k<5;k++)
          return;
}
//===========================================================================
//-- CACM Algorithm 326: Roots of low order polynomials.
//-- Nonweiler, Terence R.F., CACM Algorithm 326: Roots of low order
//-- polynomials, Communications of the ACM, vol. 11 no. 4, pages
//-- 269-270 (1968). Translated into c and programmed by M. Dow, ANUSF,
//-- Australian National University, Canberra, Australia.
//-- Accessed at http://www.netlib.org/toms/326.
//-- Modified to void functions, integers replaced with floating point
//-- where appropriate, some other slight modifications for readability
//-- and debugging ease.
//===========================================================================
void QUADROOTS (double p[], double r[][5])
{
     /*
      Array r[3][5]  p[5]
      Roots of poly p[0]*x^2 + p[1]*x + p[2]=0
      x=r[1][k] + i r[2][k]  k=1,2
    */
     double b,c,d;
     b=-p[1]/(2.0*p[0]);
     c=p[2]/p[0];
     d=b*b-c;
     if(d>=0.0)
     {
          if(b>0.0)
               b=(r[1][2]=(sqrt(d)+b));
          else
               b=(r[1][2]=(-sqrt(d)+b));
          r[1][1]=c/b;
          r[2][1]=(r[2][2]=0.0);
     }
     else
     {
          d=(r[2][1]=sqrt(-d));
          r[2][2]=-d;
          r[1][1]=(r[1][2]=b);
     }
     return;
}

//-- check whether an intersection point is a tangent or a cross-point
int istanpt (double x, double y, double A1, double B1, double AA, double BB,
             double CC, double DD, double EE, double FF)
{
     double x1, y1, x2, y2, theta, test1, test2, eps_radian;

     //-- Avoid inverse trig calculation errors: there could be an error
     //-- if |x1/A| > 1.0 when calling acos().  If execution arrives here,
     //-- then the point is on the ellipse within EPS.
     if (fabs (x) > A1)
          x = (x < 0) ? -A1 : A1;

     //-- Calculate the parametric angle on the ellipse for (x, y)
     //-- The parametric angles depend on the quadrant where each point
     //-- is located.  See Table 1 in the reference.
     if (y < 0.0)    //-- Quadrant III or IV
          theta = twopi - acos (x / A1);
     else             //-- Quadrant I or II
          theta = acos (x / A1);

     //-- determine the distance from the origin to the point (x, y)
     /* branch = sqrt (x*x + y*y); */

     /* //-- use the distance to find a small angle, such that the distance */
     /* //-- along ellipse 1 is approximately 2*EPS */
     /* if (branch < 100.0*EPS) */
     /*     eps_radian = 2.0*EPS; */
     /* else */
     /*     eps_radian = asin (2.0*EPS/branch); */

     //fix 24.11.12
     eps_radian = 0.1; //arbitrary value

     //-- determine two points that are on each side of (x, y) and lie on
     //-- the first ellipse
     x1 = A1*cos (theta + eps_radian);
     y1 = B1*sin (theta + eps_radian);
     x2 = A1*cos (theta - eps_radian);
     y2 = B1*sin (theta - eps_radian);

     //-- evaluate the two adjacent points in the second ellipse equation
     test1 = ellipse2tr (x1, y1, AA, BB, CC, DD, EE, FF);
     test2 = ellipse2tr (x2, y2, AA, BB, CC, DD, EE, FF);

     //-- if the ellipses are tangent at the intersection point, then
     //-- points on both sides will either both be inside ellipse 1, or
     //-- they will both be outside ellipse 1
#if DEBUG
     printf("\t\t--- debug istanpt with (x,y)=(%f, %f), A1=%f, B1=%f\n", x, y, A1, B1);
     printf("theta=%f\n", theta);
     printf("eps_Radian=%f\n", eps_radian);
     printf("(x1, y1)=(%f, %f)\n", x1, y1);
     printf("(x2, y2)=(%f, %f)\n", x2, y2);
     printf("test1=%f\n", test1);
     printf("test2=%f\n", test2);
#endif

     if ((test1*test2) > 0.0)
          return TANGENT_POINT;
     else
          return INTERSECTION_POINT;
}
//-- two distinct intersection points (x1, y1) and (x2, y2) find overlap area
double twointpts (double x[], double y[], double A1, double B1, double PHI_1,
                  double A2, double B2, double H2_TR, double K2_TR,
                  double PHI_2, double AA, double BB, double CC, double DD,
                  double EE, double FF, int *rtnCode)
{
     double area1, area2;
     double xmid, ymid, xmid_rt, ymid_rt;
     double theta1, theta2;
     double tmp, trsign;
     double x1_tr, y1_tr, x2_tr, y2_tr;
     //double discr;
     double cosphi, sinphi;

     //-- if execution arrives here, the intersection points are not
     //-- tangents.

     //-- determine which direction to integrate in the ellipse_segment
     //-- routine for each ellipse.

     //-- find the parametric angles for each point on ellipse 1
     if (fabs (x[0]) > A1)
          x[0] = (x[0] < 0) ? -A1 : A1;
     if (y[0] < 0.0)     //-- Quadrant III or IV
          theta1 = twopi - acos (x[0] / A1);
     else             //-- Quadrant I or II
          theta1 = acos (x[0] / A1);

     if (fabs (x[1]) > A1)
          x[1] = (x[1] < 0) ? -A1 : A1;
     if (y[1] < 0.0)     //-- Quadrant III or IV
          theta2 = twopi - acos (x[1] / A1);
     else             //-- Quadrant I or II
          theta2 = acos (x[1] / A1);

     //-- logic is for proceeding counterclockwise from theta1 to theta2
     if (theta1 > theta2)
     {
          tmp = theta1;
          theta1 = theta2;
          theta2 = tmp;
     }

     //-- find a point on the first ellipse that is different than the two
     //-- intersection points.
     xmid = A1*cos ((theta1 + theta2)/2.0);
     ymid = B1*sin ((theta1 + theta2)/2.0);

     //-- the point (xmid, ymid) is on the first ellipse 'between' the two
     //-- intersection points (x[1], y[1]) and (x[2], y[2]) when travelling
     //-- counter- clockwise from (x[1], y[1]) to (x[2], y[2]).  If the point
     //-- (xmid, ymid) is inside the second ellipse, then the desired segment
     //-- of ellipse 1 contains the point (xmid, ymid), so integrate
     //-- counterclockwise from (x[1], y[1]) to (x[2], y[2]).  Otherwise,
     //-- integrate counterclockwise from (x[2], y[2]) to (x[1], y[1])
     if (ellipse2tr (xmid, ymid, AA, BB, CC, DD, EE, FF) > 0.0)
     {
          tmp = theta1;
          theta1 = theta2;
          theta2 = tmp;
     }

     //-- here is the ellipse segment routine for the first ellipse
     if (theta1 > theta2)
          theta1 -= twopi;
     if ((theta2 - theta1) > pi)
          trsign = 1.0;
     else
          trsign = -1.0;


     area1 = 0.5*(A1*B1*(theta2 - theta1)
                  + trsign*fabs (x[0]*y[1] - x[1]*y[0]));

     if (area1 < 0)
     {
          printf("TWO area1=%f\n",area1);
          area1 += A1*B1;
          getc(stdin);
     }
     //-- find ellipse 2 segment area.  The ellipse segment routine
     //-- needs an ellipse that is centered at the origin and oriented
     //-- with the coordinate axes.  The intersection points (x[1], y[1]) and
     //-- (x[2], y[2]) are found with both ellipses translated and rotated by
     //-- (-H1, -K1) and -PHI_1.  Further translate and rotate the points
     //-- to put the second ellipse at the origin and oriented with the
     //-- coordinate axes.  The translation is (-H2_TR, -K2_TR), and the
     //-- rotation is -(PHI_2 - PHI_1) = PHI_1 - PHI_2
     cosphi = cos (PHI_1 - PHI_2);
     sinphi = sin (PHI_1 - PHI_2);
     x1_tr = (x[0] - H2_TR)*cosphi + (y[0] - K2_TR)*-sinphi;
     y1_tr = (x[0] - H2_TR)*sinphi + (y[0] - K2_TR)*cosphi;
     x2_tr = (x[1] - H2_TR)*cosphi + (y[1] - K2_TR)*-sinphi;
     y2_tr = (x[1] - H2_TR)*sinphi + (y[1] - K2_TR)*cosphi;

     //-- determine which branch of the ellipse to integrate by finding a
     //-- point on the second ellipse, and asking whether it is inside the
     //-- first ellipse (in their once-translated+rotated positions)
     //-- find the parametric angles for each point on ellipse 1
     if (fabs (x1_tr) > A2)
          x1_tr = (x1_tr < 0) ? -A2 : A2;
     if (y1_tr < 0.0)    //-- Quadrant III or IV
          theta1 = twopi - acos (x1_tr/A2);
     else             //-- Quadrant I or II
          theta1 = acos (x1_tr/A2);

     if (fabs (x2_tr) > A2)
          x2_tr = (x2_tr < 0) ? -A2 : A2;
     if (y2_tr < 0.0)    //-- Quadrant III or IV
          theta2 = twopi - acos (x2_tr/A2);
     else             //-- Quadrant I or II
          theta2 = acos (x2_tr/A2);

     //-- logic is for proceeding counterclockwise from theta1 to theta2
     if (theta1 > theta2)
     {
          tmp = theta1;
          theta1 = theta2;
          theta2 = tmp;
     }

     //-- find a point on the second ellipse that is different than the two
     //-- intersection points.
     xmid = A2*cos ((theta1 + theta2)/2.0);
     ymid = B2*sin ((theta1 + theta2)/2.0);

     //-- translate the point back to the second ellipse in its once-
     //-- translated+rotated position
     cosphi = cos (PHI_2 - PHI_1);
     sinphi = sin (PHI_2 - PHI_1);
     xmid_rt = xmid*cosphi + ymid*-sinphi + H2_TR;
     ymid_rt = xmid*sinphi + ymid*cosphi + K2_TR;

     //-- the point (xmid_rt, ymid_rt) is on the second ellipse 'between' the
     //-- intersection points (x[1], y[1]) and (x[2], y[2]) when travelling
     //-- counterclockwise from (x[1], y[1]) to (x[2], y[2]).  If the point
     //-- (xmid_rt, ymid_rt) is inside the first ellipse, then the desired
     //-- segment of ellipse 2 contains the point (xmid_rt, ymid_rt), so
     //-- integrate counterclockwise from (x[1], y[1]) to (x[2], y[2]).
     //-- Otherwise, integrate counterclockwise from (x[2], y[2]) to
     //-- (x[1], y[1])
     if (((xmid_rt*xmid_rt)/(A1*A1) + (ymid_rt*ymid_rt)/(B1*B1)) > 1.0)
     {
          tmp = theta1;
          theta1 = theta2;
          theta2 = tmp;
     }

     //-- here is the ellipse segment routine for the second ellipse
     if (theta1 > theta2)
          theta1 -= twopi;
     if ((theta2 - theta1) > pi)
          trsign = 1.0;
     else
          trsign = -1.0;
     area2 = 0.5*(A2*B2*(theta2 - theta1)
                  + trsign*fabs (x1_tr*y2_tr - x2_tr*y1_tr));

     if (area2 < 0)
     {
#if DEBUG
          printf("TWO area2=%f\n",area2);
#endif
          area2 += A2*B2;

     }

     (*rtnCode) = TWO_INTERSECTION_POINTS;
#if DEBUG
     printf("Twointpts: \t area1 =%f,  area2=%f\n",area1, area2);
#endif
     return area1 + area2;
}

//-- three distinct intersection points, must have two intersections
//-- and one tangent, which is the only possibility
double threeintpts (double xint[], double yint[], double A1, double B1,
                    double PHI_1, double A2, double B2, double H2_TR,
                    double K2_TR, double PHI_2, double AA, double BB,
                    double CC, double DD, double EE, double FF,
                    int *rtnCode)
{
     int i, tanpts, tanindex, fnRtn;
     double OverlapArea;

     //-- need to determine which point is a tangent, and which two points
     //-- are intersections
     tanpts = 0;
     for (i = 0; i < 3; i++)
     {
          fnRtn = istanpt (xint[i], yint[i], A1, B1, AA, BB, CC, DD, EE, FF);

          if (fnRtn == TANGENT_POINT)
          {
               tanpts++;
               tanindex = i;
          }
     }
#if DEBUG
     printf("tanindex=%d\n",tanindex);
#endif
     //-- there MUST be 2 intersection points and only one tangent
     if (tanpts != 1)
     {
          //-- should never get here unless there is a problem discerning
          //-- whether or not a point is a tangent or intersection
          (*rtnCode) = ERROR_INTERSECTION_PTS;
          return -1.0;
     }

     //-- store the two interesection points into (x[1], y[1]) and
     //-- (x[2], y[2])
     switch (tanindex)
     {
     case 0:
          xint[0] = xint[2];
          yint[0] = yint[2];

          break;

     case 1:
          xint[1] = xint[2];
          yint[1] = yint[2];

          break;

     case 2:
          //-- intersection points are already in the right places
          break;
     }

     OverlapArea = twointpts (xint, yint, A1, B1, PHI_1, A2, B2, H2_TR, K2_TR,
                              PHI_2, AA, BB, CC, DD, EE, FF, rtnCode);
     (*rtnCode) = THREE_INTERSECTION_POINTS;
     return OverlapArea;
}

//-- four intersection points
double fourintpts (double xint[], double yint[], double A1, double B1,
                   double PHI_1, double A2, double B2, double H2_TR,
                   double K2_TR, double PHI_2, double AA, double BB,
                   double CC, double DD, double EE, double FF, int *rtnCode)
{
     int i, j, k;
     double xmid, ymid, xint_tr[4], yint_tr[4], OverlapArea;
     double theta[4], theta_tr[4], cosphi, sinphi, tmp0, tmp1, tmp2;
     double area1=0, area2=0, area3=0, area4=0, area5=0;


     //some tmp-variables to avoid calculating the same thing several times.
     double A1B1 = A1*B1;
     double A2B2 = A2*B2;
     double Area_1 = pi*A1B1;
     double Area_2 = pi*A2B2;


     //-- only one case, which involves two segments from each ellipse, plus
     //-- two triangles.
     //-- get the parametric angles along the first ellipse for each of the
     //-- intersection points
//      for (i = 1; i <= 4; i++)
     for (i = 0; i <= 3; i++)
     {
          if (fabs (xint[i]) > A1)
               xint[i] = (xint[i] < 0) ? -A1 : A1;
          if (yint[i] < 0.0)     //-- Quadrant III or IV
               theta[i] = twopi - acos (xint[i] / A1);
          else             //-- Quadrant I or II
               theta[i] = acos (xint[i] / A1);
     }

     //-- sort the angles by straight insertion, and put the points in
     //-- counter-clockwise order
#if DEBUG
     for (k=0; k<=3; k++)
     {
          printf("k=%d:  Theta = %f, xint=%f, yint=%f\n",k,theta[k], xint[k], yint[k]);
     }
#endif
     for (j = 1; j <= 3; j++)
     {
          tmp0 = theta[j];
          tmp1 = xint[j];
          tmp2 = yint[j];

          for (k = j - 1; k >= 0; k--)
          {
               if (theta[k] <= tmp0)
                    break;

               theta[k+1] = theta[k];
               xint[k+1] = xint[k];
               yint[k+1] = yint[k];
          }

          theta[k+1] = tmp0;
          xint[k+1] = tmp1;
          yint[k+1] = tmp2;
     }
#if DEBUG
     printf("AFTER sorting\n");
     for (k=0; k<=3; k++)
     {
          printf("k=%d:  Theta = %f, xint=%f, yint=%f\n",k,theta[k], xint[k], yint[k]);
     }
#endif

     //-- find the area of the interior quadrilateral
     /* area1 = 0.5*fabs ((xint[3] - xint[1])*(yint[4] - yint[2]) */
     /*                   - (xint[4] - xint[2])*(yint[3] - yint[1])); */
     area1 = 0.5*fabs ((xint[2] - xint[0])*(yint[3] - yint[1])
                       - (xint[3] - xint[1])*(yint[2] - yint[0]));

     //-- the intersection points lie on the second ellipse in its once
     //-- translated+rotated position.  The segment algorithm is implemented
     //-- for an ellipse that is centered at the origin, and oriented with
     //-- the coordinate axes; so, in order to use the segment algorithm
     //-- with the second ellipse, the intersection points must be further
     //-- translated+rotated by amounts that put the second ellipse centered
     //-- at the origin and oriented with the coordinate axes.
     cosphi = cos (PHI_1 - PHI_2);
     sinphi = sin (PHI_1 - PHI_2);
     for (i = 0; i <= 3; i++)
     {
          xint_tr[i] = (xint[i] - H2_TR)*cosphi + (yint[i] - K2_TR)*-sinphi;
          yint_tr[i] = (xint[i] - H2_TR)*sinphi + (yint[i] - K2_TR)*cosphi;

          if (fabs (xint_tr[i]) > A2)
               xint_tr[i] = (xint_tr[i] < 0) ? -A2 : A2;
          if (yint_tr[i] < 0.0)      //-- Quadrant III or IV
               theta_tr[i] = twopi - acos (xint_tr[i]/A2);
          else             //-- Quadrant I or II
               theta_tr[i] = acos (xint_tr[i]/A2);
     }

     //-- get the area of the two segments on ellipse 1
     xmid = A1*cos ((theta[0] + theta[1])/2.0);
     ymid = B1*sin ((theta[0] + theta[1])/2.0);
     //-- the point (xmid, ymid) is on the first ellipse 'between' the two
     //-- sorted intersection points (xint[1], yint[1]) and (xint[2], yint[2])
     //-- when travelling counter- clockwise from (xint[1], yint[1]) to
     //-- (xint[2], yint[2]).  If the point (xmid, ymid) is inside the second
     //-- ellipse, then one desired segment of ellipse 1 contains the point
     //-- (xmid, ymid), so integrate counterclockwise from (xint[1], yint[1])
     //-- to (xint[2], yint[2]) for the first segment, and from
     //-- (xint[3], yint[3] to (xint[4], yint[4]) for the second segment.
     if (ellipse2tr (xmid, ymid, AA, BB, CC, DD, EE, FF) < 0.0)
     {
          area2 = 0.5*(A1B1*(theta[1] - theta[0])
                       - fabs (xint[0]*yint[1] - xint[1]*yint[0]));

          area3 = 0.5*(A1B1*(theta[3] - theta[2])
                        - fabs (xint[2]*yint[3] - xint[3]*yint[2]));

          area4 = 0.5*(A2B2*(theta_tr[2] - theta_tr[1]) - fabs (xint_tr[1]*yint_tr[2] - xint_tr[2]*yint_tr[1]) );

          if (theta_tr[3] > theta_tr[0])
               area5 = 0.5*(A2B2*(theta_tr[0] - (theta_tr[3] - twopi))
                            - fabs (xint_tr[3]*yint_tr[0] - xint_tr[0]*yint_tr[3]));
          else
               area5 = 0.5*(A2B2*(theta_tr[0] - theta_tr[3])
                            - fabs (xint_tr[3]*yint_tr[0] - xint_tr[0]*yint_tr[3]));
     }
     else
     {
          area2 = 0.5*(A1B1*(theta[2] - theta[1])
                       - fabs (xint[1]*yint[2] - xint[2]*yint[1]));
          area3 = 0.5*(A1B1*(theta[0] - (theta[3] - twopi))
                       - fabs (xint[3]*yint[0] - xint[0]*yint[3]));
          area4 = 0.5*(A2B2*(theta_tr[1] - theta_tr[0])
                       - fabs (xint_tr[0]*yint_tr[1] - xint_tr[1]*yint_tr[0]));
          area5 = 0.5*(A2B2*(theta_tr[3] - theta_tr[2])
                       - fabs (xint_tr[2]*yint_tr[3] - xint_tr[3]*yint_tr[2]));
     }
     if(area5<0)
     {
#if DEBUG
          printf("\n\t\t-------------> area5 is negativ (%f). Add: pi*A2*B2=%f <------------\n",area5, Area_2);
#endif
          area5 += Area_2;
     }
     if(area4<0)
     {
#if DEBUG
          printf("\n\t\t-------------> area4 is negativ (%f). Add: pi*A2*B2=%f <------------\n",area4, Area_2);
#endif
          area4 += Area_2;
     }
     if(area3<0)
     {
#if DEBUG
          printf("\n\t\t-------------> area3 is negativ (%f). Add: pi*A2*B2=%f <------------\n",area3, Area_1);
#endif
          area3 += Area_1;
     }
     if(area2<0)
     {
#if DEBUG
          printf("\n\t\t-------------> area2 is negativ (%f). Add: pi*A2*B2=%f <------------\n",area2, Area_1);
#endif
          area2 += Area_1;
     }

#if DEBUG
     printf("\narea1=%f, area2=%f area3=%f, area4=%f, area5=%f\n\n",area1, area2, area3, area4, area5);
#endif
     OverlapArea = area1 + area2 + area3 + area4 + area5;

     (*rtnCode) = FOUR_INTERSECTION_POINTS;
     return OverlapArea;
}

double ellipse2tr (double x, double y, double AA, double BB,
                   double CC, double DD, double EE, double FF)
{
     return (AA*x*x + BB*x*y + CC*y*y + DD*x + EE*y + FF);
}

double nointpts (double A1, double B1, double A2, double B2, double H1,
                 double K1, double H2, double K2, double PHI_1, double PHI_2,
                 double H2_TR, double K2_TR, double AA, double BB,
                 double CC, double DD, double EE, double FF, int *rtnCode)
{
     //some tmp-variables to avoid doing things several times.
     double A1B1 = A1*B1;
     double A2B2 = A2*B2;
     double Area_1 = pi*A1B1;
     double Area_2 = pi*A2B2;
     //-- The relative size of the two ellipses can be found from the axis
     //-- lengths
     double relsize = A1B1 - A2B2;
     if (relsize > 0.0)
     {
          //-- First Ellipse is larger than second ellipse.
          //-- If second ellipse center (H2_TR, K2_TR) is inside
          //-- first ellipse, then ellipse 2 is completely inside
          //-- ellipse 1. Otherwise, the ellipses are disjoint.
          if ( ((H2_TR*H2_TR) / (A1*A1)
                + (K2_TR*K2_TR) / (B1*B1)) < 1.0 )
          {
               (*rtnCode) = ELLIPSE2_INSIDE_ELLIPSE1;
               return Area_2;
          }
          else
          {
               (*rtnCode) = DISJOINT_ELLIPSES;
               return 0.0;
          }
     }
     else if (relsize < 0.0)
     {
          //-- Second Ellipse is larger than first ellipse
          //-- If first ellipse center (0, 0) is inside the
          //-- second ellipse, then ellipse 1 is completely inside
          //-- ellipse 2. Otherwise, the ellipses are disjoint
          //--   AA*x^2 + BB*x*y + CC*y^2 + DD*x + EE*y + FF = 0
          if (FF < 0.0)
          {
               (*rtnCode) = ELLIPSE1_INSIDE_ELLIPSE2;
               return Area_1;
          }
          else
          {
               (*rtnCode) = DISJOINT_ELLIPSES;
               return 0.0;
          }
     }
     else
     {
          //-- If execution arrives here, the relative sizes are identical.
          //-- Are the ellipses the same?  Check the parameters to see.
          //MC. Ellipses are the same if: H1=H2 And K1==K2 And Area_1 == Area_2
          //if ((((fabs (H1 - H2)) < EPS) && (fabs (K1 - K2) < EPS))
          //      && (fabs (PHI_1 - PHI_2) < EPS))
          if( (fabs (H1 - H2) < EPS) && (fabs (K1 - K2) < EPS) && (fabs (Area_1 - Area_2) < EPS))
          {
               (*rtnCode) = ELLIPSES_ARE_IDENTICAL;
               return Area_1;
          }
          else
          {
               //-- ellipses must be disjoint
               (*rtnCode) = DISJOINT_ELLIPSES;
               return 0.0;
          }
     }//-- end if (relsize > 0.0)
}


double ellipse_ellipse_overlap_netlibs(double PHI_1, double A1, double B1,
        double H1, double K1, double PHI_2, double A2, double B2, double H2,
        double K2, double X[4], double Y[4], int * NROOTS, int *rtnCode) {
#if DEBUG
    printf("Enter ellipse_ellipse_overlap_netlibs\n");
#endif

    //=======================================================================
    //== DEFINE LOCAL VARIABLES =============================================
    //=======================================================================
    int i, nroots, nychk, nintpts, fnRtnCode;
    double AA, BB, CC, DD, EE, FF, H2_TR, K2_TR, A22, B22, PHI_2R;
    double cosphi, cosphi2, sinphi, sinphi2, cosphisinphi;
    double tmp0, tmp1, tmp2, tmp3;
    double cy[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 }, py[5] = { 0.0, 0.0, 0.0, 0.0,
            0.0 }, r[3][5] = { { 0.0, 0.0, 0.0, 0.0, 0.0 } };
    double x1, x2;
    double ychk[4] = { 0.0, 0.0, 0.0, 0.0 };
    double xint[4], yint[4];
    //ychk will be sorted. this initial value should allow
    double OverlapArea;

    //=======================================================================
    //== DATA CHECK =========================================================
    //=======================================================================
    //-- Each of the ellipse axis lengths must be positive
    if ((!(A1 > EPS) || !(B1 > EPS)) || (!(A2 > EPS) || !(B2 > EPS))) {
        (*rtnCode) = ERROR_ELLIPSE_PARAMETERS;
        return -1.0;
    }
    if ((fabs(PHI_1) > (pi))) {
        PHI_1 = fmod(PHI_1, pi);
    }
    if ((fabs(PHI_2) > (pi))) {
        PHI_2 = fmod(PHI_2, pi);
    }

    //=======================================================================
    //== DETERMINE THE TWO ELLIPSE EQUATIONS FROM INPUT PARAMETERS ==========
    //=======================================================================
    //-- Finding the points of intersection between two general ellipses
    //-- requires solving a quartic equation.  Before attempting to solve the
    //-- quartic, several quick tests can be used to eliminate some cases
    //-- where the ellipses do not intersect.  Optionally, can whittle away
    //-- at the problem, by addressing the easiest cases first.

    //-- Working with the translated+rotated ellipses simplifies the
    //-- calculations.  The ellipses are translated then rotated so that the
    //-- first ellipse is centered at the origin and oriented with the
    //-- coordinate axes.  Then, the first ellipse will have the implicit
    //-- (polynomial) form of
    //--   x^2/A1^2 + y+2/B1^2 = 1

    //-- For the second ellipse, the center is first translated by the amount
    //-- required to put the first ellipse at the origin, e.g., by (-H1, -K1)
    //-- Then, the center of the second ellipse is rotated by the amount
    //-- required to orient the first ellipse with the coordinate axes, e.g.,
    //-- through the angle -PHI_1.
    //-- The translated and rotated center point coordinates for the second
    //-- ellipse are found with the rotation matrix, derivations are
    //-- described in the reference.
    cosphi = cos(PHI_1);
    sinphi = sin(PHI_1);
    H2_TR = (H2 - H1) * cosphi + (K2 - K1) * sinphi;
    K2_TR = (H1 - H2) * sinphi + (K2 - K1) * cosphi;
    PHI_2R = PHI_2 - PHI_1;
    if ((fabs(PHI_2R) > (twopi)))
        PHI_2R = fmod(PHI_2R, twopi);

#if DEBUG
    printf("H2_TR=%f, K2_TR=%f, PHI_2R=%f\n", H2_TR, K2_TR, PHI_2R);
#endif
    //-- Calculate implicit (Polynomial) coefficients for the second ellipse
    //-- in its translated-by (-H1, -H2) and rotated-by -PHI_1 postion
    //--       AA*x^2 + BB*x*y + CC*y^2 + DD*x + EE*y + FF = 0
    //-- Formulas derived in the reference
    //-- To speed things up, store multiply-used expressions first
    cosphi = cos(PHI_2R);
    cosphi2 = cosphi * cosphi;
    sinphi = sin(PHI_2R);
    sinphi2 = sinphi * sinphi;
    cosphisinphi = 2.0 * cosphi * sinphi;
    A22 = A2 * A2;
    B22 = B2 * B2;
    tmp0 = (cosphi * H2_TR + sinphi * K2_TR) / A22;
    tmp1 = (sinphi * H2_TR - cosphi * K2_TR) / B22;
    tmp2 = cosphi * H2_TR + sinphi * K2_TR;
    tmp3 = sinphi * H2_TR - cosphi * K2_TR;

    //-- implicit polynomial coefficients for the second ellipse
    AA = cosphi2 / A22 + sinphi2 / B22;
    BB = cosphisinphi / A22 - cosphisinphi / B22;
    CC = sinphi2 / A22 + cosphi2 / B22;
    DD = -2.0 * cosphi * tmp0 - 2.0 * sinphi * tmp1;
    EE = -2.0 * sinphi * tmp0 + 2.0 * cosphi * tmp1;
    FF = tmp2 * tmp2 / A22 + tmp3 * tmp3 / B22 - 1.0;
    //=======================================================================
    //== CREATE AND SOLVE THE QUARTIC EQUATION TO FIND INTERSECTION POINTS ==
    //=======================================================================
    //-- If execution arrives here, the ellipses are at least 'close' to
    //-- intersecting.
    //-- Coefficients for the Quartic Polynomial in y are calculated from
    //-- the two implicit equations.
    //-- Formulas for these coefficients are derived in the reference.
    /* cy[4] = pow (A1, 4.0)*AA*AA + B1*B1*(A1*A1*(BB*BB - 2.0*AA*CC) */
    /*      + B1*B1*CC*CC); */
    cy[4] = A1 * A1 * A1 * A1 * AA * AA
            + B1 * B1
                    * (A1 * A1 * (BB * BB - 2.0 * AA * CC) + B1 * B1 * CC * CC);
    cy[3] = 2.0 * B1 * (B1 * B1 * CC * EE + A1 * A1 * (BB * DD - AA * EE));
    cy[2] = A1 * A1
            * ((B1 * B1 * (2.0 * AA * CC - BB * BB) + DD * DD - 2.0 * AA * FF)
                    - 2.0 * A1 * A1 * AA * AA)
            + B1 * B1 * (2.0 * CC * FF + EE * EE);
    cy[1] = 2.0 * B1 * (A1 * A1 * (AA * EE - BB * DD) + EE * FF);
    cy[0] = (A1 * (A1 * AA - DD) + FF) * (A1 * (A1 * AA + DD) + FF);

    //-- Once the coefficients for the Quartic Equation in y are known, the
    //-- roots of the quartic polynomial will represent y-values of the
    //-- intersection points of the two ellipse curves.
    //-- The quartic sometimes degenerates into a polynomial of lesser
    //-- degree, so handle all possible cases.
#if DEBUG
    for(i=0; i<5; i++)
    printf("cy[%d]=%f\n",i, cy[i]);
#endif
    if (fabs(cy[4]) > EPS) {
        //== QUARTIC COEFFICIENT NONZERO, USE QUARTIC FORMULA ===============
        for (i = 0; i <= 3; i++)
            py[4 - i] = cy[i] / cy[4];
        py[0] = 1.0;

#if DEBUG
        for(i=0; i<5; i++)
        printf("py[%d]=%f\n",i, py[i]);
#endif
        BIQUADROOTS(py, r);
        nroots = 4;

#if DEBUG
        printf("nroots=%d\n", nroots);
#endif
    } else if (fabs(cy[3]) > EPS) {
        //== QUARTIC DEGENERATES TO CUBIC, USE CUBIC FORMULA ================
        for (i = 0; i <= 2; i++)
            py[3 - i] = cy[i] / cy[3];
        py[0] = 1.0;

        CUBICROOTS(py, r);
        nroots = 3;
        //#####################################################################################
    } else if (fabs(cy[2]) > EPS) {

        //== QUARTIC DEGENERATES TO QUADRATIC, USE QUADRATIC FORMULA ========
        for (i = 0; i <= 1; i++)
            py[2 - i] = cy[i] / cy[2];
        py[0] = 1.0;

        QUADROOTS(py, r);
        nroots = 2;
        //#####################################################################################
    } else if (fabs(cy[1]) > EPS) {
        //== QUARTIC DEGENERATES TO LINEAR: SOLVE DIRECTLY ==================
        //-- cy[1]*Y + cy[0] = 0
        r[1][1] = (-cy[0] / cy[1]);
        r[2][1] = 0.0;
        nroots = 1;
    } else {
        //== COMPLETELY DEGENERATE QUARTIC: ELLIPSES IDENTICAL??? ===========
        //-- a completely degenerate quartic, which would seem to
        //-- indicate that the ellipses are identical.  However, some
        //-- configurations lead to a degenerate quartic with no
        //-- points of intersection.
        nroots = 0;
    }
//=======================================================================
//== CHECK ROOTS OF THE QUARTIC: ARE THEY POINTS OF INTERSECTION? =======
//=======================================================================
//-- determine which roots are real, discard any complex roots
#if DEBUG
    printf ("nroots = %d\n",nroots);
#endif
    nychk = 0;
// GSL returns roots sorted in ascending order. We need descending sorted roots
    for (i = 1; i <= nroots; i++) {
        /* for (i = nroots; i >= 1; i--){ */
        if (fabs(r[2][i]) < EPS) {

            nychk++;
#if DEBUG
            printf ("nychk = %d\n",nychk);
#endif
            ychk[nychk - 1] = r[1][i] * B1;
#if DEBUG
            printf("ROOT is Real,  i=%d --> %f (B1=%f)\n",i,r[1][i],B1);
#endif
        } //if fabs()
    } // for i

//-- sort the real roots by straight insertion
    qsort(ychk, nychk, sizeof(double), double_cmp);
//size_t numbers_len = sizeof(ychk)/sizeof(double);
#if DEBUG
    printf("nychk=%d\n",nychk);
    int j;
    for(j = 0; j < nychk; j++)
    printf("\t j=%d, ychk=%f\n", j, ychk[j]);
#endif

//-- determine whether polynomial roots are points of intersection
//-- for the two ellipses
    nintpts = 0;
    for (i = 0; i < nychk; i++) {

#if DEBUG
        printf("------------->i=%d (nychk=%d)\n", i, nychk);
#endif
        //-- check for multiple roots
        if ((i < nychk - 1) && fabs(ychk[i] - ychk[i + 1]) < EPS / 2.0) {
#if DEBUG
            printf("i=%d, multiple roots: %f  <--------> %f. continue\n",i,ychk[i], ychk[i-1]);
#endif
            continue;
        }
#if DEBUG
        printf("check intersecting points. nintps is %d", nintpts);
#endif
        //-- check intersection points for ychk[i]
        if (fabs(ychk[i]) > B1)
            x1 = 0.0;
        else
            x1 = A1 * sqrt(1.0 - (ychk[i] * ychk[i]) / (B1 * B1));
        x2 = -x1;
#if DEBUG
        printf("\tx1=%f, y1=%f, A=%f. B=%f ---> ellipse2tr(x1)= %f\n",x1, ychk[i], A1, B1, ellipse2tr(x1, ychk[i], AA, BB, CC, DD, EE, FF) );
        printf("\tx2=%f, y1=%f, A=%f. B=%f ---> ellipse2tr(x2) %f\n",x2, ychk[i], A1, B1, ellipse2tr(x2, ychk[i], AA, BB, CC, DD, EE, FF) );
#endif
        if (fabs(ellipse2tr(x1, ychk[i], AA, BB, CC, DD, EE, FF)) < EPS) {
            nintpts++;
#if DEBUG
            printf("first if x1. acc nintps=%d\n",nintpts);
#endif
            if (nintpts > 4) {
                (*rtnCode) = ERROR_INTERSECTION_PTS;
                return -1.0;
            }
            xint[nintpts - 1] = x1;
            yint[nintpts - 1] = ychk[i];
#if DEBUG
            printf("nintpts=%d, xint=%f, x2=%f, i=%d, yint=%f\n",nintpts, x1, x2, i, ychk[i]);
#endif
        }

        if ((fabs(ellipse2tr(x2, ychk[i], AA, BB, CC, DD, EE, FF)) < EPS)
                && (fabs(x2 - x1) > EPS)) {
            nintpts++;
#if DEBUG
            printf("first if x2. nintps=%d, Dx=%f (eps2=%f) \n",nintpts, fabs (x2 - x1), EPS);
#endif
            if (nintpts > 4) {
                (*rtnCode) = ERROR_INTERSECTION_PTS;
                return -1.0;
            }
            xint[nintpts - 1] = x2;
            yint[nintpts - 1] = ychk[i];
#if DEBUG
            printf("nintpts=%d, x1=%f, xint=%f, i=%d, yint=%f\n",nintpts, x1, x2, i, ychk[i]);
#endif
        }
    }
    //write intersection points
    for (i = 0; i < nintpts; i++) {
        X[i] = xint[i];
        Y[i] = yint[i];
    }
    *NROOTS = nintpts;
//=======================================================================
//== HANDLE ALL CASES FOR THE NUMBER OF INTERSCTION POINTS ==============
//=========================================xs==============================
    switch (nintpts) {
    case 0:
    case 1:
        OverlapArea = nointpts(A1, B1, A2, B2, H1, K1, H2, K2, PHI_1, PHI_2,
                H2_TR, K2_TR, AA, BB, CC, DD, EE, FF, rtnCode);
        /* free (ychk); */
        /* free(xint); */
        /* free(yint); */
        return OverlapArea;

    case 2:
        //-- when there are two intersection points, it is possible for
        //-- them to both be tangents, in which case one of the ellipses
        //-- is fully contained within the other.  Check the points for
        //-- tangents; if one of the points is a tangent, then the other
        //-- must be as well, otherwise there would be more than 2
        //-- intersection points.
        fnRtnCode = istanpt(xint[0], yint[0], A1, B1, AA, BB, CC, DD, EE, FF);

        if (fnRtnCode == TANGENT_POINT) {
#if DEBUG
            printf("one point is tangent\n");
#endif
            OverlapArea = nointpts(A1, B1, A2, B2, H1, K1, H2, K2, PHI_1, PHI_2,
                    H2_TR, K2_TR, AA, BB, CC, DD, EE, FF, rtnCode);

        } else {
            OverlapArea = twointpts(xint, yint, A1, B1, PHI_1, A2, B2, H2_TR,
                    K2_TR, PHI_2, AA, BB, CC, DD, EE, FF, rtnCode);
#if DEBUG
            printf("check twointpts\n");
#endif
        }
        return OverlapArea;

    case 3:
        //-- when there are three intersection points, one and only one
        //-- of the points must be a tangent point.
        OverlapArea = threeintpts(xint, yint, A1, B1, PHI_1, A2, B2, H2_TR,
                K2_TR, PHI_2, AA, BB, CC, DD, EE, FF, rtnCode);
        return OverlapArea;

    case 4:
        //-- four intersections points has only one case.
        OverlapArea = fourintpts(xint, yint, A1, B1, PHI_1, A2, B2, H2_TR,
                K2_TR, PHI_2, AA, BB, CC, DD, EE, FF, rtnCode);
        return OverlapArea;

    default:
        //-- should never get here (but get compiler warning for missing
        //-- return value if this line is omitted)
        (*rtnCode) = ERROR_INTERSECTION_PTS;

        return -1.0;
    } //switch
}


//===========================================================================
//== DEPENDENT FUNCTIONS ====================================================
//===========================================================================

/* qsort double comparison function
   implements quick qort
*/
int double_cmp(const void *a, const void *b)
{
     const double *ia = (const double *)a; // casting pointer types
     const double *ib = (const double *)b;
     return *ia  - *ib;
     /* double comparison: returns negative if b > a
       and positive if a > b */
}

