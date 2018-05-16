#ifndef _GRIDCOORD_H
#define _GRIDCOORD_H

#include "veins/base/utils/Coord.h"

/**
 * @brief Represents a position inside a grid.
 *
 * Originally internal helper class of BaseConnectionManager, now also used by other classes.
 * This class provides some converting functions from a Coord to a GridCoord.
 */
class GridCoord
{
public:
    /** @name Coordinates in the grid.*/
    /*@{*/
    int x;
    int y;
    int z;
    /*@}*/

public:
    /**
     * @brief Initialize this GridCoord with the origin.
     * Creates a 3-dimensional coord.
     */
    GridCoord()
        :x(0), y(0), z(0) {};

    /**
     * @brief Initialize a 2-dimensional GridCoord with x and y.
     */
    GridCoord(int x, int y)
        :x(x), y(y), z(0) {};

    /**
     * @brief Initialize a 3-dimensional GridCoord with x, y and z.
     */
    GridCoord(int x, int y, int z)
        :x(x), y(y), z(z) {};

    /**
     * @brief Simple copy-constructor.
     */
    GridCoord(const GridCoord& o) {
        x = o.x;
        y = o.y;
        z = o.z;
    }

    /**
     * @brief Creates a GridCoord from a given Coord by dividing the
     * x,y and z-values by "gridCellWidth".
     * The dimension of the GridCoord depends on the Coord.
     */
    GridCoord(const Coord& c, const Coord& gridCellSize = Coord(1.0,1.0,1.0)) {
        x = static_cast<int>(c.x / gridCellSize.x);
        y = static_cast<int>(c.y / gridCellSize.y);
        z = static_cast<int>(c.z / gridCellSize.z);
    }

    /** @brief Output string for this coordinate.*/
    std::string info() const {
        std::stringstream os;
        os << "(" << x << "," << y << "," << z << ")";
        return os.str();
    }

    /** @brief Comparison operator for coordinates.*/
    friend bool operator==(const GridCoord& a, const GridCoord& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    /** @brief Comparison operator for coordinates.*/
    friend bool operator!=(const GridCoord& a, const GridCoord& b) {
        return !(a==b);
    }
};

#endif
