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

#ifndef __VEINS_RectIntLib_H_
#define __VEINS_RectIntLib_H_

#include <veins/modules/application/f2md/mdSupport/rectIntLib/Line.h>
#include <veins/modules/application/f2md/mdSupport/rectIntLib/Rectangle.h>
#include <veins/modules/application/f2md/mdSupport/rectIntLib/Vector.h>
#include <cmath>
#include <iostream>

class RectIntLib {
private:
    void RectangleVertices(Vector* vList, Rectangle r1);
    double IntersectionArea(Rectangle r1, Rectangle r2);

public:

    double RectIntArea(double cx1, double cy1, double w1, double h1, double r1,
            double cx2, double cy2, double w2, double h2, double r2);

};

#endif
