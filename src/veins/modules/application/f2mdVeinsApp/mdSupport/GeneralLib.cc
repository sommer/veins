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

#include <veins/modules/application/f2mdVeinsApp/mdSupport/GeneralLib.h>

GeneralLib::GeneralLib(){
}

Coord GeneralLib::TypeToSize(std::string type){

    return Coord(1.8,2,0);

    //lust
    if (type == "passenger1")
        return Coord(1.8,5.0 - 1.5 -0,0);
    if (type == "passenger2a")
        return Coord(1.8,4.5- 1.5 -0,0);
    if (type == "passenger2b")
        return Coord(1.8,4.5- 1.5 -0,0);
    if (type == "passenger3")
        return Coord(1.8,6.0- 1.5 -0,0);
    if (type == "passenger4")
        return Coord(1.8,5.5- 1.5 -0,0);
    if (type == "passenger5")
        return Coord(1.8,7.0- 2.5 -0,0);
    if (type == "bus")
        return Coord(1.8,12-3 -0,0);

    // irt
    if (type == "DEFAULT_VEHTYPE")
        return Coord(1.8,2.5 -0,0);
    if (type == "bait")
        return Coord(1.8,2.5 -0,0);
    if (type == "victim")
        return Coord(1.8,2.5 -0,0);
    if (type == "attacker")
        return Coord(1.8,2.5 -0,0);

}

double GeneralLib::RandomDouble(double fMin, double fMax)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    random::mt19937 rng(tm.tv_nsec);
    std::uniform_real_distribution<> one(fMin,fMax);

    double f = one(rng);
    return f;
}

int GeneralLib::RandomInt(int min, int max)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME, &tm);

    random::mt19937 rng(tm.tv_nsec);
    std::uniform_int_distribution<> one(min,max);

    int guess = one(rng);
    return guess;
}
