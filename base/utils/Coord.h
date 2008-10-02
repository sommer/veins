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
 * Most methods of Coord does not check for dimension compatibility.
 * For example the overloaded "+"-operator does not check if both
 * Coord are of the same dimension. The user has to assure the
 * proper use of this methods!
 *
 * @ingroup support
 * @author Christian Frank
 */
class Coord : public cObject
{
public:
	static const double UNDEFINED;

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
            z = UNDEFINED;
        } else {
            z = 0.0;
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
        : cObject(pos), x(pos.x), y(pos.y), z(pos.z), use2DFlag(pos.use2DFlag) {}

    /** Initializes coordinate from other coordinate.*/
    Coord( const Coord* pos )
        : cObject(*pos), x(pos->x), y(pos->y), z(pos->z), use2DFlag(pos->use2DFlag) {}

    /** Returns a string with the value of the coordinate.*/
    std::string info() const {
        std::stringstream os;
        if (use2DFlag) {
            os << "(" << x << "," << y << ")";
        } else {
            os << "(" << x << "," << y << "," << z << ")";
        }
        return os.str();
    }

    /**
     * Adds two coordinate vectors. Does not check for dimension
     * compatibility!
     */
    friend Coord operator+(const Coord& a, const Coord& b) {
        Coord tmp = a;
        tmp += b;
        return tmp;
    }

    /**
     * Subtracts two coordinate vectors. Does not check for 
     * dimension compatibility!
     */
    friend Coord operator-(const Coord& a, const Coord& b) {
        Coord tmp = a;
        tmp -= b;
        return tmp;
    }

    /** Multiplies a coordinate vector by a real number.*/
    friend Coord operator*(const Coord& a, double f) {
		Coord tmp = a;
		tmp *= f;
        return tmp;
    }

    /** Divides a coordinate vector by a real number.*/
    friend Coord operator/(const Coord& a, double f) {
		Coord tmp = a;
		tmp /= f;
        return tmp;
    }

    /**
     * Multiplies a coordinate vector by a real number.
     */
    Coord operator*=(double f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    /**
     * Divides a coordinate vector by a real number.
     */
    Coord operator/=(double f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    /**
     * Adds coordinate vector b to a.
     * Does not check for dimension compatibility!
     */
    Coord operator+=(const Coord& a) {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }

    /**
     * Assigns a this.
     * This operator can change the dimension of the coordinate.
     */
    Coord operator=(const Coord& a) {
        x = a.x;
        y = a.y;
        z = a.z;
        use2DFlag = a.use2DFlag;
        return *this;
    }

    /**
     * Subtracts coordinate vector b from a.
     * Does not check for dimension compatibility!
     */
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
     *
     * Does not check for dimension compatibility!
     */
    friend bool operator==(const Coord& a, const Coord& b) {
        return FWMath::close(a.x, b.x) && FWMath::close(a.y, b.y) && FWMath::close(a.z, b.z);
    }

    /**
     * Tests whether two coordinate vectors are not equal. Negation of
     * the operator==.
     *
     * Does not check for dimension compatibility!
     */
    friend bool operator!=(const Coord& a, const Coord& b) {
        return !(a==b);
    }

    /**
     * Returns the distance to Coord a.
     * Does not check for dimension compatibility!
     */
    double distance( const Coord& a ) const {
        Coord dist=*this-a;
        return dist.length();
    }

    /**
     * Returns distance^2 to Coord a (omits square root).
     * Does not check for dimension compatibility!
     */
    double sqrdist( const Coord& a ) const {
        Coord dist=*this-a;
        return dist.squareLength();
    }

    /**
     * Returns the squared distance on a torus to Coord a (omits square root).
     *
     * Does not check for dimension compatibility!
     */
    double sqrTorusDist(const Coord& b, const Coord& playgroundSize) const;

    /**
     * Returns the square of the length of this coords position vector
     */
    double squareLength() const
    {
        return x * x + y * y + z * z;
    }

    /**
     * Returns the length of this coords position vector
     */
    double length() const
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
     * Getter for the z coordinate.
     *
     * This method should return Coord::UNDEFINED if
     * this is a two-dimensional coordinate. If not,
     * the proper function of the Coord-methods
     * can't be assured.
     */
    double getZ() const{ return z; }

    /**
     * Setter for the z coordinate.
     *
     * This method does not check its dimension!
     * So never call it if you are working with
     * two-dimensional coordinates!
     * A 2D-Coord looses its functionality if the
     * z-value becomes another value than
     * Coord::UNDEFINED
     */
    void setZ(double z){this->z = z;}

    /**
     * Returns true if this coordinate is valid.
     * Valid means this Coord is 3-dimensional or
     * this Coord is 2-Dimensional and the z-value
     * i equal to Coord::UNDEFINED.
     */
    bool isValid() const {
        return (z == UNDEFINED) || !use2DFlag;
    }

    /**
     * Returns true if this coordinate is two-dimensional
     */
    bool is2D() const { return use2DFlag; }

    /**
     * Returns true if this coordinate is three-dimensional
     */
    bool is3D() const { return !use2DFlag; }

    /**
     * Checks if this coordinate is inside a specified rectangle.
     *
     * Does not check for dimension compatibility!
     *
     * @param upperLeftCorner The upper left corner of the rectangle.
     * @param lowerRightCorner the lower right corner of the rectangle.
     */
    bool isInRectangle(const Coord& upperLeftCorner, const Coord& lowerRightCorner) const {
        return  x >= upperLeftCorner.x && x <= lowerRightCorner.x &&
                y >= upperLeftCorner.y && y <= lowerRightCorner.y &&
                (use2DFlag || (z >= upperLeftCorner.z && z <= lowerRightCorner.z));
    }

    /**
     * Returns the minimal coordinates.
     */
    Coord min(const Coord& a) {
        Coord tmp = *this;
	tmp.setX(this->x < a.x ? this->x : a.x);
	tmp.setY(this->y < a.y ? this->y : a.y);
	if (tmp.is3D())
		tmp.setZ(this->z < a.z ? this->z : a.z);
        return tmp;
    }

    /**
     * Returns the maximal coordinates.
     */
    Coord max(const Coord& a) {
        Coord tmp = *this;
	tmp.setX(this->x > a.x ? this->x : a.x);
	tmp.setY(this->y > a.y ? this->y : a.y);
	if (tmp.is3D())
		tmp.setZ(this->z > a.z ? this->z : a.z);
        return tmp;
    }

    /**
     * Tests whether this coordinate vector is strictly larger
     * than another coordinate vector.
     */
    friend bool operator>(const Coord& a, const Coord& b) {
	return (a.x > b.x && 
		a.y > b.y && 
		(a.is3D() ? a.z > b.z : true));
    }

    /**
     * Tests whether this coordinate vector is strictly smaller
     * than another coordinate vector.
     */
    friend bool operator<(const Coord& a, const Coord& b) {
	return (a.x < b.x && 
		a.y < b.y && 
		(a.is3D() ? a.z < b.z : true));
    }
};

#endif

