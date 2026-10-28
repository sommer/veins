//
// Copyright (C) 2010-2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <algorithm>

#include "veins/modules/obstacle/Obstacle.h"

using namespace veins;

using veins::Obstacle;

Obstacle::Obstacle(std::string id, std::string type, double attenuationPerCut, double attenuationPerMeter)
    : visualRepresentation(nullptr)
    , id(id)
    , type(type)
    , attenuationPerCut(attenuationPerCut)
    , attenuationPerMeter(attenuationPerMeter)
{
}

void Obstacle::setShape(Coords shape, double height )
{
    coords = shape;
    this-> height = height;
    mesh = {};
    bboxP1 = Coord(1e7, 1e7);
    bboxP2 = Coord(-1e7, -1e7);
    Coords::const_iterator i = coords.begin();
    Coords::const_iterator j = i + 1;
    for (; i != coords.end(); j = ++i + 1) {
        bboxP1.x = std::min(i->x, bboxP1.x);
        bboxP1.y = std::min(i->y, bboxP1.y);
        bboxP2.x = std::max(i->x, bboxP2.x);
        bboxP2.y = std::max(i->y, bboxP2.y);

        if(height > 0 && j != coords.end())
        {
            // generating mesh for the walls
            mesh.push_back({ Coord(i->x,i->y,0), Coord(i->x,i->y,height), Coord(j->x,j->y,height)});
            mesh.push_back({ Coord(j->x,j->y,0), Coord(j->x,j->y,height), Coord(i->x,i->y,0)});

        }
    }
    
    Coords shapeWithoutLast(shape.begin(), shape.end() - 1);
    for(Coords monotonePolygon : partitionPolygonIntoMonotone(shapeWithoutLast))
    {
    	for(Triangle triangle : triangulateMonotonePolygon(monotonePolygon))
    	{
    		triangle.p1.z = 0;triangle.p2.z = 0;triangle.p3.z = 0;
		    mesh.push_back(triangle);
		    triangle.p1.z = height;triangle.p2.z = height;triangle.p3.z = height;
		    mesh.push_back(triangle);
    	}
    }	
}

const Obstacle::Coords& Obstacle::getShape() const
{
    return coords;
}

const Obstacle::Mesh& Obstacle::getMesh() const
{
    return mesh;
}

const double Obstacle::getHeight() const
{
    return height;
}

const Coord Obstacle::getBboxP1() const
{
    return bboxP1;
}

const Coord Obstacle::getBboxP2() const
{
    return bboxP2;
}

bool Obstacle::containsPoint(Coord point) const
{
    bool isInside = false;
    if(point.z < 0 || (point.z > getHeight() && getHeight() > 0)) return false;
    const Obstacle::Coords& shape = getShape();
    Obstacle::Coords::const_iterator i = shape.begin();
    Obstacle::Coords::const_iterator j = (shape.rbegin() + 1).base();
    for (; i != shape.end(); j = i++) {
        bool inYRangeUp = (point.y >= i->y) && (point.y < j->y);
        bool inYRangeDown = (point.y >= j->y) && (point.y < i->y);
        bool inYRange = inYRangeUp || inYRangeDown;
        if (!inYRange) continue;
        bool intersects = point.x < (i->x + ((point.y - i->y) * (j->x - i->x) / (j->y - i->y)));
        if (!intersects) continue;
        isInside = !isInside;
    }
    return isInside;
}

namespace {

double segmentsIntersectSegment(const Coord& p1From, const Coord& p1To, const Coord& p2From, const Coord& p2To) 
{
    double p1x = p1To.x - p1From.x;
    double p1y = p1To.y - p1From.y;
    double p2x = p2To.x - p2From.x;
    double p2y = p2To.y - p2From.y;
    double p1p2x = p1From.x - p2From.x;
    double p1p2y = p1From.y - p2From.y;
    double D = (p1x * p2y - p1y * p2x);

    double p1Frac = (p2x * p1p2y - p2y * p1p2x) / D;
    if (p1Frac < 0 || p1Frac > 1) return -1;

    double p2Frac = (p1x * p1p2y - p1y * p1p2x) / D;
    if (p2Frac < 0 || p2Frac > 1) return -1;

    return p1Frac;
}

/**
 * Fast segment-triangle intersection test based on the
 * Möller–Trumbore algorithm (1997).
 *
 * Uses barycentric coordinates to determine whether the segment pierces the
 * triangle and returns the normalized intersection distance along the segment.
 */
double segmentsIntersectTriangle(const Segment& segment, const Triangle& triangle) 
{
    Coord edge1, edge2, segmentVector, h, s, q;
    double a, f, u, v, t;

    edge1.x = triangle.p2.x - triangle.p1.x;
    edge1.y = triangle.p2.y - triangle.p1.y;
    edge1.z = triangle.p2.z - triangle.p1.z;
    edge2.x = triangle.p3.x - triangle.p1.x;
    edge2.y = triangle.p3.y - triangle.p1.y;
    edge2.z = triangle.p3.z - triangle.p1.z;
    segmentVector.x = segment.p2.x - segment.p1.x;
    segmentVector.y = segment.p2.y - segment.p1.y;
    segmentVector.z = segment.p2.z - segment.p1.z;

    h = computeCrossProduct(segmentVector,edge2);
    a = computeDotProduct(edge1,h);

    if (a > -0.00001 && a < 0.00001) {
        return -1;
    }

    f = 1 / a;
    s.x = segment.p1.x - triangle.p1.x;
    s.y = segment.p1.y - triangle.p1.y;
    s.z = segment.p1.z - triangle.p1.z;

    u = f * computeDotProduct(s,h);

    if (u < 0 || u > 1) {
        return -1;
    }

    q = computeCrossProduct(s,edge1);
    v = f * computeDotProduct(segmentVector,q);

    if (v < 0 || u + v > 1) {
        return -1;
    }

    t = f * computeDotProduct(edge2,q);

    if (t > 0 && t < 1) {
        return t;
    }

    return -1;
}

} // namespace

std::vector<double> Obstacle::getIntersections(const Coord& senderPos, const Coord& receiverPos) const
{
    std::vector<double> intersectAt;

    if(getHeight() <= 0 )
    {
        const Obstacle::Coords& shape = getShape();
        Obstacle::Coords::const_iterator i = shape.begin();
        Obstacle::Coords::const_iterator j = (shape.rbegin() + 1).base();
        for (; i != shape.end(); j = i++) {
            const Coord& c1 = *i;
            const Coord& c2 = *j;

            double i = segmentsIntersectSegment(senderPos, receiverPos, c1, c2);
            if (i != -1) {
                intersectAt.push_back(i);
            }
        }
    }
    else
    {
        const Obstacle::Mesh& mesh = getMesh();
        Obstacle::Mesh::const_iterator i = mesh.begin();
        for (; i != mesh.end(); i++) {

            double res = segmentsIntersectTriangle({senderPos, receiverPos}, *i);
            if (res != -1) {
                intersectAt.push_back(res);
            }
        }
    }

    std::sort(intersectAt.begin(), intersectAt.end());
    return intersectAt;
}

std::string Obstacle::getType() const
{
    return type;
}

std::string Obstacle::getId() const
{
    return id;
}

double Obstacle::getAttenuationPerCut() const
{
    return attenuationPerCut;
}

double Obstacle::getAttenuationPerMeter() const
{
    return attenuationPerMeter;
}
