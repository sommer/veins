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

#include <veins/modules/application/f2md/mdSupport/rectIntLib/RectIntLib.h>

#define PI 3.14159265358979323846264338327950288419716939937510  /* pi */

using namespace std;

void RectIntLib::RectangleVertices(Vector* vList, Rectangle r1){
    double angle = PI*r1.r/180;

    double dx =  r1.w/2;
    double dy =  r1.h/2;

    double dxcos = dx*cos(angle);
    double dxsin = dx*sin(angle);
    double dycos = dy*cos(angle);
    double dysin = dy*sin(angle);


    Vector VBase(r1.cx, r1.cy);

    Vector V1(-dxcos - -dysin, -dxsin + -dycos);
    Vector V2( dxcos - -dysin,  dxsin + -dycos);
    Vector V3( dxcos -  dysin,  dxsin +  dycos);
    Vector V4(-dxcos -  dysin, -dxsin +  dycos);

    vList[0] = VBase.add(V1);
    vList[1] = VBase.add(V2);
    vList[2] = VBase.add(V3);
    vList[3] = VBase.add(V4);

}

double RectIntLib::IntersectionArea(Rectangle r1, Rectangle r2){

    Vector rect1[4];
    Vector rect2[4];

    RectangleVertices(rect1, r1);
    RectangleVertices(rect2, r2);

    Vector intersection[100];

    for(int i = 0; i< 4; i ++){
        intersection[i] = rect1[i];
    }

    int lenIntersection = 4;
    Vector new_intersection[100];
    int lenIntersectionTemp = 0;
    for(int i = 0; i< 4 ; i++){

        if(lenIntersection<= 2){
            break;
        }

        Vector p = rect2[i];
        Vector q;
        if(i<4-1){
         q = rect2[i+1];
        }else{
         q = rect2[0];
        }

        Line line(p, q);

        double line_values[lenIntersection];
        for(int var = 0; var<lenIntersection; var++){
            line_values[var] = line.call(intersection[var]);
        }


        for(int j = 0;  j < lenIntersection ; j++){
            Vector s = intersection[j];
            Vector t;
            if(j<lenIntersection - 1){
                t = intersection[j+1];
            }else{
                t = intersection[0];
            }

            double s_value = line_values[j];
            double t_value;
            if(j < lenIntersection - 1){
                t_value = line_values[j+1];
            }else{
                t_value = line_values[0];
            }

            if(s_value<=0){
                new_intersection[lenIntersectionTemp]= s;
                lenIntersectionTemp++;
            }

            if((s_value * t_value) < 0){
                Line newLine(s, t);
                Vector intersection_point = line.intersection(newLine);
                new_intersection[lenIntersectionTemp] = intersection_point;
                lenIntersectionTemp++;
            }
        }

        for(int var = 0; var<lenIntersectionTemp; var++){
            intersection[var] = new_intersection[var];

        }

        lenIntersection = lenIntersectionTemp;
        lenIntersectionTemp = 0;

    }

    if(lenIntersection<=2){
        return 0;
    }

    double sum = 0;
    for(int i = 0; i < lenIntersection ; i++){
        Vector p = intersection[i];
        Vector q;
        if(i<lenIntersection - 1){
            q = intersection[i+1];
        }else{
            q = intersection[0];
        }
        sum = sum + p.x*q.y - p.y*q.x ;
    }
    return 0.5 * sum;
}


double RectIntLib::RectIntArea(double cx1, double cy1, double w1, double h1, double r1,
        double cx2, double cy2, double w2, double h2, double r2){
    Rectangle rect1(cx1, cy1, w1, h1, r1);
    Rectangle rect2(cx2, cy2, w2, h2, r2);

    return IntersectionArea(rect1, rect2);
}

