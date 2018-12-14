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

#include <veins/modules/application/f2md/mdSupport/ellipseIntLib/EllipseIntLib.h>

using namespace std;

#define PI 3.14159265358979323846264338327950288419716939937510  /* pi */

double EllipseIntLib::EllipseIntArea(double cx1, double cy1, double w1, double h1, double r1,
        double cx2, double cy2, double w2, double h2, double r2){

    double angle1 = PI*r1/180;
    double angle2 = PI*r2/180;

    double dx1 =  w1/2;
    double dy1 =  h1/2;

    double dx2 =  w2/2;
    double dy2 =  h2/2;

    double x_toms[4], y_toms[4];
    int nroots_toms;
    int rtn = 0;

    double area = ellipse_ellipse_overlap_netlibs(angle1,dx1,dy1,cx1,cy1,angle2,dx2,dy2,cx2,cy2,x_toms, y_toms, &nroots_toms, &rtn);

    return area;
}

