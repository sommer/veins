#include "Coord.h"

// Workaround for C2864
// If set to another value than 0.0 one has to check the
// compliance with the overloaded operators
const double Coord::UNDEFINED = 0.0;

double Coord::sqrTorusDist(const Coord& b, const Coord& playgroundSize) const {

    double xDist = fabs(x - b.x);
    double yDist = fabs(y - b.y);
    double zDist = fabs(z - b.z);

    
    /*
     * on a torus the end and the begin of the axes are connected so you
     * get a circle. On a circle the distance between two points can't be greater
     * than half of the circumference.
     * If the normal distance between two points on one axis is bigger than
     * half of the playground there must be a "shorter way" over the playground
     * border on this axis
     */
    if(xDist * 2.0 > playgroundSize.x)
    {
        xDist = playgroundSize.x - xDist;
    }
    //same for y- and z-coordinate
    if(yDist * 2.0 > playgroundSize.y)
    {
    	yDist = playgroundSize.y - yDist;
    }
    if(zDist * 2.0 > playgroundSize.z)
    {
    	zDist = playgroundSize.z - zDist;
    }
    return xDist * xDist + yDist * yDist + zDist * zDist;
}
