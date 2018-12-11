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

#ifndef __VEINS_VectorLib_H_
#define __VEINS_VectorLib_H_

using namespace std;

class Vector {
public:
    Vector();
    Vector(double, double);
    double x;
    double y;
    Vector add(Vector v);
    Vector sub(Vector v);
    double cross(Vector v);
};

#endif
