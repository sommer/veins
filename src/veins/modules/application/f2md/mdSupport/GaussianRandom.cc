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

#include <veins/modules/application/f2md/mdSupport/GaussianRandom.h>
#define EPSILON_G 0.05

GaussianRandom::GaussianRandom(Coord* curPosConfidence, Coord* curSpeedConfidence, Coord* curHeadingConfidence) {
    this->curPosConfidence = curPosConfidence;
    this->curSpeedConfidence = curSpeedConfidence;
    this->curHeadingConfidence = curHeadingConfidence;
}

double GaussianRandom::getGaussianRand(double mean, double stddev) {

    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    random::mt19937 rng(tm.tv_nsec);

    std::normal_distribution<> d{mean,stddev};
    return d(rng);
}

Coord GaussianRandom::OffsetPosition(Coord curPosition) {

    double r =  getGaussianRand(0, curPosConfidence->x/3);

    GeneralLib genLib = GeneralLib();

    //non gaussian
    //r = genLib.RandomDouble(-curPosConfidence, curPosConfidence);

    double theta = genLib.RandomDouble(0, 2 * PI);

    while(r>=(curPosConfidence->x+EPSILON_G) || r<=-(curPosConfidence->x+EPSILON_G) ){
        r =  getGaussianRand(0, curPosConfidence->x/3);
    }

    double deltaX = r * cos(theta);
    double deltaY = r * sin(theta);
//    std::cout << "========== r:"<<r<<" theta:"<< theta << '\n';
//    std::cout<< "deltaX:" << deltaX  << '\n';
//    std::cout<< "deltaY:" << deltaY  << '\n';

    return Coord(curPosition.x + deltaX, curPosition.y + deltaY, 0);

}

Coord GaussianRandom::OffsetSpeed(Coord curSpeed) {

    double deltaVx =  getGaussianRand(0, curSpeedConfidence->x/3);
    double deltaVy =  getGaussianRand(0, curSpeedConfidence->y/3);
//    double deltaVz =  getGaussianRand(0, curSpeedConfidence.z/3);

//    if(deltaVx>curSpeedConfidence.x ){
//        deltaVx = curSpeedConfidence.x;
//    }
//    if(deltaVx<-curSpeedConfidence.x ){
//        deltaVx = -curSpeedConfidence.x;
//    }
//    if(deltaVy>curSpeedConfidence.y ){
//        deltaVy = curSpeedConfidence.y;
//    }
//    if(deltaVy<-curSpeedConfidence.y ){
//        deltaVy = -curSpeedConfidence.y;
//    }
//    if(deltaVz>curSpeedConfidence.z ){
//        deltaVz = curSpeedConfidence.z;
//    }
//    if(deltaVz<-curSpeedConfidence.z ){
//        deltaVz = -curSpeedConfidence.z;
//    }

  //  deltaVy = deltaVx;

//    std::cout << "========== speed: "<<curSpeed.x<<" "<< curSpeed.y << '\n';
//    std::cout<< "deltaVx:" << deltaVx  << '\n';
//    std::cout<< "deltaVy:" << deltaVy  << '\n';

    return Coord(curSpeed.x + deltaVx, curSpeed.y + deltaVy, 0);

}

Coord GaussianRandom::OffsetHeading(Coord curHeading) {

    MDMLib mdmLib = MDMLib();

    double headingAngle = mdmLib.calculateHeadingAngle(curHeading);

    double angle =  getGaussianRand(0, curHeadingConfidence->x/3);

    headingAngle = headingAngle + angle;

    double x = cos(headingAngle*PI/180);
    double y = sin(headingAngle*PI/180);

    return Coord(x,-y,curHeading.z);

}




