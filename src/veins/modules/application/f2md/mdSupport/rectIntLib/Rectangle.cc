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

#include <veins/modules/application/f2md/mdSupport/rectIntLib/Rectangle.h>

Rectangle::Rectangle(double cx, double cy, double w, double h, double r){
    this->cx = cx;
    this->cy = cy;
    this->w = w;
    this->h = h;
    this->r = r;
}
