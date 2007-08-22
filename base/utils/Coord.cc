#include "Coord.h"

// Workaround for C2864
const double Coord::UNDEFINED = 0.0;

/**
 * Returns the squared distance on a torus to Coord a (omits square root).
 *
 * Does not check for dimension compatibility!
 */
double Coord::sqrTorusDist(const Coord& b, const Coord& playgroundSize) const {

    Coord dist = *this - b;

    /*
     * on a torus the end and the begin of the axes are connected so you
     * get a circle. On a circle the distance between two points can't be greater
     * than half of the circumference.
     * If the normal distance between two points on one axis is bigger than
     * half of the playground there must be a "shorter way" over the playground
     * border on this axis
     */
    if(fabs(dist.x) > (playgroundSize.x * 0.5))
    {
        if(dist.x < 0.0) //check which point is nearer to the "end of the axis"
        {
            dist.x = playgroundSize.x - b.x + x; //coordinate distance over border is
        } else {                                 //calced by:
            dist.x = playgroundSize.x - x + b.x; //axis_end - bigger_coordinate + smaller_coordinate
        }
    }
    //same for y- and z-coordinate
    if(fabs(dist.y) > (playgroundSize.y * 0.5))
    {
        if(dist.y < 0.0) //check which point is nearer to the "end of the axis"
        {
            dist.y = playgroundSize.y - b.y + y; //coordinate distance over border is
        } else {                                 //calced by:
            dist.y = playgroundSize.y - y + b.y; //axis_end - bigger_coordinate + smaller_coordinate
        }
    }
    if(fabs(dist.z) > (playgroundSize.z * 0.5))
    {
        if(dist.z < 0) //check which point is nearer to the "end of the axis"
        {
            dist.z = playgroundSize.z - b.z + z; //coordinate distance over border is
        } else {                                 //calced by:
            dist.z = playgroundSize.z - z + b.z; //axis_end - bigger_coordinate + smaller_coordinate
        }
    }
    return dist.squareLength();
}
