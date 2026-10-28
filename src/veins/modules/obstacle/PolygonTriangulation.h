#include <vector>
#include <set>
#include "veins/base/utils/Coord.h"

namespace veins {

struct Triangle {
    Coord p1;
    Coord p2;
    Coord p3;
};

struct Segment {
    Coord p1;
    Coord p2;
};


struct Vertex {
    Coord pos;
    int index;

    bool operator<(const Vertex& other) const {
        return pos.x < other.pos.x || (pos.x == other.pos.x && pos.y < other.pos.y);
    }
};

inline Coord computeCrossProduct(const Coord& v1, const Coord& v2) {
    return Coord(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    );
}

inline double computeDotProduct(const Coord& v1, const Coord& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/**
 * @brief Class for storing edges of 2D polygons
 *
 * Some comparison and basic arithmetic operators are implemented.
 *
 */
class VEINS_API Edge
{
private:
    Coord* helper; // sweep line helper vertex
    int* monotonePolygonIndex; // index of the relative monotone polygon  
    int* mergeMonotonePolygonIndex; // index of the relative monotone polygon of the last seen merge vertex

public:
    Coord start;
    Coord end;
    
    /** @brief Initializes an empty edge. */
    Edge() {
        helper = new Coord;
        monotonePolygonIndex = new int(-1);
        mergeMonotonePolygonIndex = new int(-1);
    }

    /** @brief Initializes an edge from the leftmost Coord to the righmost one, based on x position. */
    Edge(const Coord& vertex1, const Coord& vertex2) : Edge() {
        if (vertex1.x < vertex2.x) {
            start = vertex1;
            end = vertex2;
        } else {
            start = vertex2;
            end = vertex1;
        }
    }

    /** @brief Initializes an edge with another coordinate as a helper for the triangulation process. */
    Edge(const Coord& vertex1, const Coord& vertex2, const Coord& help) : Edge(vertex1,vertex2) 
    {    
        *helper = help;
    }
    /** @brief Initializes an edge with a helper and the relative monotone polygon index for the triangulation process. */
    Edge(const Coord& vertex1, const Coord& vertex2, const Coord& help, int index) : Edge(vertex1,vertex2,help) 
    {    
        *monotonePolygonIndex = index;
    }

    /** @brief Copy constructor for the Edge class */
    Edge(const Edge& other)
    {
        start = other.start;
        end = other.end;

        helper = new Coord(*other.helper);
        monotonePolygonIndex = new int(*other.monotonePolygonIndex);
        mergeMonotonePolygonIndex = new int(*other.mergeMonotonePolygonIndex);
    }

    /**
     * @brief Comparison operator for sorting edge events based on y-coordinate.
     *
     * This comparison operator is used to order Edge objects based on their y-coordinates,
     * It ensures that when a collection of Edge objects is sorted using this operator,
     * they will be arranged in increasing order of their y-coordinates.
     * If two edges have the same y-coordinates, the operator considers their x-coordinates
     *
     * @param other The Edge object to compare against.
     * @return True if this Edge should be placed before the 'other' Edge, otherwise False.
     */
    bool operator<(const Edge& other) const
    {
        if(this->start.x == other.start.x && this->end.x == other.end.x && 
           this->start.y == other.start.y && this->end.y == other.end.y)
            return false;
        double thisMinY = std::min(start.y, end.y);
        double thisMaxY = std::max(start.y, end.y);

        double otherMinY = std::min(other.start.y, other.end.y);
        double otherMaxY = std::max(other.start.y, other.end.y);
        if (thisMaxY <= otherMinY) {
            return true;
        } else if (thisMinY >= otherMaxY) {
            return false;
        } else {
            return start.y < other.start.y;
        }
    }

    bool operator==(const Edge& other) const {
        return start == other.start && end == other.end;
    }

    bool operator!=(const Edge& other) const {
        return !(*this == other);
    }
    
    Coord* getHelper() const {
        return helper;
    }

    void setHelper(const Coord& newHelper) const {
        *helper = newHelper;
    }

    int getMonotonePolygonIndex() const {
        return *monotonePolygonIndex;
    }

    void setMonotonePolygonIndex(int newIndex) const {
        *monotonePolygonIndex = newIndex;
    }

    int getMergeMonotonePolygonIndex() const {
        return *mergeMonotonePolygonIndex;
    }

    void setMergeMonotonePolygonIndex(int newIndex) const {
        *mergeMonotonePolygonIndex = newIndex;
    }

    ~Edge() {
        delete helper;
        delete monotonePolygonIndex;
        delete mergeMonotonePolygonIndex;
    }

};

/** @brief Calculate arc length between two indices in a cyclic array */
int arc_length(int i, int j, int N);


/**
 * @brief Enumeration of vertex types used in polygon processing.
 *
 * This enumeration defines various types of vertices encountered during polygon processing,
 * such as triangulation and partitioning. 
 */
enum vertexType { START, END, MERGE, SPLIT, REGULAR_UPPER, REGULAR_LOWER, VERTICAL };


/**
 * @brief Finds the upper bound of an Edge in a set based on y-coordinate and x-coordinate.
 *
 * This function searches for the upper bound of an Edge object within a set of Edge objects
 * based on their y-coordinates and, if necessary, their x-coordinates. It returns a pointer
 * to the located Edge object. The set is expected to be sorted using the < operator defined
 * for Edge objects, with y-coordinates taking priority and x-coordinates breaking ties.
 *
 * @param y The y-coordinate to search for.
 * @param x The x-coordinate to further refine the search.
 * @param bounds The set of Edge objects to search within.
 * @return A pointer to the upper bound Edge in the set.
 */
const Edge* findUpperBound(double y, double x, std::set<Edge>& bounds);

/**
 * @brief Partitions a polygon into monotone sub-polygons.
 *
 * This function takes a polygon represented by a vector of Coord vertices and partitions it
 * into monotone sub-polygons. 
 * The function returns a vector of vectors of Coord, where each sub-vector
 * represents a monotone sub-polygon.
 *
 * @param polygon The input polygon to be partitioned.
 * @return A vector of monotone sub-polygons' vertices.
 */
std::vector<std::vector<Coord>> partitionPolygonIntoMonotone(std::vector<Coord>& polygon);

/**
 * @brief Triangulates a monotone polygon into triangles.
 *
 * Given a monotone polygon represented by a vector of Coord vertices, this function
 * triangulates the polygon into triangles. The input polygon is assumed to be monotone,
 * and the function returns a vector of Triangle objects representing the resulting triangles.
 *
 * @param polygon The monotone polygon to be triangulated.
 * @return A vector of Triangle objects forming the triangulation.
 */
std::vector<Triangle> triangulateMonotonePolygon(const std::vector<Coord>& polygon);

/**
 * @brief Determines the type of a vertex in relation to its neighbors.
 *
 * This function categorizes a vertex within a polygon based on its position and orientation
 * relative to its neighboring vertices. The vertex can be classified as START, END, SPLIT, MERGE,
 * or REGULAR based on its local configuration.
 *
 * @param vertex The vertex to classify.
 * @param next The next vertex in polygon traversal order.
 * @param prev The previous vertex in polygon traversal order.
 * @return The type of the vertex (START, END, SPLIT, MERGE, or REGULAR).
 */
vertexType getVertexType(const Coord& vertex, const Coord& next, const Coord& prev);

/**
 * @brief Checks if a polygon is oriented in counter-clockwise direction.
 *
 * This function determines whether a polygon represented by a vector of Coord vertices
 * is oriented in a counter-clockwise direction. It applies the "shoelace formula" to calculate
 * the signed area of the polygon and uses its sign to determine orientation.
 *
 * @param vertices The vertices of the polygon.
 * @return True if the polygon is counter-clockwise, False otherwise.
 */
bool isCounterClockwise(const std::vector<Coord>& vertices);


/**
 * @brief Determines the convexity of three points.
 *
 * This function assesses the convexity of three points
 * It calculates the cross product of the vectors formed by p2 - p1 and p3 - p1 to determine the
 * orientation of the points. The function returns 1 if the points form a concave curve, -1 if they
 * form a convex curve, and 0 if the points are aligned.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 * @param p3 The third point.
 * @return  1 if concave, -1 if convex, 0 if aligned.
 */
int DetermineConvexity(const Coord& p1, const Coord& p2, const Coord& p3);

} // namespace veins