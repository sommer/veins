//
// LinkControl - models Links that block radio transmissions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <veins/modules/application/f2mdVeinsApp/mdSupport/NetworkLinksLib/Link.h>
#include <set>
#include "math.h"
#include "float.h"

Link::Link(){
}

Link::Link(const Link& link){
    for (const Coord& cd : link.coords) {
        coords.push_back(cd);
    }
    bboxP1 = link.bboxP1;
    bboxP2 = link.bboxP2;
}

void Link::setShape(Coords shape) {
	coords = shape;
	bboxP1 = Coord(1e7, 1e7);
	bboxP2 = Coord(-1e7, -1e7);
	for (Coords::const_iterator i = coords.begin(); i != coords.end(); ++i) {
		bboxP1.x = std::min(i->x, bboxP1.x);
		bboxP1.y = std::min(i->y, bboxP1.y);
		bboxP2.x = std::max(i->x, bboxP2.x);
		bboxP2.y = std::max(i->y, bboxP2.y);
	}
}

const Link::Coords& Link::getShape() const {
	return coords;
}

const Coord Link::getBboxP1() const {
	return bboxP1;
}

const Coord Link::getBboxP2() const {
	return bboxP2;
}


namespace {

	void getABCLine(double x1, double y1, double x2, double y2, double &a, double &b, double &c)
	{
	    // (x- p1X) / (p2X - p1X) = (y - p1Y) / (p2Y - p1Y)
	    a = y2 - y1;
	    b = x2 - x1;
	    c = x1 * y2 - x2 * y1;
	}



    // LUT for fast sqrt of floats. Table will be consist of 2 parts, half for sqrt(X) and half for sqrt(2X).
    const int nBitsForSQRTprecision = 11;                       // Use only 11 most sagnificant bits from the 23 of float. We can use 15 bits instead. It will produce less error but take more place in a memory.
    const int nUnusedBits   = 23 - nBitsForSQRTprecision;       // Amount of bits we will disregard
    const int tableSize     = (1 << (nBitsForSQRTprecision+1)); // 2^nBits*2 because we have 2 halves of the table.
    static short sqrtTab[tableSize];
    static unsigned char is_sqrttab_initialized = false;        // Once initialized will be true

    // Table of precalculated sqrt() for future fast calculation. Approximates the exact with an error of about 0.5%
    // Note: To access the bits of a float in C quickly we must misuse pointers.
    // More info in: http://en.wikipedia.org/wiki/Single_precision


    void DistanceFromLine(double cx, double cy, double ax, double ay ,
                            double bx, double by, double * distanceSegment)
      {

          double distanceLine = 0;

          double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
          double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
          double r = r_numerator / r_denomenator;
      //
          double px = ax + r*(bx-ax);
          double py = ay + r*(by-ay);


      //
          double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;



          distanceLine = fabs(s)*sqrt(r_denomenator);

      //
      // (xx,yy) is the point on the lineSegment closest to (cx,cy)
      //
//          double xx = px;
//          double yy = py;

          if ( (r >= 0) && (r <= 1) )
          {
              *distanceSegment = distanceLine;

          }
          else
          {
              double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
              double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
              if (dist1 < dist2)
              {
//                  xx = ax;
//                  yy = ay;
                  *distanceSegment = sqrt(dist1);
              }
              else
              {
//                  xx = bx;
//                  yy = by;
                  *distanceSegment = sqrt(dist2);
              }


          }

         // return 0;

      }

}



double Link::getDistance(const Coord * pos) const{
    double dist = 10000;
    const Coord* oldCd = NULL;
    bool firstelem = true;

    double localdist = 0;
   for (const Coord& cd : coords) {
       if(firstelem){
           oldCd = &cd;
           firstelem = false;
       }else{
          // double localdist1 = pointToLineDistC(pos, *oldCd, cd);
          // double localdist2 = pDistance(pos.x,pos.y, oldCd->x,oldCd->y,cd.x,cd.y);
            DistanceFromLine(pos->x,pos->y, oldCd->x,oldCd->y,cd.x,cd.y, &localdist);

           if(localdist< dist){
               dist = localdist;
           }
           oldCd = &cd;
       }
   }
   return dist;
}

