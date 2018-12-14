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

#include <veins/modules/application/f2md/mdSupport/rectIntLib/Line.h>

Line::Line(Vector v1, Vector v2){
    this->a = v2.y - v1.y;
    this->b = v1.x - v2.x;
    this->c = v2.cross(v1);
}

double Line::call(Vector p){
    return this->a*p.x + this->b*p.y + this->c;
}

Vector Line::intersection(Line other){
    double w = this->a*other.b - this->b*other.a;
    return Vector((this->b*other.c - this->c*other.b)/w,
            (this->c*other.a - this->a*other.c)/w);
}
