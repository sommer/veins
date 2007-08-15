/***************************************************************************
 * file:        Coord.h
 *
 * author:      Christian Frank
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#ifndef _COORD_H
#define _COORD_H

#include <omnetpp.h>
#include "FWMath.h"

/**
 * @brief Class for storing host positions
 *
 * Class for a storing a position / vector.
 * Some comparison and basic arithmetic operators on Coord
 * structures are implemented.
 *
 * Note:
 * This class can work 2-dimensional or 3-dimensional.
 * The dimension of a Coord can only be set at construction time.
 * The only way to change it afterwards is by assigning Coord a
 * new value with the overloaded "="-operator.
 * So every time you create a new Coord variable you should make sure to
 * use the proper constructor.
 * Since most time you won't have own constructors in Omnet-modules every
 * member variable of type Coord becomes 3D at the beginning (because
 * the default-constructor creates a 3D-Coord).
 * In this case you should give it the right dimension by assigning them
 * a new value in the "initialize()"-Method by a line like this:
 *
 * coordMember = Coord(x, y);    <- 2D
 * coordMember = Coord(x, y, z); <- 3D
 *
 * See the constructors for more details.
 *
 * @ingroup support
 * @author Christian Frank
 */
class Coord : public cPolymorphic
{
public: 
	
	static const double UNDEFINED = 0.0; 
protected:
    /** @brief x and y coordinates of the position*/
    double x, y, z;

    bool use2DFlag;
    
public:

    /**
     * Initialize a 3d (default) or 2d coordinate
     * with the origin
     */
    Coord(bool use2D = false)
        :x(0.0), y(0.0), use2DFlag(use2D)
    {
        
        if (use2D) {
            x = UNDEFINED;
        } else {
            x = 0.0;
        }
    }
    
    /** Initializes 3D coordinate.*/
    Coord(double _x, double _y, double _z)
        : x(_x), y(_y), z(_z) , use2DFlag(false) {}

    /** Initializes 2D coordinate.*/
    Coord(double _x, double _y)
        : x(_x), y(_y), z(UNDEFINED) , use2DFlag(true) {}


    /** Initializes coordinate from other coordinate.*/
    Coord( const Coord& pos )
        :x(pos.x), y(pos.y), z(pos.z), use2DFlag(pos.use2DFlag) {}

    /** Initializes coordinate from other coordinate.*/
    Coord( const Coord* pos )
        :x(pos->x), y(pos->y), z(pos->z), use2DFlag(pos->use2DFlag) {}

    std::string info() const {
        std::stringstream os;
        if (use2DFlag) {
            os << "(" << x << "," << y << ")";
        } else {
            os << "(" << x << "," << y << "," << z << ")";
        }
        return os.str();
    }

    /** Adds two coordinate vectors.*/
    friend Coord operator+(const Coord& a, const Coord& b) {
        Coord tmp = a;
        tmp += b;
        return tmp;
    }

    /** Subtracts two coordinate vectors.*/
    friend Coord operator-(const Coord& a, const Coord& b) {
        Coord tmp = a;
        tmp -= b;
        return tmp;
    }

    /** Multiplies a coordinate vector by a real number.*/
    friend Coord operator*(const Coord& a, double f) {
        return Coord(a.x*f, a.y*f, a.z*f);
    }

    /** Divides a coordinate vector by a real number.*/
    friend Coord operator/(const Coord& a, double f) {
        return Coord(a.x/f, a.y/f, a.z/f);
    }

    /** Adds coordinate vector b to a.*/
    Coord operator+=(const Coord& a) {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }

    /** Assigns a this.*/
    Coord operator=(const Coord& a) {
        x = a.x;
        y = a.y;
        z = a.z;
        use2DFlag = a.use2DFlag;
        return *this;
    }

    /** Subtracts coordinate vector b from a.*/
    Coord operator-=(const Coord& a) {
        x -= a.x;
        y -= a.y;
	z -= a.z;
        return *this;
    }

    /**
     * Tests whether two coordinate vectors are equal. Because
     * coordinates are of type double, this is done through the
     * FWMath::close function.
     */
    friend bool operator==(const Coord& a, const Coord& b) {
        return FWMath::close(a.x, b.x) && FWMath::close(a.y, b.y) && FWMath::close(a.z, b.z);
    }

    /**
     * Tests whether two coordinate vectors are not equal. Negation of
     * the operator==.
     */
    friend bool operator!=(const Coord& a, const Coord& b) {
        return !(a==b);
    }

    /**
     * Returns the distance to Coord a
     */
    double distance( const Coord& a ) const {
        Coord dist=*this-a;
        return dist.length();
    }

    /**
     * Returns distance^2 to Coord a (omits square root).
     */
    double sqrdist( const Coord& a ) const {
        Coord dist=*this-a;
        return dist.squareLength();
    }

    /**
     * Returns the squared distance on a torus to Coord a (omits square root).
     * TODO make 3D
     */
    double sqrTorusDist(const Coord& b, const Coord& playgroundSize) const {
        /*double minTorDist;
        double torDist;

        torDist = FWMath::torDist(this->x,              b.x, this->y,                  b.y);
        minTorDist = torDist;
        torDist = FWMath::torDist(this->x+playGround.x, b.x, this->y,                  b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x-playGround.x, b.x, this->y,                  b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x,              b.x, this->y+playGround.y,     b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x,              b.x, this->y-playGround.y,     b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x+playGround.x, b.x, this->y+playGround.y,     b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x+playGround.x, b.x, this->y-playGround.y,     b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x-playGround.x, b.x, this->y+playGround.y,     b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        torDist = FWMath::torDist(this->x-playGround.x, b.x, this->y-playGround.y,     b.y);
        if(torDist < minTorDist) minTorDist = torDist;
        return minTorDist; */

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

    /**
     * Returns the square of the length of this coords position vector
     */
    double squareLength()
    {
        return x * x + y * y + z * z;
    }

    /**
     * Returns the length of this coords position vector
     */
    double length()
    {
        return sqrt(squareLength());
    }

    /**
     * Getter for the x coordinate
     */
    double getX() const{ return x; }

    /**
     * Setter for the x coordinate
     */
    void setX(double x){this->x = x;}

    /**
     * Getter for the y coordinate
     */
    double getY() const{ return y; }

    /**
     * Setter for the y coordinate
     */
    void setY(double y){this->y = y;}

    /**
     * Getter for the z coordinate
     */
    double getZ() const{ return z; }

    /**
     * Setter for the z coordinate
     */
    void setZ(double z){this->z = z;}

    /**
     * Returns true if this coordinate is valid, this means
     * x, y and z are unequal UNDEFINED.
     */
    /*bool isValid() {
        return x != UNDEFINED && y != UNDEFINED && (z != UNDEFINED || is2DFlag);
    }*/

    /**
     * Returns true if this coordinate is two dimensional
     */
    bool is2D() { return use2DFlag; }

    /**
     * Checks if this coordinate is inside a specified rectangle.
     *
     * @param upperLeftCorner The upper left corner of the rectangle.
     * @param lowerRightCorner the lower right corner of the rectangle.
     */
    bool isInRectangle(const Coord& upperLeftCorner, const Coord& lowerRightCorner) {
        return  x >= upperLeftCorner.x && x <= lowerRightCorner.x &&
                y >= upperLeftCorner.y && y <= lowerRightCorner.y &&
                (use2DFlag || (z >= upperLeftCorner.z && z <= lowerRightCorner.z));
    }
};

#endif

