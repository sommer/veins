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

#include <veins/modules/application/f2md/mdSupport/rectIntLib/Vector.h>

Vector::Vector() {
    this->x = 0;
    this->y = 0;
}

Vector::Vector(double x, double y) {
    this->x = x;
    this->y = y;
}

Vector Vector::add(Vector v) {
    return Vector(this->x + v.x, this->y + v.y);
}

Vector Vector::sub(Vector v) {
    return Vector(this->x - v.x, this->y - v.y);
}

double Vector::cross(Vector v) {
    return this->x * v.y - this->y * v.x;
}
