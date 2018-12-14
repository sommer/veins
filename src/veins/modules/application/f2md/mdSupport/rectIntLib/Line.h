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


#ifndef __VEINS_LineLib_H_
#define __VEINS_LineLib_H_

using namespace std;

#include <veins/modules/application/f2md/mdSupport/rectIntLib/Vector.h>

class Line {
public:
    Line(Vector, Vector);
    double a;
    double b;
    double c;
    double call(Vector p);
    Vector intersection(Line other);
};

#endif
